#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Analysis/XILINXLoopInfoUtils.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include <tuple>

using namespace llvm;

struct LoopAnalysisPass : public FunctionPass {
  static char ID;
  LoopAnalysisPass() : FunctionPass(ID) {};
  virtual bool runOnFunction(Function& func) override;

  void getAnalysisUsage(AnalysisUsage& au) const override {
    au.setPreservesCFG();
    au.addRequired<LoopInfoWrapperPass>();
  }
  
 private:
  std::tuple<std::string, int, int> analyzeLoop(const Loop& loop);
};

bool LoopAnalysisPass::runOnFunction(Function& func) {
  errs() << "Entered function " << func.getName() << ".\n";

  std::error_code ec;
  std::string filename("./loop-analysis-");
  filename += func.getName();
  raw_fd_ostream fstream(filename, ec, sys::fs::OpenFlags::F_RW);

  LoopInfo& loopInfo = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  for (auto loop : loopInfo) {
    auto loop_info = analyzeLoop(*loop);
    fstream << std::get<0>(loop_info) << " " << std::get<1>(loop_info) << " " << std::get<2>(loop_info) << "\n";
  }

  fstream.close();
  return false;
}

std::tuple<std::string, int, int> LoopAnalysisPass::analyzeLoop(const Loop& loop) {
  auto name = getLoopName(&loop).getValue();
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

  return std::make_tuple(name, line_min, line_max);
}

char LoopAnalysisPass::ID = 0;
static RegisterPass<LoopAnalysisPass> X("loopanalysis",
                                        "Analyze loops that finds out their name and location.",
                                        false,
                                        false);