#include <string>
#include <vector>
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

enum class TracerFunction : int {
  IncrementCount,
  GetCount,
  Dump,
};

struct ControlFlowTracePass : public ModulePass {
  static char ID;
  ControlFlowTracePass();

  virtual bool runOnModule(Module& module) override;

 private:
  Function* getFunctionIR(Module& module, const TracerFunction tracerFunc);
};

ControlFlowTracePass::ControlFlowTracePass() : ModulePass(ID) {}

bool ControlFlowTracePass::runOnModule(Module& module) {
  errs() << "Entered module " << module.getName() << ".\n";

  // TODO: Find branch instructions (for control flow analysis)
  // and add tracer functions with the corresponding llvm::DILocation *
  // that can be achieved via llvm::Instruction::getDebugLoc()

  auto tracerFunc = getFunctionIR(module, TracerFunction::IncrementCount);
  return false;
}

Function* ControlFlowTracePass::getFunctionIR(Module& module,
                                              const TracerFunction tracerFunc) {
  Type* retType = nullptr;
  std::vector<Type*> argTypes;
  FunctionType* funcType = nullptr;
  Function* func = nullptr;

  switch (tracerFunc) {
    case TracerFunction::IncrementCount:
      /**
       * Insu: testcode to create a member-function call with pointer arguments.
       * Not working, so I commented them out.
       * Leave it for now for study.
       */
      // retType = Type::getVoidTy(module.getContext());
      // argTypes.push_back(PointerType::get(StructType::create(module.getContext(),
      // "ControlFlowTracer"), 0));
      // argTypes.push_back(PointerType::get(StructType::create(module.getContext(),
      // "llvm::DILocation"), 0)); funcType = FunctionType::get(retType,
      // argTypes, false); func = cast<Function>(
      //     module.getOrInsertFunction("_ZN17ControlFlowTracer25incrementControlFlowCountEPKN4llvm10DILocationE",
      //                                  funcType));
      // if (func) {
      //   func->print(errs());
      // } else {
      //   errs() << "func: nil\n";
      // }

      break;
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