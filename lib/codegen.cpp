
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include <system_error> 


#include "codegen.h"
#include "MyPass.h"
#include "MyPassBBmerge.h"
#include "SEPass.h"

Codegen::Codegen(){
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("ram-compiler", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
    
    TheFPM = std::make_unique<llvm::FunctionPassManager>();
    TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
    TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
    TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();

    llvm::PassBuilder PB;
    PB.registerLoopAnalyses(*TheLAM);
    PB.registerFunctionAnalyses(*TheFAM);
    PB.registerCGSCCAnalyses(*TheCGAM);
    PB.registerModuleAnalyses(*TheMAM);

    PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);

    TheFPM->addPass(llvm::PromotePass()); 

    // TheFPM->addPass(llvm::GVNPass());  
    TheFPM->addPass(MyPass()); 
    TheFPM->addPass(MyPassBBmerge());
    TheFPM->addPass(SEPass());

}

void logerror(const char* str){
    std::cerr <<"Codegen error"<<str<<std::endl;
}

void Codegen::generate(std::vector<std::unique_ptr<FunctionDecl>> & functions){
       for(auto &func : functions){
        func->accept(*this);
       }
       std::error_code EC;
    llvm::raw_fd_ostream OS("Output.ll", EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Error opening file: " << EC.message() << "\n";
        return;
    }

    TheModule->print(OS, nullptr); 
    OS.flush(); 

    llvm::outs() << "LLVM IR written to " << "Output.ll" << "\n";

}

void Codegen::visit(FunctionDecl& node){
    std::vector<llvm::Type *> paramTypes;
    llvm::Type* funtype=GenerateType(node.funtype);
    
    for (auto &&param : node.params)
      paramTypes.emplace_back(GenerateType(param->type));
    
    llvm::FunctionType* functype = llvm::FunctionType::get(funtype,paramTypes,false);
    auto function= llvm::Function::Create(functype,llvm::Function::ExternalLinkage, node.identifier,*TheModule);
    
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*TheContext, "", function);
    Builder->SetInsertPoint(entry);

    int idx=0;
    NamedValues.clear();
    for(auto &&args :function->args()){
     args.setName(node.params[idx]->identifier);
     NamedValues[node.params[idx]->identifier] = &args;
     ++idx;
    }
    node.body->accept(*this);
    if (funtype->isVoidTy() && !Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateRetVoid(); 
    }
    llvm::verifyFunction(*function);
    TheFPM->run(*function,*TheFAM);
}

void Codegen::visit(Block& node){
     for(auto &stmt : node.statements){
        stmt->accept(*this);
     }
}

void Codegen::visit(DeclRefExpr& node){
     llvm::Value *value= NamedValues[node.identifier];
     if(!value){
        logerror("Unknown variable name identified");
        lastValue=nullptr;
        return;
     }
     if (llvm::isa<llvm::AllocaInst>(value)) {
         lastValue = Builder->CreateLoad(
             llvm::cast<llvm::AllocaInst>(value)->getAllocatedType(),
             value,
             node.identifier
         );
     } else {
         lastValue = value;
     }
}

void Codegen::visit(ReturnStmt& node) {
    if(node.expr){
        node.expr->accept(*this);
        if(lastValue){
            lastValue=Builder->CreateRet(lastValue);
        }
        else {
            lastValue= Builder->CreateRet(llvm::ConstantInt::get(*TheContext, llvm::APInt(64, 0)));
        }
    }
    else{
        Builder->CreateRetVoid(); 
    }
    return;
}

void Codegen::visit(PrintExpr& node){
     std::vector<llvm::Value*> argsP;
     std::string formatStr = "";
    for(auto &arg:node.args){
        arg->accept(*this);
        if(lastValue){
            argsP.push_back(lastValue);
            if (lastValue->getType()->isDoubleTy()) {
                formatStr += "%f ";
            } else if (lastValue->getType()->isIntegerTy()) {
                formatStr += "%d ";
            } else if (lastValue->getType()->isPointerTy()) {
                formatStr += "%s ";
            }
        }
    }
    formatStr += "\n";
   llvm::Value* formatStrVar;
        if (formatStringCache.count(formatStr)) {
            formatStrVar = formatStringCache[formatStr];
        } else {
            formatStrVar = Builder->CreateGlobalString(formatStr, "fmt", 0, TheModule.get());
            formatStringCache[formatStr] = formatStrVar;
        }
    argsP.insert(argsP.begin(), formatStrVar);
    
    llvm::FunctionType *printfType = llvm::FunctionType::get(
        llvm::IntegerType::getInt32Ty(*TheContext), 
        llvm::PointerType::get(*TheContext, 0),
        true 
    );
    llvm::FunctionCallee calleeF = TheModule->getOrInsertFunction("printf", printfType);
    Builder->CreateCall(calleeF, argsP, "printfCall");
    lastValue = nullptr;
}

void Codegen::visit(CallExpr& node){
    auto *fidentifier= TheModule->getFunction(node.identifier);

    if(!fidentifier){
        logerror("Undefined function call");
        lastValue=nullptr;
        return;
    }
    if(fidentifier->arg_size() !=  node.arguments.size()){
        logerror("incorrect no of parameters passsed to the function");
        lastValue=nullptr;
        return;
    }
    std::vector<llvm::Value *> argsC;
    for(auto &arg : node.arguments){
        arg->accept(*this);
        if(lastValue){
            argsC.push_back(lastValue);
        }
    }

    if (fidentifier->getReturnType()->isVoidTy())
        lastValue = Builder->CreateCall(fidentifier, argsC);
    else
        lastValue = Builder->CreateCall(fidentifier, argsC, "calltmp");
}

void Codegen::visit(NumberLiteral& node) {
    if (node.value.find('.') != std::string::npos) {
        lastValue = llvm::ConstantFP::get(*TheContext, 
            llvm::APFloat(llvm::APFloat::IEEEdouble(), llvm::StringRef(node.value))
        );
    } else {
        lastValue = llvm::ConstantInt::get(*TheContext, 
            llvm::APInt(64, std::stoll(node.value))
        );
    }
}


void Codegen::visit(StringLiteral& node){
   lastValue = Builder->CreateGlobalString(node.value, "str", 0, TheModule.get());
}

void Codegen::visit(Stmt& node) {
}

void Codegen::visit(Expr& node) {
}

void Codegen::visit(ParamDecl& node) {
}


void Codegen::visit(BinaryExpr& node) {
    node.left->accept(*this);
    llvm::Value* left = lastValue;
    
    node.right->accept(*this);
    llvm::Value* right = lastValue;
    
    if (!left || !right) {
        lastValue = nullptr;
        return;
    }

    bool isDouble = left->getType()->isDoubleTy() || right->getType()->isDoubleTy();

    if (isDouble) {
        if (!left->getType()->isDoubleTy())
             left = Builder->CreateUIToFP(left, llvm::Type::getDoubleTy(*TheContext), "casttmp");
        if (!right->getType()->isDoubleTy())
             right = Builder->CreateUIToFP(right, llvm::Type::getDoubleTy(*TheContext), "casttmp");
    }

    switch (node.op) {
        case TokenKind::plus:
            if (isDouble)
                lastValue = Builder->CreateFAdd(left, right, "addtmp");
            else
                lastValue = Builder->CreateAdd(left, right, "addtmp");
            break;
        case TokenKind::minus:
            if (isDouble)
                lastValue = Builder->CreateFSub(left, right, "subtmp");
            else
                lastValue = Builder->CreateSub(left, right, "subtmp");
            break;
        case TokenKind::mul:
            if (isDouble)
                lastValue = Builder->CreateFMul(left, right, "multmp");
            else
                lastValue = Builder->CreateMul(left, right, "multmp");
            break;
        case TokenKind::slash:
            if (isDouble)
                lastValue = Builder->CreateFDiv(left, right, "divtmp");
            else
                lastValue = Builder->CreateSDiv(left, right, "divtmp");
            break;
        case TokenKind::percent:
            if (isDouble)
                lastValue = Builder->CreateFRem(left, right, "modtmp");
            else
                lastValue = Builder->CreateSRem(left, right, "modtmp");
            break;
        case TokenKind::lessthan:
            if (isDouble)
                lastValue = Builder->CreateFCmpOLT(left, right, "cmptmp");
            else
                lastValue = Builder->CreateICmpSLT(left, right, "cmptmp");
            break;
        case TokenKind::greaterthan:
            if (isDouble)
                lastValue = Builder->CreateFCmpOGT(left, right, "cmptmp");
            else
                lastValue = Builder->CreateICmpSGT(left, right, "cmptmp");
            break;
        case TokenKind::less_equal:
            if (isDouble)
                lastValue = Builder->CreateFCmpOLE(left, right, "cmptmp");
            else
                lastValue = Builder->CreateICmpSLE(left, right, "cmptmp");
            break;
        case TokenKind::great_equal:
            if (isDouble)
                lastValue = Builder->CreateFCmpOGE(left, right, "cmptmp");
            else
                lastValue = Builder->CreateICmpSGE(left, right, "cmptmp");
            break;
        case TokenKind::doublequal:
            if (isDouble)
                lastValue = Builder->CreateFCmpOEQ(left, right, "cmptmp");
            else
                lastValue = Builder->CreateICmpEQ(left, right, "cmptmp");
            break;
        case TokenKind::not_equal:
            if (isDouble)
                lastValue = Builder->CreateFCmpONE(left, right, "cmptmp");
            else
                lastValue = Builder->CreateICmpNE(left, right, "cmptmp");
            break;
        default:
            logerror("Unknown binary operator");
            lastValue = nullptr;
            break;
    }
}

void Codegen::visit(Decl& node) {
}

void Codegen::visit(VariableDecl& node) {
    llvm::Type* varType = GenerateType(node.type);
    llvm::AllocaInst* alloca = Builder->CreateAlloca(varType, nullptr, node.identifier);
    NamedValues[node.identifier] = alloca;
    if (node.initializer) {
        node.initializer->accept(*this);
        if (lastValue) {
            Builder->CreateStore(lastValue, alloca);
        }
    }
    lastValue = nullptr;
}

void Codegen::visit(AssignmentExpr& node) {
    llvm::Value* variable = NamedValues[node.target];
    if (!variable) {
        logerror("Unknown variable in assignment");
        lastValue = nullptr;
        return;
    }
    node.value->accept(*this);
    if (!lastValue) {
        logerror("Failed to generate RHS of assignment");
        return;
    }
    Builder->CreateStore(lastValue, variable);
}

void Codegen::visit(BooleanLiteral& node) {
    double value = node.value ? 1.0 : 0.0;
    lastValue = llvm::ConstantFP::get(*TheContext, llvm::APFloat(value));
}

void Codegen::visit(IfStmt& node) {
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;
    
    if (!condValue) {
        logerror("Failed to generate if condition");
        return;
    }
    
    // Convert condition to boolean by comparing with 0.0
    // Convert condition to boolean
    llvm::Value* condBool;
    if (condValue->getType()->isDoubleTy()) {
        condBool = Builder->CreateFCmpONE(
            condValue, 
            llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), 
            "ifcond"
        );
    } else if (condValue->getType()->isIntegerTy()) {
        condBool = Builder->CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(condValue->getType(), 0),
            "ifcond"
        );
    } else {
        logerror("Invalid type for if condition");
        return;
    }
    
    llvm::Function* function = Builder->GetInsertBlock()->getParent();
    
    // Create blocks for then, else, and merge
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*TheContext, "then", function);
    llvm::BasicBlock* elseBB = node.elseBlock ? 
        llvm::BasicBlock::Create(*TheContext, "else") : nullptr;
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");
    
    // Branch based on condition
    if (elseBB) {
        Builder->CreateCondBr(condBool, thenBB, elseBB);
    } else {
        Builder->CreateCondBr(condBool, thenBB, mergeBB);
    }
    
    // Generate then block
    Builder->SetInsertPoint(thenBB);
    node.thenBlock->accept(*this);
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(mergeBB);
    }
    
    // Generate else block if it exists
    if (elseBB) {
        function->insert(function->end(), elseBB);
        Builder->SetInsertPoint(elseBB);
        node.elseBlock->accept(*this);
        if (!Builder->GetInsertBlock()->getTerminator()) {
            Builder->CreateBr(mergeBB);
        }
    }
    
    // Generate merge block
    function->insert(function->end(), mergeBB);
    Builder->SetInsertPoint(mergeBB);
    
    lastValue = nullptr;
}

void Codegen::visit(WhileStmt& node) {
    llvm::Function* function = Builder->GetInsertBlock()->getParent();
    
    // Create blocks for condition, body, and after loop
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*TheContext, "whilecond", function);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*TheContext, "whilebody");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(*TheContext, "afterwhile");
    
    // Branch to condition block
    Builder->CreateBr(condBB);
    
    // Generate condition block
    Builder->SetInsertPoint(condBB);
    node.condition->accept(*this);
    llvm::Value* condValue = lastValue;
    
    if (!condValue) {
        logerror("Failed to generate while condition");
        return;
    }
    
    // Convert condition to boolean
    llvm::Value* condBool;
    if (condValue->getType()->isDoubleTy()) {
        condBool = Builder->CreateFCmpONE(
            condValue, 
            llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)), 
            "whilecond"
        );
    } else if (condValue->getType()->isIntegerTy()) {
        condBool = Builder->CreateICmpNE(
            condValue,
            llvm::ConstantInt::get(condValue->getType(), 0),
            "whilecond"
        );
    } else {
        logerror("Invalid type for while condition");
        return;
    }
    
    Builder->CreateCondBr(condBool, bodyBB, afterBB);
    
    // Generate body block
    function->insert(function->end(), bodyBB);
    Builder->SetInsertPoint(bodyBB);
    node.body->accept(*this);
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(condBB);  // Loop back to condition
    }
    
    // Generate after block
    function->insert(function->end(), afterBB);
    Builder->SetInsertPoint(afterBB);
    
    lastValue = nullptr;
}
