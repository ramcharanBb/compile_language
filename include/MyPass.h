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
private:
bool make_instruction_dead(llvm::Instruction *instr, 
                           std::set<llvm::Instruction*> &nextdelinstructs, 
                           llvm::TargetLibraryInfo &info);
bool elim_dead_code(llvm::Function &F, llvm::TargetLibraryInfo &info);

};


// llvm::PassPluginLibraryInfo getMyXPassPluginInfo() {
//     return {LLVM_PLUGIN_API_VERSION, "MyPass", LLVM_VERSION_STRING,
//             [](llvm::PassBuilder &PB) {
//                 PB.registerPipelineParsingCallback(
//                     [](llvm::StringRef Name, XPassManager &MPM,
//                        llvm::ArrayRef<PassBuilder::PipelineElement>) {
//                         if (Name == "MyPass") {
//                             FPM->add(MyPass());
//                             return true;
//                         }
//                         return false;
//                     });
//             }};
// }

// extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
// llvmGetPassPluginInfo() {
//     return getMyXPassPluginInfo();
// }


#endif // MYPASS_H