#ifndef MYPASS_H
#define MYPASS_H

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"


class MyPass : public llvm::PassInfoMixin<MyPass> {
public:
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &FAM);
    static llvm::StringRef name() { return "MyPass"; }
};


// extern "C" LLVM_ATTRIBUTE_WEAK llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
//     return {
//         LLVM_PLUGIN_API_VERSION,
//         "MyPass",
//         LLVM_VERSION_STRING,
//         [](llvm::PassBuilder &PB) {
//             PB.registerPipelineParsingCallback(
//                 [](llvm::StringRef Name,
//                    llvm::FunctionPassManager &FPM,
//                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
//                     if (Name == "mypass") {
//                         FPM.addPass(MyPass());
//                         return true;
//                     }
//                     return false;
//                 });
//         }
//     };
// }

#endif // MYPASS_H