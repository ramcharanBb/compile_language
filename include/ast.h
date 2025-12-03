#ifndef AST_H
#define AST_H

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <utility>
#include <optional>

#include "utils.h"
#include "token.h"

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
    virtual void visit(class ReturnStmt    &node) =0;
    virtual void visit(class IfStmt        &node) = 0;
    virtual void visit(class WhileStmt     &node) = 0;

    virtual void visit(class Decl          &node) = 0;
    virtual void visit(class ParamDecl     &node) = 0;
    virtual void visit(class VariableDecl  &node) = 0;
    virtual void visit(class FunctionDecl  &node) = 0;

    virtual void visit(class Expr          &node) = 0;
    virtual void visit(class NumberLiteral &node) = 0;
    virtual void visit(class StringLiteral &node) = 0;
    virtual void visit(class BooleanLiteral &node) = 0;
    virtual void visit(class DeclRefExpr   &node) = 0;
    virtual void visit(class CallExpr      &node) = 0;
    virtual void visit(class BinaryExpr    &node) = 0;
    virtual void visit(class AssignmentExpr &node) = 0;

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


class Stmt : public virtual ASTNode {
public:
    using ASTNode::ASTNode;
    void accept(ASTVisitor &visitor) override =0;
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

class Decl : public virtual ASTNode {
public:
    std::string identifier;
    std::optional<Type> resolvedType;  // Populated during semantic analysis
    
    Decl(SourceLocation loc, std::string id)
        : ASTNode(std::move(loc)), identifier(std::move(id)) {}
    void accept(ASTVisitor &visitor) override  =0;
};

class ParamDecl : public Decl {
public:
    std::string type;  
    
    ParamDecl(SourceLocation loc, std::string id, std::string tp)
        : ASTNode(loc), Decl(loc, id), type(std::move(tp)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); };
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
        : ASTNode(loc), Decl(loc, name),
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
        : ASTNode(loc), Expr(loc), args(std::move(a)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class NumberLiteral : public Expr {
public:
    std::string value;
    
    NumberLiteral(SourceLocation loc, std::string v)
        : ASTNode(loc), Expr(loc), value(std::move(v)) {
        resolvedType = Type::NUMBER; 
    }
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class StringLiteral : public Expr {
public:
    std::string value;
    
    StringLiteral(SourceLocation loc, std::string v)
        : ASTNode(loc), Expr(loc), value(std::move(v)) {
        resolvedType = Type::STRING;  
    }
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BooleanLiteral : public Expr {
public:
    bool value;
    
    BooleanLiteral(SourceLocation loc, bool v)
        : ASTNode(loc), Expr(loc), value(v) {
        resolvedType = Type::NUMBER;  // Treat booleans as numbers (0/1)
    }
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DeclRefExpr : public Expr {
public:
    std::string identifier;
    Decl *resolvedDecl = nullptr;  // Set during semantic analysis
    
    DeclRefExpr(SourceLocation loc, std::string id)
        : ASTNode(loc), Expr(loc), identifier(std::move(id)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class VariableDecl : public Stmt, public Decl {
public:
    std::string type;
    std::unique_ptr<Expr> initializer; 

    VariableDecl(SourceLocation loc,std::string id, std::string tp, std::unique_ptr<Expr> init = nullptr)
        : ASTNode(loc), Stmt(loc), Decl(loc, id), type(std::move(tp)), initializer(std::move(init)) {}

    void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};
class ReturnStmt : public Stmt{
    public:
      std::unique_ptr<Expr> expr=nullptr;
      ReturnStmt(SourceLocation location, std::unique_ptr<Expr> expr)
      : ASTNode(location), Stmt(location),
        expr(std::move(expr)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock;  // Optional

    IfStmt(SourceLocation loc,
           std::unique_ptr<Expr> cond,
           std::unique_ptr<Block> thenB,
           std::unique_ptr<Block> elseB = nullptr)
        : ASTNode(loc), Stmt(loc),
          condition(std::move(cond)),
          thenBlock(std::move(thenB)),
          elseBlock(std::move(elseB)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Block> body;

    WhileStmt(SourceLocation loc,
              std::unique_ptr<Expr> cond,
              std::unique_ptr<Block> b)
        : ASTNode(loc), Stmt(loc),
          condition(std::move(cond)),
          body(std::move(b)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class CallExpr : public Expr {
public:
    std::string identifier;
    std::vector<std::unique_ptr<Expr>> arguments;
    FunctionDecl *resolvedCallee = nullptr;  // Set during semantic analysis
    
    CallExpr(SourceLocation loc,
             std::string id,
             std::vector<std::unique_ptr<Expr>> args)
        : ASTNode(loc), Expr(loc),
          identifier(std::move(id)),
          arguments(std::move(args)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    TokenKind op;
    std::unique_ptr<Expr> right;

    BinaryExpr(SourceLocation loc,
               std::unique_ptr<Expr> lhs,
               TokenKind operation,
               std::unique_ptr<Expr> rhs)
        : ASTNode(loc), Expr(loc),
          left(std::move(lhs)),
          op(operation),
          right(std::move(rhs)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class AssignmentExpr : public Expr {
public:
    std::string target;  // Variable name being assigned to
    std::unique_ptr<Expr> value;  // RHS expression
    Decl *resolvedTarget = nullptr;  // Set during semantic analysis

    AssignmentExpr(SourceLocation loc,
                   std::string tgt,
                   std::unique_ptr<Expr> val)
        : ASTNode(loc), Expr(loc),
          target(std::move(tgt)),
          value(std::move(val)) {}

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

    void visit(ReturnStmt & node) override{
    dumpHeader("ReturnStmt: "); 
    size_t oldLevel = currentLevel;
        currentLevel++;
    if (node.expr) {
        node.expr->accept(*this);
    }
    currentLevel = oldLevel;
    }

    void visit(IfStmt &node) override {
        dumpHeader("IfStmt:");
        size_t oldLevel = currentLevel;
        currentLevel++;
        dumpHeader("Condition:");
        currentLevel++;
        node.condition->accept(*this);
        currentLevel--;
        dumpHeader("Then:");
        node.thenBlock->accept(*this);
        if (node.elseBlock) {
            dumpHeader("Else:");
            node.elseBlock->accept(*this);
        }
        currentLevel = oldLevel;
    }

    void visit(WhileStmt &node) override {
        dumpHeader("WhileStmt:");
        size_t oldLevel = currentLevel;
        currentLevel++;
        dumpHeader("Condition:");
        currentLevel++;
        node.condition->accept(*this);
        currentLevel--;
        dumpHeader("Body:");
        node.body->accept(*this);
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
    void visit(BooleanLiteral &node) override { 
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("BooleanLiteral: " + std::string(node.value ? "true" : "false") + typeInfo); 
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
        dumpHeader("Identifier: " + node.identifier);
        for (auto &a : node.arguments) a->accept(*this);
        currentLevel = oldLevel;
    }
    void visit(BinaryExpr &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("BinaryExpr" + typeInfo + ": " + Token::kindToString(node.op));
        size_t oldLevel = currentLevel;
        currentLevel++;
        node.left->accept(*this);
        node.right->accept(*this);
        currentLevel = oldLevel;
    }
    void visit(VariableDecl &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : " : " + node.type;
        std::string initInfo = node.initializer ? " (with initializer)" : "";
        dumpHeader("VariableDecl: " + node.identifier + typeInfo + initInfo);
        if (node.initializer) {
            size_t oldLevel = currentLevel;
            currentLevel++;
            node.initializer->accept(*this);
            currentLevel = oldLevel;
        }
    }
    void visit(AssignmentExpr &node) override {
        std::string typeInfo = node.resolvedType ? 
            " : " + typeToString(*node.resolvedType) : "";
        dumpHeader("AssignmentExpr" + typeInfo + ": " + node.target);
        size_t oldLevel = currentLevel;
        currentLevel++;
        node.value->accept(*this);
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