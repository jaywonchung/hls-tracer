#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"

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

  // TODO: Find branch instructions (for control flow analysis)
  // and add tracer functions with the corresponding llvm::DILocation *
  // that can be achieved via llvm::Instruction::getDebugLoc()

  // TODO1: Insert a creation call of global ControlFlowTracer.


  // TODO2: Using this value, call member function in our target code.
  for (auto& func: module.getFunctionList()) {
    if (func.getName().contains("sigma_n") == false) continue;
    
    for (auto &bb: func) {
      for (auto bi = bb.begin(), bend = bb.end(); bi != bend; bi++) {
        if (isa<BranchInst>(bi) == false) continue;
        auto increaseCountTracerFunc = getTracerFunction(TracerFunction::IncrementCount);
        std::vector<Value*> args;
        // TODO: Put arguments.
        Instruction* newInst = CallInst::Create(increaseCountTracerFunc, args);
        bi->insertBefore(newInst);
        changed = true;
      }
    }

    break;
  }

  return changed;
}

int ControlFlowTracePass::getTracerFunctions(Module::FunctionListType& functions) {
  int function_num = 0;
  for (auto& func: functions) {
    if (func.getName().contains("ControlFlowTracer") == false) continue;
    tracerFunctions.insert({func.getName(), &func});
  }
  return function_num;
}

Function* ControlFlowTracePass::getTracerFunction(const TracerFunction tracerFunc) {
  Function* func = nullptr;
  std::string key;
  if (tracerFunc == TracerFunction::IncrementCount) key = "incrementControlFlowCount";
  else if (tracerFunc == TracerFunction::GetCount) key = "getCount";
  else if (tracerFunc == TracerFunction::Dump) key = "Dump";
  else return nullptr;
  
  auto it = std::find_if(tracerFunctions.begin(), tracerFunctions.end(),
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