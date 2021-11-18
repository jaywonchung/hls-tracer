// The size of the buffer is expected to be a power of 2.
static int current_index_;
static int buffer_size_;
static int buffer_size_mask_;

void controlFlowTracerInit(int size);
void controlFlowTracerRecord(int *array, int row, int column);

void controlFlowTracerInit(int size) {
  current_index_ = 0;
  buffer_size_ = size;
  buffer_size_mask_ = size - 1;
}

void controlFlowTracerRecord(int *array, int row, int column) {
  // Jae-Won: All operations in this function must be as simple and
  // hardware-friendly as possible.
  array[current_index_] = row;
  array[current_index_ + 1] = column;
  current_index_ += 2;
  current_index_ &= buffer_size_mask_;
}
