#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct ControlFlowTracePass : public FunctionPass {
  static char ID;
  ControlFlowTracePass();

  virtual bool runOnFunction(Function &func) override;
};

ControlFlowTracePass::ControlFlowTracePass()
  : FunctionPass(ID) {}

bool ControlFlowTracePass::runOnFunction(Function &func) {
  errs() << "Entered function " << func.getName() << ".\n";

  for (auto fit = func.begin(), fend = func.end(); fit != fend; fit++) {
    for (auto bit = fit->begin(), bend = fit->end(); bit != bend; bit++) {
      const DILocation *loc = bit->getDebugLoc();
      if (loc == nullptr) continue;
      
    }
  }

  return false;
}

}  // namespace

char ControlFlowTracePass::ID = 0;
static RegisterPass<ControlFlowTracePass> X("controlflowtrace",
                                            "Pass for tracing control flow",
                                            false,
                                            false);