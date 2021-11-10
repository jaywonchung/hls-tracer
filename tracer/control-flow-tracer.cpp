#include <map>
#include <sstream>
#include <string>
#include <iostream>

class ControlFlowTracer {
 public:
  ControlFlowTracer(): current_index_(0), buffer_size_(0), array_(nullptr) {};
  
  void init(int *array, int size);
  void record(int row, int column);

 private:
  int current_index_;
  int buffer_size_;
  int *array_;
};

/* Should not be defined in class definition, otherwise inlined and cannot found in IR */
void ControlFlowTracer::init(int *array, int size) {
  std::cout << "Entered ControlFlowTracer init function" << std::endl;
  buffer_size_ = size;
  array_ = array;
}

void ControlFlowTracer::record(int row, int column) {
  std::cout << "Entered ControlFlowTracer record function" << std::endl;
  array_[current_index_] = row;
  array_[current_index_ + 1] = column;
  current_index_ = (current_index_ + 2 ) % (buffer_size_ / sizeof(int));
}

// Global variable that our pass will reference.
ControlFlowTracer controlFlowTracer;
