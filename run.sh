#!/bin/bash
# 
# Driver script for HLS with control flow tracing.
#
# Usage:
#   ./run.sh USER_CODE_PATH [TOP_FUNCTION_NAME]

export HLS_TRACER_USER_CODE="$1"
export HLS_TRACER_USER_TB="${1%.cpp}_test.cpp"
export HLS_TRACER_TOP_FUNCTION="${2:-top}"

vitis_hls -f hls_tracer.tcl
