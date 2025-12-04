#ifndef MYPASS_BBMERGE_H
#define MYPASS_BBMERGE_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

// Basic block merging optimization pass
class MyPassBBmerge : public llvm::PassInfoMixin<MyPassBBmerge> {
public:
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
    static llvm::StringRef name() { return "MyPassBBmerge"; }
};

#endif // MYPASS_BBMERGE_H
