# Resource Efficiency Analysis for Hot Loop Unrolling

## Introduction

Loop unrolling is an HLS optimization that replicates the body of the loop
multiples times so that multiple bodies can be executed simultaneously.
The loop unroll factor determines the number of replications of the body,
thus impacting performance gain and resource usage.

The purpose of this analysis is to provide HLS developers concrete insight
about the performance gain and resource usage of unrolling loops, allowing
them to make more informed decisions about the effect of changing the
unroll factor.

## Example Usage

```bash
# Build the HotLoopCandidatePass shared object file
source /mnt/storage/jcma/Xilinx/Vitis_HLS/2020.2/settings64.sh
(cd pass; ./build.sh)

# Run the driver script
./main.py ../../testfunctions/nested.cpp ../../proj/solution
```
