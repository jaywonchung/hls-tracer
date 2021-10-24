#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct ControlFlowTracePass : public ModulePass {
  static char ID;
  ControlFlowTracePass() : ModulePass(ID) {}

  virtual bool runOnModule(Module& module) override {
    errs() << "Entered module " << module.getName() << ".\n";
    return false;
  }
};
}  // namespace

char ControlFlowTracePass::ID = 0;
static RegisterPass<ControlFlowTracePass> X("controlflowtrace",
                                            "Pass for tracing control flow",
                                            false,
                                            false);