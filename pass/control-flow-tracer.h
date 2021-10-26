#ifndef __CONTROL_FLOW_TRACER_H__
#define __CONTROL_FLOW_TRACER_H__

#include <map>
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"

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

#endif