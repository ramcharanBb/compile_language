#ifndef SEMANTIC_ANALYSIS
#define SEMANTIC_ANALYSIS

#include <iostream>
#include <string>
#include <memory>
#include <optional>

#include "ast.h"

class SemanticAnalysis {
    std::vector<std::unique_ptr<FunctionDecl>> &TopLevel;
    std::vector<std::vector<Decl *>> scopes;
    std::vector<std::string> diagnostics;
    
    void error(SourceLocation location, std::string_view message) {
        const auto& [file, line, col] = location;
        std::ostringstream oss;
        oss << file << ':' << line << ':' << col << ": error: " << message;
        diagnostics.push_back(oss.str());
        std::cerr << oss.str() << "\n";
    }

    Decl *lookupDecl(const std::string &id) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            for (auto &&decl : *it) {
                if (decl->identifier == id) {
                    return decl;
                }
            }
        }
        return nullptr;
    }
    
    std::optional<Type> resolveType(const std::string &typeSpecifier) {
        if (typeSpecifier == "void") return Type::VOID;
        if (typeSpecifier == "string") return Type::STRING;
        if (typeSpecifier == "number") return Type::NUMBER;
        return std::nullopt;
    }

    bool resolveDeclRefExpr(DeclRefExpr &DRE) {
        Decl *decl = lookupDecl(DRE.identifier);
        if (!decl) {
            error(DRE.location, "symbol '" + DRE.identifier + "' not found");
            return false;
        }
        DRE.resolvedDecl = decl;
        DRE.resolvedType = decl->resolvedType;
        return true;
    }

    bool resolvePrintExpr(PrintExpr &printexpr) {
        for (auto &&arg : printexpr.args) {
            if (!resolveExpr(*arg)) {
                return false;
            }
        }
        
        if (printexpr.args.empty()) {
            error(printexpr.location, "print requires at least one argument");
            return false;
        }
        
        printexpr.resolvedType = Type::VOID;
        return true;
    }

    bool resolveCallExpr(CallExpr &cexpr) {
 
        if (!resolveDeclRefExpr(*cexpr.identifier)) {
            return false;
        }
        
        auto functionDecl = dynamic_cast<FunctionDecl *>(cexpr.identifier->resolvedDecl);
        if (!functionDecl) {
            error(cexpr.location, "calling non-function element");
            return false;
        }
        
        cexpr.resolvedCallee = functionDecl;
        if (cexpr.arguments.size() != functionDecl->params.size()) {
            error(cexpr.location, "wrong number of arguments in function call");
            return false;
        }
        
        for (size_t idx = 0; idx < cexpr.arguments.size(); ++idx) {
            if (!resolveExpr(*cexpr.arguments[idx])) {
                return false;
            }
            
            if (cexpr.arguments[idx]->resolvedType != functionDecl->params[idx]->resolvedType) {
                error(cexpr.arguments[idx]->location, 
                      "unexpected type of argument in " + functionDecl->identifier + " function call");
                return false;
            }
        }
        
        cexpr.resolvedType = functionDecl->resolvedType;
        return true;
    }

    bool resolveExpr(Expr &expr) {
        if (auto stringLiteral = dynamic_cast<StringLiteral *>(&expr)) {
            return true;  
        }
        if (auto numberLiteral = dynamic_cast<NumberLiteral *>(&expr)) {
            return true;
        }
        if (auto dre = dynamic_cast<DeclRefExpr *>(&expr)) {
            return resolveDeclRefExpr(*dre);
        }
        if (auto printexpr = dynamic_cast<PrintExpr *>(&expr)) {
            return resolvePrintExpr(*printexpr);
        }
        if (auto cexpr = dynamic_cast<CallExpr *>(&expr)) {
            return resolveCallExpr(*cexpr);
        }
        
        error(expr.location, "unknown expression type");
        return false;
    }

    bool resolveStmt(Stmt &stmt) {
        if (auto expr = dynamic_cast<Expr *>(&stmt)) {
            return resolveExpr(*expr);
        }
        error(stmt.location, "unknown statement type");
        return false;
    }

    bool resolveBody(Block &body) {
        for (auto &&stmt : body.statements) {
            if (!resolveStmt(*stmt)) {
                return false;
            }
        }
        return true;
    }

    bool resolveParamDecl(ParamDecl &param) {
        std::optional<Type> param_type = resolveType(param.type);
        if (!param_type) {
            error(param.location, std::string{"parameter '"} +
                  param.identifier + "' has invalid '" +
                  param.type + "' type");
            return false;
        }
        
        if (lookupDecl(param.identifier)) {
            error(param.location, std::string{"parameter '"} +
                  param.identifier + "' redeclared");
            return false;
        }
        
        param.resolvedType = *param_type;
        return true;
    }

    bool resolveFunctionSignature(FunctionDecl &function) {
        std::optional<Type> type = resolveType(function.funtype);
        if (!type) {
            error(function.location, "invalid return type '" + function.funtype + "'");
            return false;
        }
        function.resolvedType = *type;
        for (auto &&param : function.params) {
            if (!resolveParamDecl(*param)) {
                return false;
            }
        }
        
        return true;
    }

public:
    SemanticAnalysis(std::vector<std::unique_ptr<FunctionDecl>> &TopLevel)
        : TopLevel(TopLevel) {}
    
    bool resolve() {
        scopes.emplace_back();
        
        // First pass: resolve function signatures and add to scope
        for (auto &&function : TopLevel) {
            if (!resolveFunctionSignature(*function)) {
                return false;
            }
            scopes.back().emplace_back(function.get());
        }
        
        // Second pass: resolve function bodies
        for (auto &&function : TopLevel) {
            scopes.emplace_back();
            
            // Add parameters to scope
            for (auto &&param : function->params) {
                scopes.back().emplace_back(param.get());
            }
            
            // Resolve body
            if (!resolveBody(*function->body)) {
                return false;
            }
            
            scopes.pop_back();
        }
        
        return true;
    }
};

#endif