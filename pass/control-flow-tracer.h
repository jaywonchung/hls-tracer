#ifndef __CONTROL_FLOW_TRACER_H__
#define __CONTROL_FLOW_TRACER_H__

#include "llvm/IR/DebugInfoMetadata.h"
#include <map>
#include <cerrno>

class ControlFlowTracer {
 public:
  ControlFlowTracer() = default;
  void incrementControlFlowCount(const llvm::DILocation *location);
  int getControlFlowCount(const llvm::DILocation *location) const;
  void dump() const;

 private:
  std::map<const llvm::DILocation *, unsigned int> counts_;
};

#endif