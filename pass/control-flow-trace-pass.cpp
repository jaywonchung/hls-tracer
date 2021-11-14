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
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

enum class TracerFunction : int {
  Init,
  Record,
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

#define ITEM_WIDTH 8
#define MAX_ITEM_NUM 128

bool ControlFlowTracePass::runOnModule(Module& module) {
  bool changed = false;
  errs() << "Entered module " << module.getName() << ".\n";
  getTracerFunctions(module.getFunctionList());

  IRBuilder<> builder(module.getContext());

  /**
   * Insu: targeting named controlFlowTracer (defined in control-flow-trace-pass.cpp)
   * pointerToTracer refers: ControlFlowTracer * (value: &controlFlowTracer);
   * 
   * Nov 13: Now tracer has been changed to C style, with no global tracer instance,
   * We don't have to store a pointer heading to the global variable.
   * Code is remained to refer how to access pointer of the variable.
   */
  // GlobalVariable* controlFlowTracer =
  //     module.getNamedGlobal("controlFlowTracer");
  // assert(controlFlowTracer != nullptr &&
  //        "Cannot find a global ControlFlowTracer variable.");
  // Value* pointerToTracer = builder.CreatePointerCast(
  //     controlFlowTracer, controlFlowTracer->getType());

  // Insu: Use llvm::IRBuilder to create a call and insert it.
  // TODO: Need to adjust insertion points.
  for (auto& func : module.getFunctionList()) {
    if (func.getName().contains("sigma_n") == false)
      continue;

    // Inject init function
    auto initTracerFunc = getTracerFunction(TracerFunction::Init);
    assert(initTracerFunc && "Cannot find a record tracer function!");
    auto fi = func.getBasicBlockList().begin()->getFirstInsertionPt();
    builder.SetInsertPoint(&*fi);

    ArrayRef<Value*> args = {func.getArg(1),
                             builder.getInt32(ITEM_WIDTH * MAX_ITEM_NUM)};
    builder.CreateCall(initTracerFunc, args);
    changed = true;

#if 0
    for (auto& bb : func) {
      for (auto bi = bb.begin(), bend = bb.end(); bi != bend; bi++) {
        if (isa<BranchInst>(bi) == false)
          continue;
        DILocation* loc = bi->getDebugLoc().get();
        assert(loc && "Cannot find debug location");

        // New instructions will be added here.
        // Without this code a segfault occurs, saying
        // "Error reading file: No such file or directory".
        builder.SetInsertPoint(&*bi);

        auto recordTracerFunc = getTracerFunction(TracerFunction::Record);
        assert(recordTracerFunc && "Cannot find a record tracer function!");

        ArrayRef<Value*> args = {pointerToTracer,
                                 builder.getInt32(loc->getLine()),
                                 builder.getInt32(loc->getColumn())};

        builder.CreateCall(recordTracerFunc, args);
        changed = true;
      }
    }
    #endif
  }
  return false;
}

int ControlFlowTracePass::getTracerFunctions(
    Module::FunctionListType& functions) {
  int function_num = 0;
  for (auto& func : functions) {
    if (func.getName().contains("controlFlowTracer") == false)
      continue;
    tracerFunctions.insert({func.getName(), &func});
    errs() << "Function: " << func.getName()
           << " added into tracer functions\n";
  }
  return function_num;
}

Function* ControlFlowTracePass::getTracerFunction(
    const TracerFunction tracerFunc) {
  Function* func = nullptr;
  std::string key;
  if (tracerFunc == TracerFunction::Init)
    key = "TracerInit";
  else if (tracerFunc == TracerFunction::Record)
    key = "TracerRecord";
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