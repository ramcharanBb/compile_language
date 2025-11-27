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
    FunctionDecl* currentFunction = nullptr;
    
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
 
        Decl *decl = lookupDecl(cexpr.identifier);
        if (!decl) {
            error(cexpr.location, "function '" + cexpr.identifier + "' not found");
            return false;
        }
        
        auto functionDecl = dynamic_cast<FunctionDecl *>(decl);
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

    bool resolveBinaryExpr(BinaryExpr &bexpr) {
        // Resolve left and right operands
        if (!resolveExpr(*bexpr.left)) {
            return false;
        }
        if (!resolveExpr(*bexpr.right)) {
            return false;
        }
        
        // For arithmetic operations, both operands must be numbers
        if (bexpr.op == TokenKind::plus || bexpr.op == TokenKind::minus ||
            bexpr.op == TokenKind::mul || bexpr.op == TokenKind::slash ||
            bexpr.op == TokenKind::percent) {
            
            if (bexpr.left->resolvedType != Type::NUMBER) {
                error(bexpr.left->location, "left operand of arithmetic operator must be a number");
                return false;
            }
            if (bexpr.right->resolvedType != Type::NUMBER) {
                error(bexpr.right->location, "right operand of arithmetic operator must be a number");
                return false;
            }
            bexpr.resolvedType = Type::NUMBER;
            return true;
        }
        
        // For comparison operations, both operands must be numbers, result is number (0 or 1)
        if (bexpr.op == TokenKind::lessthan || bexpr.op == TokenKind::greaterthan ||
            bexpr.op == TokenKind::less_equal || bexpr.op == TokenKind::great_equal ||
            bexpr.op == TokenKind::doublequal || bexpr.op == TokenKind::not_equal) {
            
            if (bexpr.left->resolvedType != Type::NUMBER) {
                error(bexpr.left->location, "left operand of comparison operator must be a number");
                return false;
            }
            if (bexpr.right->resolvedType != Type::NUMBER) {
                error(bexpr.right->location, "right operand of comparison operator must be a number");
                return false;
            }
            // Comparisons return a boolean, but we represent as NUMBER (0 or 1)
            bexpr.resolvedType = Type::NUMBER;
            return true;
        }
        
        // For logical operations
        if (bexpr.op == TokenKind::amp_amp || bexpr.op == TokenKind::pipe_pipe) {
            // Logical operations also work on numbers (treated as booleans)
            if (bexpr.left->resolvedType != Type::NUMBER) {
                error(bexpr.left->location, "left operand of logical operator must be a number");
                return false;
            }
            if (bexpr.right->resolvedType != Type::NUMBER) {
                error(bexpr.right->location, "right operand of logical operator must be a number");
                return false;
            }
            bexpr.resolvedType = Type::NUMBER;
            return true;
        }
        
        error(bexpr.location, "unknown binary operator");
        return false;
    }

    bool resolveVariableDecl(VariableDecl &varDecl) {
        std::optional<Type> varType = resolveType(varDecl.type);
        if (!varType) {
            error(varDecl.location, "variable '" + varDecl.identifier + "' has invalid type '" + varDecl.type + "'");
            return false;
        }
        
        if (lookupDecl(varDecl.identifier)) {
            error(varDecl.location, "variable '" + varDecl.identifier + "' redeclared");
            return false;
        }
        
        varDecl.resolvedType = *varType;
        
        // If there's an initializer, resolve it and check type compatibility
        if (varDecl.initializer) {
            if (!resolveExpr(*varDecl.initializer)) {
                return false;
            }
            if (varDecl.initializer->resolvedType != *varType) {
                error(varDecl.initializer->location, "initializer type does not match variable type");
                return false;
            }
        }
        
        // Add to current scope
        scopes.back().emplace_back(&varDecl);
        return true;
    }

    bool resolveAssignmentExpr(AssignmentExpr &assignExpr) {
        // Look up the target
        Decl *decl = lookupDecl(assignExpr.target);
        if (!decl) {
            error(assignExpr.location, "assignment to undefined variable '" + assignExpr.target + "'");
            return false;
        }
        
        assignExpr.resolvedTarget = decl;
        
        // Resolve the RHS
        if (!resolveExpr(*assignExpr.value)) {
            return false;
        }
        
        // Check type compatibility
        if (assignExpr.value->resolvedType != decl->resolvedType) {
            error(assignExpr.value->location, "assignment type mismatch");
            return false;
        }
        
        assignExpr.resolvedType = decl->resolvedType;
        return true;
    }

    bool resolveExpr(Expr &expr) {
        if (dynamic_cast<StringLiteral *>(&expr)) {
            return true;  
        }
        if (dynamic_cast<NumberLiteral *>(&expr)) {
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
        if (auto bexpr = dynamic_cast<BinaryExpr *>(&expr)) {
            return resolveBinaryExpr(*bexpr);
        }
        if (auto assignExpr = dynamic_cast<AssignmentExpr *>(&expr)) {
            return resolveAssignmentExpr(*assignExpr);
        }
        
        error(expr.location, "unknown expression type");
        return false;
    }
  bool resolveReturnStmt(ReturnStmt *stmt){
       if (currentFunction && (currentFunction->resolvedType == Type::VOID)){
           if(stmt->expr!= nullptr){
            error(stmt->location, "Cannot return a value from a function with 'void' return type.");
            return false; 
           }
       }
       if (auto expr = dynamic_cast<Expr *>((stmt->expr).get())) {
            return resolveExpr(*expr);
        }
        return true;
  }
    bool resolveStmt(Stmt &stmt) {
        if (auto rstmt  = dynamic_cast<ReturnStmt *>(&stmt)) {
            return resolveReturnStmt(rstmt);
        }
        if (auto varDecl = dynamic_cast<VariableDecl *>(&stmt)) {
            return resolveVariableDecl(*varDecl);
        }
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
            currentFunction = function.get();
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