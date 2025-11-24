#ifndef AST_H
#define AST_H

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <utility>
#include <optional>

#include "utils.h"

enum class Type { NUMBER, STRING, VOID };

inline std::string typeToString(Type t) {
    switch (t) {
        case Type::NUMBER: return "number";
        case Type::STRING: return "string";
        case Type::VOID: return "void";
    }
    return "unknown";
}

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(class Stmt          &node) = 0;
    virtual void visit(class Block         &node) = 0;
    virtual void visit(class PrintExpr     &node) = 0;

    virtual void visit(class Decl          &node) = 0;
    virtual void visit(class ParamDecl     &node) = 0;
    virtual void visit(class FunctionDecl  &node) = 0;

    virtual void visit(class Expr          &node) = 0;
    virtual void visit(class NumberLiteral &node) = 0;
    virtual void visit(class StringLiteral &node) = 0;
    virtual void visit(class DeclRefExpr   &node) = 0;
    virtual void visit(class CallExpr      &node) = 0;

    void setLevel(size_t l) { currentLevel = l; }
    size_t getLevel() const { return currentLevel; }

protected:
    size_t currentLevel = 0;
};

class ASTNode : public Dumpable {
public:
    SourceLocation location;
    explicit ASTNode(SourceLocation loc) : location(std::move(loc)) {}
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
    void dump(size_t level = 0) override;
};

class DumpVisitor;


class Stmt : public ASTNode {
public:
    using ASTNode::ASTNode;
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class Block : public ASTNode {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    Block(SourceLocation loc,
          std::vector<std::unique_ptr<Stmt>> stmts)
        : ASTNode(std::move(loc)), statements(std::move(stmts)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

/*-------------------- Declarations --------------------*/

class Decl : public ASTNode {
public:
    std::string identifier;
    std::optional<Type> resolvedType;  // Populated during semantic analysis
    
    Decl(SourceLocation loc, std::string id)
        : ASTNode(std::move(loc)), identifier(std::move(id)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class ParamDecl : public Decl {
public:
    std::string type;  
    
    ParamDecl(SourceLocation loc, std::string id, std::string tp)
        : Decl(std::move(loc), std::move(id)), type(std::move(tp)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class FunctionDecl : public Decl {
public:
    std::string funtype;  // String return type from parser
    std::vector<std::unique_ptr<ParamDecl>> params;
    std::unique_ptr<Block> body;
    
    FunctionDecl(SourceLocation loc,
                 std::string name,
                 std::string ft,
                 std::vector<std::unique_ptr<ParamDecl>> ps,
                 std::unique_ptr<Block> b)
        : Decl(std::move(loc), std::move(name)),
          funtype(std::move(ft)),
          params(std::move(ps)),
          body(std::move(b)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

/*-------------------- Expressions --------------------*/

class Expr : public Stmt {
public:
    std::optional<Type> resolvedType;  // Populated during semantic analysis
    using Stmt::Stmt;
};

class PrintExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> args;
    
    PrintExpr(SourceLocation loc,
              std::vector<std::unique_ptr<Expr>> a)
        : Expr(std::move(loc)), args(std::move(a)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class NumberLiteral : public Expr {
public:
    std::string value;
    
    NumberLiteral(SourceLocation loc, std::string v)
        : Expr(std::move(loc)), value(std::move(v)) {
        resolvedType = Type::NUMBER; 
    }
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class StringLiteral : public Expr {
public:
    std::string value;
    
    StringLiteral(SourceLocation loc, std::string v)
        : Expr(std::move(loc)), value(std::move(v)) {
        resolvedType = Type::STRING;  
    }
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DeclRefExpr : public Expr {
public:
    std::string identifier;
    Decl *resolvedDecl = nullptr;  // Set during semantic analysis
    
    DeclRefExpr(SourceLocation loc, std::string id)
        : Expr(std::move(loc)), identifier(std::move(id)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<DeclRefExpr> identifier;
    std::vector<std::unique_ptr<Expr>> arguments;
    FunctionDecl *resolvedCallee = nullptr;  // Set during semantic analysis
    
    CallExpr(SourceLocation loc,
             std::unique_ptr<DeclRefExpr> id,
             std::vector<std::unique_ptr<Expr>> args)
        : Expr(std::move(loc)),
          identifier(std::move(id)),
          arguments(std::move(args)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};



class DumpVisitor : public ASTVisitor {
public:
    void dumpHeader(const std::string &msg) const {
        indent(currentLevel);
        std::cerr << msg << "\n";
    }
    void indent(size_t level) const {
        for (size_t i = 0; i < level; ++i) std::cerr << "  ";
    }

    /* Statements */
    void visit(Stmt &node) override { dumpHeader("Stmt"); }
    void visit(Block &node) override {
        dumpHeader("Block:");
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &s : node.statements) {
            s->accept(*this);
        }
        currentLevel = oldLevel;
    }
    void visit(PrintExpr &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("PrintExpr" + typeInfo + ":");
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &a : node.args) a->accept(*this);
        currentLevel = oldLevel;
    }

    void visit(Decl &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("Decl: " + node.identifier + typeInfo);
    }
    void visit(ParamDecl &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : " : " + node.type;
        dumpHeader("Parameter Declaration: " + node.identifier + typeInfo);
    }
    void visit(FunctionDecl &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : " : " + node.funtype;
        dumpHeader("FunctionDecl: " + node.identifier + typeInfo);
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &p : node.params) p->accept(*this);
        if (node.body) node.body->accept(*this);
        currentLevel = oldLevel;
    }

    /* Expressions */
    void visit(Expr &node) override { 
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("Expr" + typeInfo); 
    }
    void visit(NumberLiteral &node) override { 
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("NumberLiteral " + node.value + typeInfo); 
    }
    void visit(StringLiteral &node) override { 
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("StringLiteral: " + node.value + typeInfo); 
    }
    void visit(DeclRefExpr &node) override { 
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("DeclRefExpr: " + node.identifier + typeInfo); 
    }
    void visit(CallExpr &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("CallExpr" + typeInfo + ":");
        size_t oldLevel = currentLevel;
        currentLevel++;
        node.identifier->accept(*this);
        for (auto &a : node.arguments) a->accept(*this);
        currentLevel = oldLevel;
    }
};

// Implement ASTNode::dump
inline void ASTNode::dump(size_t level) {
    DumpVisitor visitor;
    visitor.setLevel(level);
    this->accept(visitor);
}

#endif // AST_H