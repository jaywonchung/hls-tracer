#include <algorithm>
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
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
  Function* getTracerFunction(const TracerFunction tracerFunc);
  int getTracerFunctions(Module::FunctionListType& functions);

 private:
  std::map<std::string, Function*> tracerFunctions;
};

ControlFlowTracePass::ControlFlowTracePass() : ModulePass(ID) {}

bool ControlFlowTracePass::runOnModule(Module& module) {
  bool changed = false;
  errs() << "Entered module " << module.getName() << ".\n";
  getTracerFunctions(module.getFunctionList());

  IRBuilder<> builder(module.getContext());

  // Insu: Here get a global variable pointer (pointerToTracer),
  // targeting named controlFlowTracer (defined in control-flow-trace-pass.cpp)
  // (Hopefully).
  GlobalVariable* controlFlowTracer =
      module.getNamedGlobal("controlFlowTracer");
  assert(controlFlowTracer != nullptr &&
         "Cannot find a global ControlFlowTracer variable.");
  Value* pointerToTracer = builder.CreatePointerCast(
      controlFlowTracer, controlFlowTracer->getType());

  // Insu: Use llvm::IRBuilder to create a call and insert it.
  // TODO: Need to adjust insertion points.
  for (auto& func : module.getFunctionList()) {
    if (func.getName().contains("sigma_n") == false)
      continue;

    for (auto& bb : func) {
      for (auto bi = bb.begin(), bend = bb.end(); bi != bend; bi++) {
        if (isa<BranchInst>(bi) == false)
          continue;
        DILocation* loc = bi->getDebugLoc().get();

        // New instructions will be added here.
        // Without this code a segfault occurs, saying
        // "Error reading file: No such file or directory".
        builder.SetInsertPoint(&*bi);

        auto increaseCountTracerFunc =
            getTracerFunction(TracerFunction::IncrementCount);
        assert(increaseCountTracerFunc && "Not found tracer function!");

        auto filename = builder.CreateGlobalStringPtr(loc->getFilename(),
                                                      loc->getFilename());
        Value* pointerToFilename = builder.CreatePointerCast(
            filename, increaseCountTracerFunc->getArg(1)->getType());

        ArrayRef<Value*> args = {pointerToTracer, pointerToFilename,
                                 builder.getInt32(loc->getLine()),
                                 builder.getInt32(loc->getColumn())};

        builder.CreateCall(increaseCountTracerFunc, args);
        changed = true;
      }
    }
  }

  return false;
}

int ControlFlowTracePass::getTracerFunctions(
    Module::FunctionListType& functions) {
  int function_num = 0;
  for (auto& func : functions) {
    if (func.getName().contains("ControlFlowTracer") == false)
      continue;
    tracerFunctions.insert({func.getName(), &func});
  }
  return function_num;
}

Function* ControlFlowTracePass::getTracerFunction(
    const TracerFunction tracerFunc) {
  Function* func = nullptr;
  std::string key;
  if (tracerFunc == TracerFunction::IncrementCount)
    key = "incrementControlFlowCount";
  else if (tracerFunc == TracerFunction::GetCount)
    key = "getCount";
  else if (tracerFunc == TracerFunction::Dump)
    key = "Dump";
  else
    return nullptr;

  // This checks whether the given key value is contained in the map element's
  // key.
  auto it = std::find_if(
      tracerFunctions.begin(), tracerFunctions.end(),
      [&key](const std::pair<std::string, Function*>& element) -> bool {
        return element.first.find(key) != std::string::npos;
      });
  if (it != tracerFunctions.end()) {
    func = it->second;
  }
  return func;
}

}  // namespace

char ControlFlowTracePass::ID = 0;
static RegisterPass<ControlFlowTracePass> X("controlflowtrace",
                                            "Pass for tracing control flow",
                                            false,
                                            false);