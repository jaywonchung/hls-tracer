#include <map>
#include <sstream>
#include <string>

class ControlFlowTracer {
 public:
  ControlFlowTracer() = default;
  void incrementControlFlowCount(const std::string& filename,
                                 const unsigned int line,
                                 const unsigned int column);
  int getControlFlowCount(const std::string& filename,
                          const unsigned int line,
                          const unsigned int column) const;

  void dump() const;

 private:
  std::string constructLocationInfo(const std::string& filename,
                                    const unsigned int line,
                                    const unsigned int column) const;

 private:
  std::map<std::string, unsigned int> counts_;
};

// Global variable that our pass will reference.
ControlFlowTracer controlFlowTracer;

std::string ControlFlowTracer::constructLocationInfo(
    const std::string& filename,
    const unsigned int line,
    const unsigned int column) const {
  std::stringstream ss;
  ss << filename << ":" << line << ":" << column;
  return ss.str();
}

void ControlFlowTracer::incrementControlFlowCount(const std::string& filename,
                                                  const unsigned int line,
                                                  const unsigned int column) {
  // Try to insert the given location with count 1.
  auto result =
      counts_.insert({constructLocationInfo(filename, line, column), 1});
  // If result.second == false, it means there already is an entry for the
  // location. Increment the count.
  if (result.second == false) {
    result.first->second++;
  }
}

int ControlFlowTracer::getControlFlowCount(const std::string& filename,
                                           const unsigned int line,
                                           const unsigned int column) const {
  auto result = counts_.find(constructLocationInfo(filename, line, column));
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
