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

import sys
import argparse
import subprocess
from dataclasses import dataclass

def main(user_code: str, solution_dir: str, custom_opt: str, hotloop_pass_so: str) -> int:
    run_hotloop_candidate_analysis(user_code, custom_opt, hotloop_pass_so)
    hotloop = identify_hotloop()
    print(solution_dir)
    print(custom_opt)
    return 0

@dataclass
class Loop:
    """Internal class holding information about a single loop.

    Implemented as a class to ease extension.

    Attributes:
        name (str): The name of the loop (llvm.loop.name).
                    Either specified by the user in the source code or assigned by Vitis.
        line_range (Tuple[int, int]): The loop's minimum and maximum line numbers.
        frequency (int): The number of times a loop was executed.
    """
    name: str
    line_range: tuple[int, int]
    frequency: int

    def compute_temperature(self) -> int:
        """The loop with the highest temperature is the hottest one."""
        return self.frequency

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("USER_CODE", help="Path to the user code.")
    parser.add_argument("SOLUTION_DIR", help="Path to the Vitis project solution directory.")
    parser.add_argument(
        "--hot-loop-pass-so",
        default="hot-loop-candidate-pass.so",
        help="Path to the HotLoopCandidate analysis pass shared object file."
    )
    parser.add_argument(
        "--custom-opt",
        default="/mnt/storage/jcma/Xilinx/Vitis_HLS/2020.2/lnx64/tools/clang-3.9-csynth/bin/opt",
        help="Path to the custom opt binary"
    )
    return parser.parse_args()

def run_hotloop_candidate_analysis(user_code: str, custom_opt: str, hotloop_pass_so: str) -> None:
    command = [
        custom_opt,
        user_code,
        "-load",
        hotloop_pass_so,
        "-hotloopcandidate"
    ]
    subprocess.check_call(command)

def identify_hotloop() -> Loop:
    pass

if __name__ == "__main__":
    args = parse_args()
    sys.exit(main(
        args.USER_CODE,
        args.SOLUTION_DIR,
        args.custom_opt,
        args.hot_loop_pass_so
    ))
