// Control Flow Tracer
//
// The goal of the tracer is to accumulate control flow trace (a sequence
// of [line number, column number]) of the high-level source code executed 
// inside hardware.
//
// The tracer consists of three functions that implement the behavior of
// the tracer and five static variables that enclose the state of the tracer.
//
// Trace will be written to an integer array sequentially. When the array is
// full, the tracer will wrap around and overwrite from the beginning. This
// information needs to be conveyed to the user who will only have access to
// an array of integers. Thus, the last two entries of the trace array is
// reserved for 1) the current index (where the next trace data would have
// been written) and 2) whether a wrap ever happened (1 means happened).
//
// IMPORTANT:
// It is required that the size of the buffer is of the form 2^n + 2.
// Then the first 2^n entries will contain trace data (2^(n-1) records since
// one record writes two integers, line number and column number). Then
// the later two entries each contain the current index and the wrap indicator.

#ifndef _CONTROL_FLOW_TRACER_H_
#define _CONTROL_FLOW_TRACER_H_

// The index of the trace array where the next write will happen.
static int current_index_;
// A boolean indicator that shows whether an index wrap occurred.
static int wrapped_;
// The size of the trace array. Value is 2^n + 2.
static int buffer_size_;
// A mask used to wrap current_index_. Value is 2^n - 1.
static int buffer_size_mask_;
// A mask used to set the wrap indicator. Value is 2^n.
static int buffer_wrapped_mask_;

// NOTE: The need to always pass the tracer array as argument
// The pointer to the trace array cannot be stored as a static variable because
// Vitis HLS does not support pointer to pointers. In essence, Vitis HLS cannot
// guarantee whether the address stored in the static variable is not some
// arbitrary address, even if we store the address of the tracer array to the
// static variable only once.

// Initializes all five static variables. Only called exactly once at the
// beginning of the top-level function.
void controlFlowTracerInit(int size);
// Writes two integers to the trace array: row (line number) and column (column
// number). Called at every trace record location. It has been observed that
// the column number does not always make sense.
void controlFlowTracerRecord(int *array, int row, int column);
// Writes current_index_ and wrapped_ at the last two entries reserved in the
// trace array. Called right before the return instruction of the top-level
// function.
void controlFlowTracerFinish(int *array);

#endif
