#ifndef SEMANTIC_ANALYSIS
#define SEMANTIC_ANALYSIS

#include<iostream>
#include<string>
#include<memory>
#include<optional>

#include "ast.h"

class SemanticAnalysis {
    std::vector<std::unique_ptr<FunctionDecl>> TopLevel;
    std::vector<std::vector<ResolvedDecl *>> scopes; 
    std::vector<std::string> diagnostics;
    
    void error(SourceLocation location,std::string_view message){
    const auto& [file, line, col] = location;
    std::ostringstream oss;
    oss << file << ':' << line << ':' << col << ": error: " << message;
    diagnostics.push_back(oss.str());
    std::cerr << oss.str() << "\n";
    }

    

    ResolvedDecl *lookupDecl (const std::string id){
        for (auto it =scopes.begin() ; it != scopes.end();++it){
            for(auto &&decl: *it){
                if(decl->identifier==id){
                    return decl;
                }
            }
        }
        return nullptr;
    }
    
    std::optional<Type> resolveType(const std::string &typeSpecifier){
        if (typeSpecifier == "void") return Type::VOID;
        if (typeSpecifier == "string") return Type::STRING;
        if (typeSpecifier == "number") return Type::NUMBER;
        return std::nullopt;
    }
    std::unique_ptr<ResolvedDeclRefExpr> resolveDeclRefExpr(const DeclRefExpr &DRE){
        ResolvedDecl *decl = lookupDecl(DRE.identifier);
        if (!decl) {
            error(DRE.location, "symbol '" + DRE.identifier + "' not found");
            return nullptr;
        }
        return std::make_unique<ResolvedDeclRefExpr>(DRE.location, decl);
    }

    std::unique_ptr<ResolvedPrintExpr> resolvePrintExpr( const PrintExpr &printexpr){
      std::vector<std::unique_ptr<ResolvedExpr>> resolvedArguments;
      for (auto &&arg : printexpr.args){
        auto resolvedArg = resolveExpr(*arg);
        if (!resolvedArg) {                 
            return nullptr;
        }
        resolvedArguments.emplace_back(std::move(resolvedArg));
      }
       if (resolvedArguments.size() != printexpr.args.size()) {
            error(printexpr.location, "missing arguments in the print statement");
            return nullptr;
       }

    return std::make_unique<ResolvedPrintExpr>(
        printexpr.location,std::move(resolvedArguments));

    }

    std::unique_ptr<ResolvedCallExpr> resolveCallExpr( const CallExpr &cexpr){
        auto resolvedCallee = resolveDeclRefExpr(*cexpr.identifier);
        if(!resolvedCallee){
            return nullptr;
        }
        auto resolvedFunctionDecl = dynamic_cast<ResolvedFunctionDecl *>(resolvedCallee->decl);
        if (!resolvedFunctionDecl) {
            error(cexpr.location, "calling non-function element");
            return nullptr;
        }
      std::vector<std::unique_ptr<ResolvedExpr>> resolvedArguments;
      int idx =0;
      for (auto &&arg : cexpr.arguments){
        if(auto resolvedarg= resolveExpr(*arg)){
            if(resolvedarg->type != resolvedFunctionDecl->params[idx]->type){ 
                error(resolvedarg->location,"unexpected type of argument in " + resolvedFunctionDecl->identifier + " function call");
                return nullptr;
            }
            resolvedArguments.emplace_back(std::move(resolvedarg));
        }
        else{
            return nullptr;
        }
        idx++;
      }
       if (resolvedArguments.size() != resolvedFunctionDecl->params.size()) {
            error(cexpr.location, "missing arguments from function call");
            return nullptr;
       }

    return std::make_unique<ResolvedCallExpr>(
        cexpr.location, resolvedFunctionDecl, std::move(resolvedArguments));

    }

    std::unique_ptr<ResolvedExpr> resolveExpr(const Expr &expr){
        if(auto stringLiteral= dynamic_cast<const StringLiteral *> (&expr)){
            return std::make_unique<ResolvedStringLiteral>(stringLiteral->location,
                                                     stringLiteral->value);
        }
        if(auto numberLiteral= dynamic_cast<const NumberLiteral *> (&expr)){
            return std::make_unique<ResolvedNumberLiteral>(numberLiteral->location,
                                                     numberLiteral->value);
        }
        if(auto dre= dynamic_cast<const DeclRefExpr *> (&expr)){
            return resolveDeclRefExpr(*dre);
        }
        if(auto printexpr= dynamic_cast<const PrintExpr *> (&expr)){
            return resolvePrintExpr(*printexpr);
        }
        if(auto cexpr= dynamic_cast<const CallExpr *> (&expr)){
            return resolveCallExpr(*cexpr);
        }
        return nullptr;
    }

    std::unique_ptr<ResolvedStmt> resolveStmt(const Stmt &stmt){
        return resolveExpr(dynamic_cast<const Expr &>(stmt));
    }

    std::unique_ptr<ResolvedBlock> resolveBody(const Block &body){
        std::vector<std::unique_ptr<ResolvedStmt>> resolvedstatements;

        for(auto &&stmt : body.statements){
            if(auto resolvedstmt = resolveStmt(*stmt)){
                resolvedstatements.emplace_back(std::move(resolvedstmt));
            }
            else{
                return nullptr;
            }
        }
        return std::make_unique<ResolvedBlock>(body.location,std::move(resolvedstatements));        
    }

    std::unique_ptr<ResolvedParamDecl> resolveParamDecl(const ParamDecl &param){
    std::optional<Type> param_type= resolveType(param.type);
    if(!param_type){
        error(param.location,std::string{"parameter '"} +
                                       param.identifier + "' has invalid '" +
                                       param.type + "' type");
        return nullptr;
    }
    if(lookupDecl(param.identifier)){
        error(param.location, std::string{"parameter '"} +
                                       param.identifier + "' redeclared");
        return nullptr;
    }
    return std::make_unique<ResolvedParamDecl>(param.location,param.identifier,*param_type);
   }

    std::unique_ptr<ResolvedFunctionDecl> resolveFunctionWithoutBody(const FunctionDecl &function){
        std::vector<std::unique_ptr<ResolvedParamDecl>> resolvedParams;
        for (auto &&param: function.params){
            if(auto resolvedparam = resolveParamDecl(*param)){
                resolvedParams.emplace_back(std::move(resolvedparam));
            }else{
                return nullptr;
            }
        }
        std::optional<Type> type= resolveType(function.funtype);
        return std::make_unique<ResolvedFunctionDecl> (function.location, function.identifier, *type,std::move(resolvedParams),nullptr);

    }


    public:
    SemanticAnalysis(std::vector<std::unique_ptr<FunctionDecl>> TopLevel):TopLevel(std::move(TopLevel)){}
    std::vector<ResolvedFunctionDecl> resolve(){
        scopes.emplace_back();

        std::vector<ResolvedFunctionDecl> resolvedFunctions; 
        resolvedFunctions.reserve(TopLevel.size());

        for (auto &&function:TopLevel){
                if(auto resolvedFunction = resolveFunctionWithoutBody(*function)){
                    resolvedFunctions.emplace_back(std::move(*resolvedFunction));
                    scopes.back().emplace_back(&resolvedFunctions.back());
             }
        }
        if(resolvedFunctions.size()!= TopLevel.size()){
            return {};
        }
        for (size_t i=0; i< TopLevel.size();++i){
            scopes.emplace_back();
            for(auto &&param:resolvedFunctions[i].params)
                scopes.back().emplace_back(param.get());
            
            if(auto resolvedbody = resolveBody(*TopLevel[i]->body))
                resolvedFunctions[i].body =std::move(resolvedbody);
            else 
              return {};
            
            scopes.pop_back();  
            }
        return std::move(resolvedFunctions);
    }
    
};
#endif 