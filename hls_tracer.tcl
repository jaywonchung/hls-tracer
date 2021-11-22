# TCL driver script for Vitis HLS
#
# The TCL script first prepends a invocation to llvm-link to
# LLVM_CUSTOM_CMD. This is executed right before the tracer pass
# is executed by opt. The output of llvm-link is suppressed so that
# it does not contaminate the actual command in LLVM_CUSTOM_CMD.
#
# Requires the following three environment variables to be set:
# - HLS_TRACER_USER_CODE:    The C/C++ code to be added as source
# - HLS_TRACER_USER_TB:      The C/C++ testbench code to be added
# - HLS_TRACER_TOP_FUNCTION: The name of the top-level function
#
# Usage:
#   vitis_hls -f hls_tracer.tcl
#
# Note:
#   Consider using run.sh with the files in testfunctions/ to get started.

# Check whether the tracer instrumentation pass is built
set ::HLS_LLVM_PLUGIN_DIR pass
set ::HLS_LLVM_TRACER_DIR tracer
if { ![file exists $::HLS_LLVM_PLUGIN_DIR/control-flow-trace-pass.so] } {
  error "Must build control-flow-trace-pass.so before running this script"
}

# Include our tracer pass to the Vitis workflow
# Do Yoon: inject llvm-link call in LLVM custom command to inject our tracer modules into the given code.
set ::LLVM_CUSTOM_CMD {[exec llvm-link -suppress-warnings $LLVM_CUSTOM_INPUT $::HLS_LLVM_TRACER_DIR/control-flow-tracer.bc -o $LLVM_CUSTOM_INPUT > /dev/null]}
append ::LLVM_CUSTOM_CMD {$LLVM_CUSTOM_OPT -load $::HLS_LLVM_PLUGIN_DIR/control-flow-trace-pass.so -controlflowtrace $LLVM_CUSTOM_INPUT -o $LLVM_CUSTOM_OUTPUT}

# Open a project and remove any existing data
open_project -reset proj

# Add kernel
add_files $::env(HLS_TRACER_USER_CODE)

# Add testbench
add_files -tb $::env(HLS_TRACER_USER_TB)

# Set the top-level function
# NOTE: The environment variable HLS_TRACER_TOP_FUNCTION is also read by
#       the tracer instrumentation pass. Thus the pass should also be modified
#       in case the name of this environment variable is to be changed.
set_top $::env(HLS_TRACER_TOP_FUNCTION)

# Open a solution and remove any existing data
open_solution -reset -flow_target vitis solution

# Set the target device
set_part "virtex7"

# Create a virtual clock for the current solution
create_clock -period "300MHz"

# Compile and runs pre-synthesis C simulation using the provided C test bench
csim_design

# Synthesize to RTL
csynth_design

# Execute post-synthesis co-simulation of the synthesized RTL with the original C/C++-based test bench
cosim_design
