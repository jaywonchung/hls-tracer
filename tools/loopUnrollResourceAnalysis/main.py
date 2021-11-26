#!/usr/bin/env python

"""
Resource Efficiency Analysis for Hot Loop Unrolling

This script automates the following process of analysis:
1. Run the HotLoopCandidate analysis pass on the user code. This pass
   will identify the names of top-level loops and the line numbers the
   loop with respect to the high-level source code.
2. Analyze the control flow trace files of the module to identify the
   "hottest" loop, defined as the loop most frequently executed across
   multiple executions of the module on different data.
3. For the hottest loop, run HLS with different unroll factors. For
   each unroll factor, figure out the latency and resource usage of
   the circuit.
4. Plot the resource efficiency against the loop unroll factor.
"""

from __future__ import annotations

import argparse
import glob
import json
import multiprocessing.pool
import os
import subprocess
import sys
import tempfile
import threading
from dataclasses import dataclass
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

HOT_LOOP_PASS_PATH = "pass/run_pass.tcl"
HOT_LOOP_PASS_LOG_PATH = "pass/vitis_hls.log"
HOT_LOOP_PASS_RESULT_PASS = "loop-analysis.txt"
TEMPLATE_TCL_PATH = "run_unroll.tcl.template"

VITIS_TRACE_FILE_PATHS = "{solution_dir}/sim/wrapc_pc/trace-*.json"
VITIS_LATENCY_RRT_PATH = "proj{factor}/solution/sim/report/verilog/lat.rpt"
VITIS_SOLUTION_DATA_JSON_PATH = "proj{factor}/solution/solution_data.json"

UNROLL_RESULT_JSON_PATH = "unroll-result.json"
UNROLL_RESULT_CSV_PATH = "unroll-result.csv"
UNROLL_RESULT_PLOT_PATH = "unroll-result.png"

NUM_WORKERS = 5
UNROLL_RANGE = range(1, 9)


def main(user_code: str, solution_dir: str, top_function: str, array_name: str) -> None:
    """The main routine for the analysis.

    See the function `parse_args` for explanations on the arguments.
    """
    run_hot_loop_candidate_pass(user_code, top_function)
    hotloop = identify_hotloop(solution_dir)
    explore_unroll_factor(hotloop, user_code, top_function, array_name)
    plot_results()


@dataclass
class Loop:
    """Internal class holding information about a single loop.

    Implemented as a class to ease extension, especially `compute_temperature`.

    Attributes:
        name (str): The name of the loop (llvm.loop.name).
                    Either specified by the user in the source code or assigned by Vitis.
        line_range (tuple[int, int]): The loop's minimum and maximum line numbers.
        occurrence (int): The number of times a loop was executed.
    """

    name: str
    line_range: tuple[int, int]
    occurrence: int

    @staticmethod
    def from_line(line: str) -> Loop:
        """Construct a Loop object from a line from the hot loop analysis result."""
        name, min_line, max_line = line.strip().split(" ")
        return Loop(name, (int(min_line), int(max_line)), 0)

    def appears_in(self, trace: list[int]) -> bool:
        """Check whether this loop is ever executed in the given trace."""
        minl, maxl = self.line_range
        return any(minl <= line_number <= maxl for line_number in trace)

    def compute_temperature(self) -> int:
        """The loop with the highest temperature is the hottest one."""
        temperature = self.occurrence
        print(f"Temperature of {self} is {temperature}.")
        return temperature


def run_hot_loop_candidate_pass(user_code: str, top_function: str) -> None:
    """Run the HotLoopCandidatePass on the given user source code.

    This invokes csynth_design in Vitis HLS in order to run the pass.
    The result of synthesis is discarded upon completion.
    """
    print(
        f"Running HotLoopCandidatePass on '{user_code}' with top-level function '{top_function}'."
    )
    command = ["vitis_hls", HOT_LOOP_PASS_PATH, "-l", HOT_LOOP_PASS_LOG_PATH]
    env = {
        **os.environ,
        "HOT_LOOP_USER_CODE": user_code,
        "HOT_LOOP_TOP_FUNCTION": top_function,
    }
    # Vitis HLS logs are automatically stored in `vitis_hls.log`.
    subprocess.check_call(
        command, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )


def identify_hotloop(solution_dir: str) -> Loop:
    """Finds and returns the hottest loop.

    1. Parse `loop-analysis.txt` and construct a list of `Loop` objects.
    2. Find all trace JSON files inside the `solution_dir` with glob.
    3. Increment the occurrence of a loop by one every time it appears in a trace file.
    4. Find the loop with the highest temperature. Take draws into account.
    """
    # Create Loop objects.
    loops: list[Loop] = []
    with open(HOT_LOOP_PASS_RESULT_PASS) as f:
        for line in f:
            loops.append(Loop.from_line(line))
    if not loops:
        print("No candidate loops were found in the analysis pass. Aborting.")
        sys.exit(1)
    print(f"Candidate loops found: {loops}")

    # Count the occurrence of each loop for each trace file.
    trace_files: list[str] = glob.glob(
        VITIS_TRACE_FILE_PATHS.format(solution_dir=solution_dir)
    )
    if not trace_files:
        print("No trace files were found in the solution directory. Aborting.")
        sys.exit(1)
    for trace_file in trace_files:
        full_trace: list[dict[str, int]] = json.load(open(trace_file))
        # We're only interested in line numbers.
        trace: list[int] = list(map(lambda d: int(d["line"]), full_trace))
        for loop in loops:
            if loop.appears_in(trace):
                loop.occurrence += 1

    # Find the loop with the highest temperature.
    # Keep in mind that there may be multiple loops with the max temperature.
    hottest_loops: list[Loop] = []
    max_temperature = -1
    for loop in loops:
        temperature = loop.compute_temperature()
        if max_temperature < temperature:
            max_temperature = temperature
            hottest_loops.clear()
            hottest_loops.append(loop)
        elif max_temperature == temperature:
            hottest_loops.append(loop)
    if len(hottest_loops) > 1:
        print(
            f"{len(hottest_loops)} loops have the same temperature. Just selecting the first one."
        )
    print(f"Hottest loop: {hottest_loops[0]}")
    return hottest_loops[0]


def explore_unroll_factor(
    loop: Loop, user_code: str, top_function: str, array_name: str
) -> None:
    """Try a bunch of unroll factors in parallel.

    Results are written in a JSON file.
    Keys:
        factor (int): The unroll factor used.
        latency (int): The total execution tile in cycles.
        ff (int): The number of Flip-Flops used.
        lut (int): The number of Look-Up Tables used.
    """
    template_tcl = open(TEMPLATE_TCL_PATH).read()

    # Worker function that generates a TCL script for a specific unroll factor
    # and runs it with Vitis HLS.
    def try_one_unroll_factor(unroll_factor: int) -> None:
        print(f"[TID {threading.get_ident()}] Trying unroll factor {unroll_factor}.")
        # Create a temporary file to write the generated TCL script.
        # NamedTemporaryFile is race-safe.
        f = tempfile.NamedTemporaryFile(mode="w", suffix=".tcl")

        # Fill in the template.
        proj_name = f"proj{unroll_factor}"
        ucp = Path(user_code)
        user_tb = str(ucp.with_name(f"{ucp.stem}_test{ucp.suffix}"))
        f.write(
            template_tcl.format(
                proj_name=proj_name,
                user_code=user_code,
                user_tb=user_tb,
                top_function=top_function,
                loop_name=loop.name,
                unroll_factor=unroll_factor,
                array_name=array_name,
            )
        )
        f.flush()

        print(f"[TID {threading.get_ident()}] Launching Vitis HLS.")
        command = ["vitis_hls", f.name, "-l", f"vitis_hls_{unroll_factor}.log"]
        p = subprocess.Popen(
            command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        )
        p.wait()
        f.close()

        print(f"[TID {threading.get_ident()}] Done.")

    # Run in process pool.
    with multiprocessing.pool.ThreadPool(NUM_WORKERS) as p:
        p.map(try_one_unroll_factor, UNROLL_RANGE)

    # Result list.
    results: list[dict[str, int]] = [{"factor": factor} for factor in UNROLL_RANGE]

    # Parse total execution latency.
    for result in results:
        report = open(VITIS_LATENCY_RRT_PATH.format(factor=result["factor"])).read()
        latency = int(report.splitlines()[-1].split(" = ")[-1][1:-1])
        result["latency"] = latency

    # Parse resource usage (FF and LUT).
    for result in results:
        report = json.load(
            open(VITIS_SOLUTION_DATA_JSON_PATH.format(factor=result["factor"]))
        )
        resource = report["ModuleInfo"]["Metrics"][top_function]["Area"]
        result["ff"] = int(resource["FF"])
        result["lut"] = int(resource["LUT"])

    # Dump result to JSON.
    results_with_metadata = dict(
        metadata=dict(
            user_code=user_code,
            top_function=top_function,
            unrolled_loop_name=loop.name,
            partitioned_array_name=array_name,
        ),
        results=results,
    )
    with open(UNROLL_RESULT_JSON_PATH, "w") as f:
        json.dump(results_with_metadata, f, indent=2)

    # Also dump raw results to CSV.
    with open(UNROLL_RESULT_CSV_PATH, "w") as f:
        f.write("factor,latency,ff,lut\n")
        for result in results:
            f.write(
                f"{result['factor']},{result['latency']},{result['ff']},{result['lut']}\n"
            )


def plot_results() -> None:
    """Plot the results of unroll factor exploration."""
    # Read results into a Pandas DataFrame from CSV.
    df = pd.read_csv(UNROLL_RESULT_CSV_PATH)

    # Compute resource efficiency, for each ff and lut.
    df["ff_efficiency"] = 1 / (df["ff"] * df["latency"])
    df["lut_efficiency"] = 1 / (df["lut"] * df["latency"])

    # Plot with seaborn.
    sns.set()
    fig, axes = plt.subplots(1, 2, figsize=(10, 6))
    axes[0].set_title("Flip-Flop efficiency")
    axes[1].set_title("Look-Up Table efficiency")
    sns.lineplot(ax=axes[0], x=df["factor"], y=df["ff_efficiency"])
    sns.lineplot(ax=axes[1], x=df["factor"], y=df["lut_efficiency"])
    fig.savefig(UNROLL_RESULT_PLOT_PATH)

    print(f"Saved plot to {UNROLL_RESULT_PLOT_PATH}.")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("USER_CODE", help="Path to the user code.")
    parser.add_argument(
        "SOLUTION_DIR", help="Path to the Vitis project solution directory."
    )
    parser.add_argument(
        "--top-function", default="top", help="The name of the top-level function."
    )
    parser.add_argument(
        "--array-name", default="acc", help="The name of the array to partition."
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    main(args.USER_CODE, args.SOLUTION_DIR, args.top_function, args.array_name)
