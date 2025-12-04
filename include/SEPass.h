#ifndef SEPASS_H
#define SEPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"


class SEPass : public llvm::PassInfoMixin<SEPass>{
private:
  unsigned is_power_of_2(llvm::Value *operand);
  public:
  llvm::PreservedAnalyses run(llvm::Function &F,llvm::FunctionAnalysisManager &FAM);
  static llvm::StringRef name(){return "SEPass";}
};

#endif