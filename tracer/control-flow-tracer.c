#include "control-flow-tracer.h"

void controlFlowTracerInit(int size) {
  current_index_ = 0;
  wrapped_ = 0;
  buffer_size_ = size;
  buffer_size_mask_ = size - 3;
  buffer_wrapped_mask_ = size - 2;
}

void controlFlowTracerRecord(int *array, int row, int column) {
  // Jae-Won: All operations in this function must be as simple and
  // hardware-friendly as possible.
  array[current_index_] = row;
  array[current_index_ + 1] = column;
  current_index_ += 2;
  wrapped_ |= (current_index_ & buffer_wrapped_mask_);  // bitwise-and is non-zero when current_index_ >= 2^n
  current_index_ &= buffer_size_mask_; // bitwise-and ensures current_index_ range 0 ~ 2^n - 1
}

void controlFlowTracerFinish(int *array) {
  array[buffer_size_ - 2] = current_index_; 
  array[buffer_size_ - 1] = wrapped_ ? 1 : 0;
}
