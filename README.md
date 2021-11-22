# Control Flow Tracing for High-Level Synthesis

## Introduction

High-Level Synthesis is increasingly adopted as a method for programming FPGAs with high-level C/C++ code.
However, the lack of observability in hardware precludes both convenient debugging and profile-guided optimizations (PGO).
Thus, in this project, we aim to implement control flow tracing for HLS.
The goal of our framework is to transparently instrument the high-level code via a custom LLVM pass to allow traces to be collected directly on FPGAs.

## How it Works

We provide some simple test workloads inside the `testfunctions` directory.
For example, to run `testfunctions/sigma.cpp` and see the collected trace with RTL simulation, run the following:

```bash
./run.sh testfunctions/sigma.cpp
```

This will add `testfunctions/sigma.cpp` as the source file and `testfunctions/sigmal_test.cpp` as the testbench file.
Then, `vitis_hls` will be invoked with `hls_tracer.tcl`. This will run C-only simulation, run high-level synthesis, and then run co-simulation.
Our LLVM pass will be applied at the time of high-level synthesis and instrument the given user code (`testfunctions/sigma.cpp` in this case) with calls to our tracer implementation.
When the instrumented code runs during co-simulation, trace data (a sequence of code lines and columns) will be written to the array (the first argument to the top-level function).
