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

import os
import sys
import glob
import json
import argparse
import subprocess
from dataclasses import dataclass

import seaborn as sns

def main(user_code: str, solution_dir: str, top_function: str) -> None:
    """The main routine for the analysis.

    See the function `parse_args` for explanations on the arguments.
    """
    run_hot_loop_candidate_pass(user_code, top_function)
    hotloop = identify_hotloop(solution_dir)
    results = explore_unroll_factor(hotloop)
    plot_results(results)

@dataclass
class Loop:
    """Internal class holding information about a single loop.

    Implemented as a class to ease extension, especially `computer_temperature`.

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
        """Construct a Loop object from a line from loop-analysis.txt."""
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
    print(f"Running HotLoopCandidatePass on '{user_code}' with top-level function '{top_function}'.")
    command = ["vitis_hls", "pass/run_pass.tcl"]
    env = {**os.environ, "HOT_LOOP_USER_CODE": user_code, "HOT_LOOP_TOP_FUNCTION": top_function}
    # Vitis HLS logs are automatically stored in `vitis_hls.log`.
    subprocess.check_call(command, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def identify_hotloop(solution_dir: str) -> Loop:
    """Finds and returns the hottest loop.

    1. Parse `loop-analysis.txt` and construct a list of `Loop` objects.
    2. Find all trace JSON files inside the `solution_dir` with glob.
    3. Increment the occurrence of a loop by one every time it appears in a trace file.
    4. Find the loop with the highest temperature. Take draws into account.
    """
    # Create Loop objects.
    loops: list[Loop] = []
    with open("loop-analysis.txt") as f:
        for line in f:
            loops.append(Loop.from_line(line))
    if not loops:
        print("No candidate loops were found in the analysis pass. Aborting.")
        sys.exit(1)
    print(f"Candidate loops found: {loops}")

    # Count the occurrence of each loop for each trace file.
    trace_files: list[str] = glob.glob(f"{solution_dir}/sim/wrapc_pc/trace-*.json")
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
        print(f"{len(hottest_loops)} loops have the same temperature. Just selecting the first one.")
    print(f"Hottest loop: {hottest_loops[0]}")
    return hottest_loops[0]

def explore_unroll_factor(loop: Loop) -> list[dict[str, int]]:
    """Try a bunch of unroll factors.

    Results are written in `unroll-result.json`.
    Keys:
        factor (int): The unroll factor used.
        latency (int): The average number of cycles.
        ff (int): The number of Flip-Flops used.
        lut (int): The number of Look-Up Tables used.
    """
    return []

def plot_results(results: list[dict[str, int]]) -> None:
    """Plot the results of unroll factor exploration."""
    sns.set_theme()
    pass

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("USER_CODE", help="Path to the user code.")
    parser.add_argument("SOLUTION_DIR", help="Path to the Vitis project solution directory.")
    parser.add_argument("--top-function", default="top", help="The name of the top-level function.")
    return parser.parse_args()

if __name__ == "__main__":
    args = parse_args()
    main(args.USER_CODE, args.SOLUTION_DIR, args.top_function)
