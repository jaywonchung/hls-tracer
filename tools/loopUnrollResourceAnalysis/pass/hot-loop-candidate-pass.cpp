#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/XILINXLoopInfoUtils.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include <tuple>

using namespace llvm;

struct HotLoopAnalysisPass : public FunctionPass {
  static char ID;
  HotLoopAnalysisPass() : FunctionPass(ID) {};
  virtual bool runOnFunction(Function& func) override;

  void getAnalysisUsage(AnalysisUsage& au) const override {
    au.setPreservesCFG();
    au.addRequired<LoopInfoWrapperPass>();
  }
  
 private:
  std::tuple<std::string, int, int> analyzeLoop(const Loop& loop);
};

bool HotLoopAnalysisPass::runOnFunction(Function& func) {
  errs() << "Entered function " << func.getName() << ".\n";

  std::error_code ec;
  std::string filename("./loop-analysis.txt");
  raw_fd_ostream fstream(filename, ec, sys::fs::OpenFlags::F_RW);

  LoopInfo& loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  for (auto loop : loopInfo) {
    auto loop_info = analyzeLoop(*loop);
    fstream << std::get<0>(loop_info) << " " << std::get<1>(loop_info) << " " << std::get<2>(loop_info) << "\n";
  }

  fstream.close();
  return false;
}

std::tuple<std::string, int, int> HotLoopAnalysisPass::analyzeLoop(const Loop& loop) {
  auto loopname = getLoopName(&loop);
  if (!loopname.hasValue()) {
    errs() << "Found a loop without a name:\n" << loop << '\n';
    exit(1);
  }
  auto name = loopname.getValue();
  unsigned int line_min = INT_MAX, line_max = 0;

  for (auto bb : loop.getBlocks()) {
    for (auto ii = bb->begin(), iend = bb->end(); ii != iend; ii++) {
      auto loc = ii->getDebugLoc().get();
      if (loc == nullptr) continue;
      auto line = loc->getLine();
      if (line > line_max) line_max = line;
      if (line < line_min) line_min = line;
    }
  }

  /* return {name, line_min, line_max}; */
  return std::make_tuple(name, line_min, line_max);
}

char HotLoopAnalysisPass::ID = 0;
static RegisterPass<HotLoopAnalysisPass> X("hotloopcandidate",
                                        "Analyzes top-level loops and records their names and min/max line number.",
                                        false,
                                        false);
