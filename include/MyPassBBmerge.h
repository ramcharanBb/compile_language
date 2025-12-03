#ifndef BBMERGE_H
#define BBMERGE_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"


class MyPassBBmerge :public llvm::PassInfoMixin<MyPassBBmerge>{

    public:
    llvm::PreservedAnalyses run(llvm::Function &F,llvm::FunctionAnalysisManager &FAM);
    static llvm::StringRef name() { return "MyPassBBmerge"; };
};



#endif

