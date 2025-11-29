#include "MyPass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"


llvm::PreservedAnalyses MyPass::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    int instructionCount = 0;
    for (auto BB_iter = F.begin(); BB_iter != F.end(); ++BB_iter) {
        for (auto I_iter = BB_iter->begin(); I_iter != BB_iter->end(); ++I_iter) {
            instructionCount++;
        }
    }
    llvm::outs() << "Function '" << F.getName() 
                 << "' has " << instructionCount 
                 << " instructions.\n";
    return llvm::PreservedAnalyses::all();
}
