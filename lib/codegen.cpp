
#include "codegen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/APFloat.h"


Codegen::Codegen(){
    TheContext = std::make_unique<llvm::LLVMContext>();
    TheModule = std::make_unique<llvm::Module>("ram-compiler", *TheContext);
    Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

void logerror(const char* str){
    std::cerr <<"Codegen error"<<str<<std::endl;
}

void Codegen::generate(std::vector<std::unique_ptr<FunctionDecl>> & functions){
       for(auto &func : functions){
        func->accept(*this);
       }
       TheModule->print(llvm::errs(),nullptr);
}

void Codegen::visit(FunctionDecl& node){
    std::vector<llvm::Type *> paramTypes;
    llvm::Type* funtype=GenerateType(node.funtype);
    
    for (auto &&param : node.params)
      paramTypes.emplace_back(GenerateType(param->type));
    
    llvm::FunctionType* functype = llvm::FunctionType::get(funtype,paramTypes,false);
    auto function= llvm::Function::Create(functype,llvm::Function::ExternalLinkage,node.identifier == "main"? "__main": node.identifier,*TheModule);
    
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
    llvm::verifyFunction(*function);
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
     lastValue=value;
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
    
    llvm::Function *calleeF = TheModule->getFunction("printf");
    if (!calleeF) {
        std::vector<llvm::Type*> args;
        args.push_back(llvm::PointerType::get(*TheContext, 0));
        llvm::FunctionType *printfType = llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(*TheContext), args, true);
        calleeF = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", TheModule.get());
    }
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
    lastValue = llvm::ConstantFP::get(*TheContext, 
        llvm::APFloat(llvm::APFloat::IEEEdouble(), llvm::StringRef(node.value))
    );
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
    
    // Arithmetic operations
    switch (node.op) {
        case TokenKind::plus:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFAdd(left, right, "addtmp");
            else
                logerror("Invalid types for addition");
            break;
        case TokenKind::minus:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFSub(left, right, "subtmp");
            else
                logerror("Invalid types for subtraction");
            break;
        case TokenKind::mul:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFMul(left, right, "multmp");
            else
                logerror("Invalid types for multiplication");
            break;
        case TokenKind::slash:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFDiv(left, right, "divtmp");
            else
                logerror("Invalid types for division");
            break;
        case TokenKind::percent:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFRem(left, right, "modtmp");
            else
                logerror("Invalid types for modulo");
            break;
        
        // Comparison operations
        case TokenKind::lessthan:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFCmpOLT(left, right, "cmptmp");
            else
                logerror("Invalid types for less than");
            break;
        case TokenKind::greaterthan:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFCmpOGT(left, right, "cmptmp");
            else
                logerror("Invalid types for greater than");
            break;
        case TokenKind::less_equal:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFCmpOLE(left, right, "cmptmp");
            else
                logerror("Invalid types for less than or equal");
            break;
        case TokenKind::great_equal:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFCmpOGE(left, right, "cmptmp");
            else
                logerror("Invalid types for greater than or equal");
            break;
        case TokenKind::doublequal:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFCmpOEQ(left, right, "cmptmp");
            else
                logerror("Invalid types for equality");
            break;
        case TokenKind::not_equal:
            if (left->getType()->isDoubleTy() && right->getType()->isDoubleTy())
                lastValue = Builder->CreateFCmpONE(left, right, "cmptmp");
            else
                logerror("Invalid types for not equal");
            break;
        default:
            logerror("Unknown binary operator");
            lastValue = nullptr;
            break;
    }
}

void Codegen::visit(Decl& node) {
}
