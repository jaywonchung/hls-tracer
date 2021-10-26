#include <map>
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

class ControlFlowTracer {
 public:
  ControlFlowTracer(llvm::Module& module) : module_(module){};
  void incrementControlFlowCount(const llvm::DILocation* location);
  int getControlFlowCount(const llvm::DILocation* location) const;

  void dump() const;

 private:
  llvm::Module& module_;
  std::map<const llvm::DILocation*, unsigned int> counts_;
};

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

/**
 * Insu: Code commented due to version incompatibility issue.
 * I am testing with regular LLVM 10 before using Vitis clang,
 * which no longer has llvm::fs::OpenFlags::F_RW.
 */
void ControlFlowTracer::dump() const {
  // std::error_code ec;
  // llvm::raw_fd_ostream os(llvm::StringRef("trace.result"), ec,
  //                         llvm::sys::fs::OpenFlags::F_RW);

  // for (auto count : counts_) {
  //   count.first->print(os);
  // }

  // os.flush();
  // os.close();
}
