# Check whether the pass shared object file is built
set ::HOT_LOOP_CANDIDATE_PASS_SO pass/hot-loop-candidate-pass.so
if { ![file exists $::HOT_LOOP_CANDIDATE_PASS_SO] } {
  error "Must build hot-loop-candidate-pass.so before running this script"
}

set ::LLVM_CUSTOM_CMD {$LLVM_CUSTOM_OPT -load $::HOT_LOOP_CANDIDATE_PASS_SO -hotloopcandidate $LLVM_CUSTOM_INPUT -o $LLVM_CUSTOM_OUTPUT}

# Open a dummy project
open_project -reset proj

# Add the user code
add_files $::env(HOT_LOOP_USER_CODE)

# Set the top-level function
set_top $::env(HOT_LOOP_TOP_FUNCTION)

# Open a dummy solution
open_solution -reset -flow_target vitis solution

# Set the target device
set_part "virtex7"

# Create a virtual clock for the current solution
create_clock -period "300MHz"

# Run synthesis, in order to run the HotLoopCandidatePass
csynth_design

# Run cleanup
exec rm -rf proj
