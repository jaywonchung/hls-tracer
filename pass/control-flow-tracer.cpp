#include "control-flow-tracer.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

void ControlFlowTracer::incrementControlFlowCount(
    const llvm::DILocation* location) {
  // Try to insert the given location with count 1.
  auto result = counts_.insert({location, 1});
  // If result.second == false, it means there already is an entry for the
  // location. Increment the count.
  if (result.second == false) {
    result.first->second++;
  }
}

int ControlFlowTracer::getControlFlowCount(
    const llvm::DILocation* location) const {
  auto result = counts_.find(location);
  if (result == counts_.end()) {
    return -ENOENT;
  }
  return result->second;
}

void ControlFlowTracer::dump() const {
  std::error_code ec;
  llvm::raw_fd_ostream os(llvm::StringRef("trace.result"), ec,
                          llvm::sys::fs::OpenFlags::F_RW);

  for (auto count : counts_) {
    count.first->print(os);
  }
}