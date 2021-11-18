#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include "llvm/IR/CFG.h"
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

#define MAX_ITEM_NUM 256

bool ControlFlowTracePass::runOnModule(Module& module) {
  errs() << "Entered module " << module.getName() << ".\n";
  getTracerFunctions(module.getFunctionList());

  // Jae-Won: Get the name of the top-level function.
  const char* top_func_name = std::getenv("HLS_TRACER_TOP_FUNCTION");
  assert(top_func_name &&
         "Environment variable HLS_TRACER_TOP_FUNCTION is not set.");
  errs() << "Using top-level function '" << top_func_name << "'.\n";

  IRBuilder<> builder(module.getContext());

  /**
   * Insu: targeting named controlFlowTracer (defined in
   * control-flow-trace-pass.cpp) pointerToTracer refers: ControlFlowTracer *
   * (value: &controlFlowTracer);
   *
   * Nov 13: Now tracer has been changed to C style, with no global tracer
   * instance, We don't have to store a pointer heading to the global variable.
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
    if (func.getName().contains(top_func_name) == false)
      continue;

    // Inject init function
    auto initTracerFunc = getTracerFunction(TracerFunction::Init);
    assert(initTracerFunc && "Cannot find a record tracer function!");
    auto fi = func.getBasicBlockList().begin()->getFirstInsertionPt();

    ArrayRef<Value*> args = {builder.getInt32(MAX_ITEM_NUM)};

    builder.SetInsertPoint(&*fi);
    builder.CreateCall(initTracerFunc, args);

    errs() << "Inserted init function in the top-level function.\n";

    /**
     * Inject record functions.
     * Algorithm: store all successor BBs of BBs, the last two instructions of
     * which are cmp and branch. This tracks all branches and control flow.
     * Next, we remove BBs from the list, the last two instructions of which are
     * cmp and branch.
     */
    std::vector<BasicBlock*> record_candidate_bbs;

    // First state: add all successor BBs of BBs with cmp+br.
    for (auto& bb : func) {
      /**
       * https://llvm.org/docs/LangRef.html#terminator-instructions
       * Every block ends with a terminator instruction (== last instruction).
       */
      auto termi = bb.getTerminator();
      if (isa<BranchInst>(termi) && termi->getPrevNode() &&
          isa<CmpInst>(termi->getPrevNode())) {
        for (auto succ : successors(&bb)) {
          record_candidate_bbs.push_back(succ);
        }
      }
    }

    // Second stage: remove BBs if one ends with cmp+br.
    auto remove = std::remove_if(
        record_candidate_bbs.begin(), record_candidate_bbs.end(),
        [](BasicBlock* bb) {
          auto termi = bb->getTerminator();
          return isa<BranchInst>(termi) && termi->getPrevNode() &&
                 isa<CmpInst>(termi->getPrevNode());
        });
    record_candidate_bbs.erase(remove, record_candidate_bbs.end());

    // Insert tracer function call at the first location of each target BB.
    auto recordTracerFunc = getTracerFunction(TracerFunction::Record);
    assert(initTracerFunc && "Cannot find a record tracer function!");
    for (auto& bb : func) {
      auto fi = bb.getFirstInsertionPt();

      // Segmentation fault occurs if we use
      // bb.getFirstInsertionPt()->getDebugLoc() or whatever.
      // TODO: fix this targeting the first valid instruction.
      DILocation* loc = bb.getTerminator()->getDebugLoc();
      assert(loc && "Cannot find debug location!");
      ArrayRef<Value*> args = {func.getArg(1),
                               builder.getInt32(loc->getLine()),
                               builder.getInt32(loc->getColumn())};

      builder.SetInsertPoint(&*fi);
      builder.CreateCall(recordTracerFunc, args);

      errs() << "Inserted record function at: " << loc->getFilename() << ":"
             << loc->getLine() << ":" << loc->getColumn() << "\n";
    }
    break;
  }

  return true;
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
