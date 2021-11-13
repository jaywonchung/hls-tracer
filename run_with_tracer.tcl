# Check whether the tracer instrumnetation pass is built
set ::HLS_LLVM_PLUGIN_DIR pass
if { ![file exists $::HLS_LLVM_PLUGIN_DIR/control-flow-trace-pass.so] } {
  error "Must build control-flow-trace-pass.so before running this script"
}

# Include our tracer pass to the Vitis workflow
set ::LLVM_CUSTOM_CMD {$LLVM_CUSTOM_OPT -load $::HLS_LLVM_PLUGIN_DIR/control-flow-trace-pass.so -controlflowtrace $LLVM_CUSTOM_INPUT -o $LLVM_CUSTOM_OUTPUT}

# Open a project and remove any existing data
open_project -reset proj

# Add kernel and testbench
# TODO: Make sigma.cpp and sigma_test.cpp configurable
add_files testfunctions/sigma.cpp
add_files -tb testfunctions/sigma_test.cpp

# Tell the top
# # TODOL Make sigma_n configurable
set_top sigma_n

# Open a solution and remove any existing data
open_solution -reset solution

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
