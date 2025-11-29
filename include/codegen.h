#ifndef CODEGEN_H
#define CODEGEN_H

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/PassManager.h" 
#include "llvm/Passes/PassBuilder.h" 
#include "llvm/Transforms/InstCombine/InstCombine.h" 
#include "llvm/Analysis/InstructionSimplify.h" 
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast.h"

class Codegen : public ASTVisitor{
    private:
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::map<std::string, llvm::Value *> NamedValues; 
    llvm::Value* lastValue = nullptr;
    std::map<std::string, llvm::Value*> formatStringCache;
    
    std::unique_ptr<llvm::FunctionPassManager> TheFPM;
    std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
    std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
    std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
    std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
        llvm::Type *GenerateType(std::string type) {
    if (type == "number") {
        return llvm::Type::getDoubleTy(*TheContext);
    } else if (type == "string") {
        return llvm::PointerType::get(*TheContext, 0); 
    } else if (type == "void") {
        return llvm::Type::getVoidTy(*TheContext);
    }
    return nullptr; 
    }

    void logError(const char* str);
  
    public:
    Codegen();
    void generate(std::vector<std::unique_ptr<FunctionDecl>> & program);
    llvm::Module* getModule() { return TheModule.get(); }

    void visit(NumberLiteral& node) override;
    void visit(StringLiteral& node) override ;
    void visit(Block& node) override;
    void visit(DeclRefExpr& node) override;
    void visit(CallExpr& node) override; 
    void visit(Stmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(Expr& node) override;

    void visit(FunctionDecl& node) override;
    void visit(ParamDecl& node) override;
    void visit(Decl& node) override;
    void visit(VariableDecl& node) override;
    void visit(PrintExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(AssignmentExpr& node) override;
    
};

#endif
