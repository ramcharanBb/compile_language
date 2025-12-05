#include <iostream>
#include <set>
#include "MyPass.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/raw_ostream.h"

bool MyPass::make_instruction_dead(llvm::Instruction *instr, 
                           std::set<llvm::Instruction*> &nextdelinstructs, 
                           llvm::TargetLibraryInfo &info) {
    if (llvm::isInstructionTriviallyDead(instr, &info)) {
        for (unsigned int i = 0; i != instr->getNumOperands(); i++) {
            llvm::Value *operand = instr->getOperand(i);
            instr->setOperand(i, nullptr);
            if (!operand->use_empty() || instr == operand) {
                continue;
            }
            
            if (auto operand_instr = llvm::dyn_cast<llvm::Instruction>(operand)) {
                if (llvm::isInstructionTriviallyDead(operand_instr, &info)) {
                    nextdelinstructs.insert(operand_instr);
                }
            }
        }
        instr->eraseFromParent();
        return true;
    } else {
        return false;
    }
}
                         
bool MyPass::elim_dead_code(llvm::Function &F, llvm::TargetLibraryInfo &info) {
    bool changed = false;
    std::set<llvm::Instruction *> nextdelinstructs;
    
    for (llvm::BasicBlock &BB : F) {
    for (llvm::Instruction &instr : llvm::make_early_inc_range(BB)) {
        if (!nextdelinstructs.count(&instr)) {
            changed |= make_instruction_dead(&instr, nextdelinstructs, info);
        }
    }}
    
    while (!nextdelinstructs.empty()) {
        auto it = nextdelinstructs.begin();
        llvm::Instruction *instr = *it;
        nextdelinstructs.erase(it);

        changed |= make_instruction_dead(instr, nextdelinstructs, info);
    }
    return changed;
}
 
llvm::PreservedAnalyses MyPass::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM) {
    if (elim_dead_code(F, FAM.getResult<llvm::TargetLibraryAnalysis>(F))) {
        return llvm::PreservedAnalyses::none(); 
    }
    return llvm::PreservedAnalyses::all();
}
