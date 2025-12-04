#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h" 
#include "SEPass.h"

unsigned SEPass::is_power_of_2(llvm::Value *operand){
        llvm::ConstantInt* CI = llvm::dyn_cast<llvm::ConstantInt>(operand);
        if (!CI) return 0;
        return CI->getValue().isPowerOf2()?CI->getValue().logBase2():0;
}

llvm::PreservedAnalyses SEPass::run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM){
    bool  changed =false;
    for (auto &BB: F){
    for (auto &BB: F){
          for (auto I = BB.begin(), E = BB.end(); I != E; ) {
             llvm::Instruction *Inst = &*I++;
             if (Inst->getOpcode() == llvm::Instruction::Mul) {
                llvm::Value *leftOperand = Inst->getOperand(0);
                llvm::Value *rightOperand = Inst->getOperand(1);
               
                unsigned leftShiftAmt = is_power_of_2(leftOperand);
                unsigned rightShiftAmt = is_power_of_2(rightOperand);

                llvm::Value *baseValue = nullptr;
                unsigned shiftAmount = 0;

                if (leftShiftAmt > 0) {
                    baseValue = rightOperand;
                    shiftAmount = leftShiftAmt;
                } else if (rightShiftAmt > 0) {
                    baseValue = leftOperand;  
                    shiftAmount = rightShiftAmt;
                }

                if (baseValue) {
                    if (!shiftAmount) continue;
                    llvm::Type* type = baseValue->getType();
                    llvm::Value* shiftAmountVal = llvm::ConstantInt::get(type, shiftAmount);
                    llvm::Instruction *newShiftInst = llvm::BinaryOperator::CreateShl(baseValue, shiftAmountVal, "shl_tmp");
                    llvm::ReplaceInstWithInst(Inst, newShiftInst);
                    changed = true;
                }
            }
        }
    }
    }

    if (changed) {
        return llvm::PreservedAnalyses::none();
    } else {
        return llvm::PreservedAnalyses::all();
    }
}
