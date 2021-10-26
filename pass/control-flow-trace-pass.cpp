#include <string>
#include <vector>
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

enum class TracerFunction : int {
  IncrementCount,
  GetCount,
  Dump,
};

struct ControlFlowTracePass : public FunctionPass {
  static char ID;
  ControlFlowTracePass();

  virtual bool doInitialization(Module& mod) override;
  virtual bool runOnFunction(Function& func) override;

 private:
  Function* getFunctionIR(const TracerFunction func);

 private:
  Module* module_;
};

ControlFlowTracePass::ControlFlowTracePass()
    : FunctionPass(ID), module_(nullptr) {}

bool ControlFlowTracePass::doInitialization(Module& mod) {
  module_ = &mod;
  return false;
}

bool ControlFlowTracePass::runOnFunction(Function& func) {
  errs() << "Entered function " << func.getName() << ".\n";

  // TODO: Find branch instructions (for control flow analysis)
  // and add tracer functions with the corresponding llvm::DILocation *
  // that can be achieved via llvm::Instruction::getDebugLoc()

  auto tracerFunc = getFunctionIR(TracerFunction::IncrementCount);
  return false;
}

Function* ControlFlowTracePass::getFunctionIR(const TracerFunction func) {
  FunctionType* retType = nullptr;
  std::vector<Type*> argTypes;
  Function* func = nullptr;

  switch (func) {
    case TracerFunction::IncrementCount: {
      break;
    }
    case TracerFunction::GetCount:

      break;
    case TracerFunction::Dump:

      break;
  }

  return func;
}

}  // namespace

char ControlFlowTracePass::ID = 0;
static RegisterPass<ControlFlowTracePass> X("controlflowtrace",
                                            "Pass for tracing control flow",
                                            false,
                                            false);