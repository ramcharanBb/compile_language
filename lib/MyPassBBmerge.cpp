
#include "MyPassBBmerge.h"


llvm::PreservedAnalyses MyPassBBmerge::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    for(llvm::BasicBlock &BB : F){
       llvm::outs() << BB.empty(); 
    }
    return llvm::PreservedAnalyses::all();
}