#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/STLExtras.h" 
#include "MyPassBBmerge.h"
#include <vector>

llvm::PreservedAnalyses MyPassBBmerge::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    std::vector<llvm::BasicBlock *> blocksneed2del;
    bool changed = false;
    
    //constant conditional branches to unconditional branches
    for(llvm::BasicBlock &BB : F){
        llvm::Instruction *TerminatorBB = BB.getTerminator();
        if (!TerminatorBB) continue;
        if (auto *BI = llvm::dyn_cast<llvm::BranchInst>(TerminatorBB)) {
            if(BI->isConditional()){
                llvm::Value *Condition = BI->getCondition();
                if (auto *ConstCond = llvm::dyn_cast<llvm::ConstantInt>(Condition)) {
                  llvm::BasicBlock *TargetBB = ConstCond->isOne() ? BI->getSuccessor(0) : BI->getSuccessor(1); 
                    llvm::BasicBlock *DeadBB = ConstCond->isOne() ? BI->getSuccessor(1) : BI->getSuccessor(0);
                    llvm::BranchInst::Create(TargetBB, &BB);
                    DeadBB->removePredecessor(&BB);
                    BI->eraseFromParent();
                    changed = true;
                    
                    llvm::outs() << "Simplified constant branch in block: " << BB.getName() << "\n";
                }
            }
        }
    }
    
    //Merge blocks connected by unconditional branches
    for(llvm::BasicBlock &BB : llvm::make_early_inc_range(F)){
        llvm::Instruction *TerminatorBB = BB.getTerminator();
        if (!TerminatorBB) continue; 
        if (auto *BI = llvm::dyn_cast<llvm::BranchInst>(TerminatorBB)) {
            if(BI->isUnconditional()){
                llvm::BasicBlock *succesorBB = BB.getSingleSuccessor();
                if(succesorBB && &BB != succesorBB && &BB == succesorBB->getUniquePredecessor()){ 
                    llvm::BasicBlock::iterator InsertPos = TerminatorBB->getIterator();
                    while (!succesorBB->empty()) {
                        llvm::Instruction &Inst = succesorBB->front();
                        Inst.moveBefore(&*InsertPos); 
                    }
                    TerminatorBB->eraseFromParent();  
                    blocksneed2del.push_back(succesorBB);
                    changed = true;
                    llvm::outs() << "Merged blocks: " << BB.getName() << " and " << succesorBB->getName() << "\n";
                }
            }
        }
    }
    
    for(auto *delBB:blocksneed2del){ 
        delBB->eraseFromParent();
    }
    
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}
