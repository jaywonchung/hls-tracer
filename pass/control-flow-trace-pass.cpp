#include <algorithm>
#include <cstdlib>
#include <map>
#include <string>
#include <utility>
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
  Finish,
};

template <typename T>
void assert_(T val, const char *message) {
  if (!val) {
    errs() << message << '\n';
    exit(1);
  }
}

struct ControlFlowTracePass : public ModulePass {
  static char ID;
  ControlFlowTracePass();
  virtual bool runOnModule(Module& module) override;

 private:
  Function* getTracerFunction(const TracerFunction tracerFunc);
  int getTracerFunctions(Module::FunctionListType& functions);

  std::pair<Instruction*, DILocation*> getInstructionLocationInfo(
      const BasicBlock* bb);

 private:
  std::map<std::string, Function*> tracerFunctions;
};

ControlFlowTracePass::ControlFlowTracePass() : ModulePass(ID) {}

bool ControlFlowTracePass::runOnModule(Module& module) {
  errs() << "Entered module " << module.getName() << ".\n";
  getTracerFunctions(module.getFunctionList());

  // Jae-Won: Get the name of the top-level function.
  const char* top_func_name = std::getenv("HLS_TRACER_TOP_FUNCTION");
  assert_(top_func_name, "Environment variable HLS_TRACER_TOP_FUNCTION is not set.");
  errs() << "Using top-level function '" << top_func_name << "'.\n";

  IRBuilder<> builder(module.getContext());

  // Insu: Use llvm::IRBuilder to create a call and insert it.
  for (auto& func : module.getFunctionList()) {
    auto fname = func.getName();

    // Skip functions from the control flow tracer, LLVM, and Vitis HLS.
    if (fname.contains("ControlFlowTracer")
        || fname.contains("llvm.dbg.declare")
        || fname.contains("SpecArrayDimSizez")) {
      continue;
    }

    // Found the top level function.
    if (fname.contains(top_func_name)) {
      /**
       * Inject the init tracer function call at the beginning.
       * To do so, we must first figure out the size of the input trace array.
       * This information can be parsed from Vitis HLS's custom clang argument
       * attribute 'fpga.decayed.dim.hint'.
       */
      int array_size = 0;
      auto param_attr = func.getAttributes().getParamAttr(0, "fpga.decayed.dim.hint");
      bool failed = param_attr.getValueAsString().getAsInteger(10, array_size);
      assert_(!failed, "Failed to parse integer from 'fpga.decayed.dim.hint' attribute.");
      errs() << "Trace array size is " << array_size << ".\n";

      // Insert init function call.
      auto initTracerFunc = getTracerFunction(TracerFunction::Init);
      assert_(initTracerFunc, "Cannot find the init tracer function!");
      auto fi = func.getBasicBlockList().begin()->getFirstInsertionPt();

      ArrayRef<Value*> args = {builder.getInt32(array_size)};
      builder.SetInsertPoint(&*fi);
      builder.CreateCall(initTracerFunc, args);

      errs() << "Inserted init function in the top-level function.\n";

      /**
       * Check whether there are return statements.
       * Inject a finish tracer function call before each ret statement.
       * This injction is for writing how many traces are inserted in DRAM.
       */
      for (auto& bb : func) {
        for (auto& inst : bb) {
          if (isa<ReturnInst>(&inst) == false)
            continue;

          auto finishTracerFunc = getTracerFunction(TracerFunction::Finish);
          assert_(finishTracerFunc, "Cannot find the finish tracer function!");

          ArrayRef<Value*> args = {func.getArg(0)};

          builder.SetInsertPoint(&inst);
          builder.CreateCall(finishTracerFunc, args);

          errs() << "Inserted finish function.\n";
        }
      }
    }

    /**
     * Inject record functions.
     *
     * Algorithm: Figure out candidate BBs where we would like to insert calls.
     * First, store all successor BBs of BBs that end with a conditional branch.
     * Recording control flow at all these branches are sufficient to record the
     * control flow.
     * Next, we remove BBs that end with a conditional branch themselves. These BBs
     * are essentially redundant because every control flow that reaches these BBs
     * are tracked by their successor BBs.
     */
    std::vector<BasicBlock*> record_candidate_bbs;

    // First state: add all successor BBs of BBs with cmp+br.
    for (auto& bb : func) {
      /**
       * https://llvm.org/docs/LangRef.html#terminator-instructions
       * Every block ends with a terminator instruction (== last instruction).
       */
      auto termi = dyn_cast<BranchInst>(bb.getTerminator());
      if (termi && termi->isConditional()) {
        for (auto succ : successors(&bb)) {
          record_candidate_bbs.push_back(succ);
        }
      }
    }

    // Second stage: remove BBs if it ends with a conditional branch.
    auto remove = std::remove_if(
        record_candidate_bbs.begin(), record_candidate_bbs.end(),
        [](BasicBlock* bb) {
          auto termi = dyn_cast<BranchInst>(bb->getTerminator());
          return termi && termi->isConditional();
        });
    record_candidate_bbs.erase(remove, record_candidate_bbs.end());

    // Insert tracer function call at the first location of each target BB.
    auto recordTracerFunc = getTracerFunction(TracerFunction::Record);
    assert_(recordTracerFunc, "Cannot find the record tracer function!");
    for (auto bb : record_candidate_bbs) {
      auto inst = getInstructionLocationInfo(bb);

      ArrayRef<Value*> args = {func.getArg(0),
                               builder.getInt32(inst.second->getLine()),
                               builder.getInt32(inst.second->getColumn())};

      builder.SetInsertPoint(inst.first);
      builder.CreateCall(recordTracerFunc, args);

      errs() << "Inserted record function at " << inst.second->getFilename()
             << ":" << inst.second->getLine() << ":" << inst.second->getColumn()
             << "\n";
    }
  }

  return true;
}

// Find the first instruction beginning from the top of the given basic block
// that has a debug location. Recursively find successive basic blocks if
// none is found in the current basic block.
std::pair<Instruction*, DILocation*>
ControlFlowTracePass::getInstructionLocationInfo(const BasicBlock* bb) {
  // First search through this BB.
  for (auto bi = bb->begin(), bend = bb->end(); bi != bend; bi++) {
    auto loc = bi->getDebugLoc().get();
    if (loc) {
      return {const_cast<Instruction*>(&*bi), loc};
    }
  }

  // If no instruction with source info found,
  // borrow instruction information from successor BB.
  // Instruction location will be the first instruction in this BB.
  auto* bb_succ = bb->getSingleSuccessor();
  assert_(bb_succ, "Multiple (or zero) successors encountered in getInstructionLocationInfo.");
  std::pair<Instruction*, DILocation*> info;
  info = getInstructionLocationInfo(bb_succ);
  const Instruction* inst = &*bb->getFirstInsertionPt();
  return {const_cast<Instruction*>(inst), info.second};
}

// Find control flow tracer functions from the current module and
// cache their pointers in a map.
int ControlFlowTracePass::getTracerFunctions(
    Module::FunctionListType& functions) {
  int function_num = 0;
  for (auto& func : functions) {
    auto fname = func.getName();
    if (fname.contains("controlFlowTracer") == false)
      continue;
    tracerFunctions.insert({fname, &func});
    errs() << "Function: " << fname << " added into tracer functions\n";
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
  else if (tracerFunc == TracerFunction::Finish)
    key = "TracerFinish";
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
                                            "Instrument the source code with control flow tracing functions.",
                                            false,
                                            false);
