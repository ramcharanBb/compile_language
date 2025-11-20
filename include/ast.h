#ifndef AST_H
#define AST_H

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <utility>

#include "utils.h"      
/*====================================================================*/
/*  Visitor interfaces                                                  */
/*====================================================================*/
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

class ResolvedVisitor {
public:
    virtual ~ResolvedVisitor() = default;

    virtual void visit(class ResolvedStmt        &node) = 0;
    virtual void visit(class ResolvedBlock       &node) = 0;
    virtual void visit(class ResolvedPrintExpr   &node) = 0;

    virtual void visit(class ResolvedDecl        &node) = 0;
    virtual void visit(class ResolvedParamDecl   &node) = 0;
    virtual void visit(class ResolvedFunctionDecl&node) = 0;

    virtual void visit(class ResolvedExpr        &node) = 0;
    virtual void visit(class ResolvedNumberLiteral &node) = 0;
    virtual void visit(class ResolvedStringLiteral &node) = 0;
    virtual void visit(class ResolvedDeclRefExpr    &node) = 0;
    virtual void visit(class ResolvedCallExpr       &node) = 0;

    void setLevel(size_t l) { currentLevel = l; }
    size_t getLevel() const { return currentLevel; }

protected:
    size_t currentLevel = 0;
};
/*====================================================================*/
/*  Base node classes                                                   */
/*====================================================================*/
class ASTNode : public Dumpable {
public:
    SourceLocation location;
    explicit ASTNode(SourceLocation loc) : location(std::move(loc)) {}
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
    void dump(size_t level = 0) override;
};
// Forward declare DumpVisitor
class DumpVisitor;

class ResolvedNode : public Dumpable {
public:
    SourceLocation location;
    explicit ResolvedNode(SourceLocation loc) : location(std::move(loc)) {}
    virtual ~ResolvedNode() = default;
    virtual void accept(ResolvedVisitor &visitor) = 0;
    void dump(size_t level = 0) override;
};
// Forward declare ResolvedDumpVisitor
class ResolvedDumpVisitor;


/*====================================================================*/
/*  Un‑resolved AST (front‑end)                                         */
/*====================================================================*/

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
    std::string funtype;
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
        : Expr(std::move(loc)), value(std::move(v)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class StringLiteral : public Expr {
public:
    std::string value;
    StringLiteral(SourceLocation loc, std::string v)
        : Expr(std::move(loc)), value(std::move(v)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class DeclRefExpr : public Expr {
public:
    std::string identifier;
    DeclRefExpr(SourceLocation loc, std::string id)
        : Expr(std::move(loc)), identifier(std::move(id)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<DeclRefExpr> identifier;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(SourceLocation loc,
             std::unique_ptr<DeclRefExpr> id,
             std::vector<std::unique_ptr<Expr>> args)
        : Expr(std::move(loc)),
          identifier(std::move(id)),
          arguments(std::move(args)) {}
    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }
};

/*====================================================================*/
/*  Resolved AST (after type checking)                                  */
/*====================================================================*/

enum class Type { NUMBER, STRING, VOID };

/*-------------------- Base resolved nodes --------------------*/

class ResolvedStmt : public ResolvedNode {
public:
    using ResolvedNode::ResolvedNode;
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedDecl : public ResolvedNode {
public:
    std::string identifier;
    Type        type;
    ResolvedDecl(SourceLocation loc,
                 std::string id,
                 Type t)
        : ResolvedNode(std::move(loc)),
          identifier(std::move(id)),
          type(t) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

/*-------------------- Statements --------------------*/

class ResolvedBlock : public ResolvedNode {
public:
    std::vector<std::unique_ptr<ResolvedStmt>> statements;
    ResolvedBlock(SourceLocation loc,
                  std::vector<std::unique_ptr<ResolvedStmt>> stmts)
        : ResolvedNode(std::move(loc)), statements(std::move(stmts)) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};



/*-------------------- Declarations --------------------*/

class ResolvedParamDecl : public ResolvedDecl {
public:
    ResolvedParamDecl(SourceLocation loc,
                      std::string id,
                      Type t)
        : ResolvedDecl(std::move(loc), std::move(id), t) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedFunctionDecl : public ResolvedDecl {
public:
    std::vector<std::unique_ptr<ResolvedParamDecl>> params;
    std::unique_ptr<ResolvedBlock> body;
    ResolvedFunctionDecl(SourceLocation loc,
                         std::string id,
                         Type t,
                         std::vector<std::unique_ptr<ResolvedParamDecl>> ps,
                         std::unique_ptr<ResolvedBlock> b)
        : ResolvedDecl(std::move(loc), std::move(id), t),
          params(std::move(ps)),
          body(std::move(b)) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

/*-------------------- Expressions --------------------*/

class ResolvedExpr : public ResolvedStmt {
public:
    Type type;
    ResolvedExpr(SourceLocation loc, Type t = Type::VOID)
        : ResolvedStmt(std::move(loc)), type(t) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedPrintExpr : public ResolvedExpr {
public:
    std::vector<std::unique_ptr<ResolvedExpr>> args;
    ResolvedPrintExpr(SourceLocation loc,
                      std::vector<std::unique_ptr<ResolvedExpr>> a)
        : ResolvedExpr(std::move(loc)), args(std::move(a)) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedNumberLiteral : public ResolvedExpr {
public:
    std::string value;
    ResolvedNumberLiteral(SourceLocation loc, std::string v)
        : ResolvedExpr(std::move(loc), Type::NUMBER), value(std::move(v)) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedStringLiteral : public ResolvedExpr {
public:
    std::string value;
    ResolvedStringLiteral(SourceLocation loc, std::string v)
        : ResolvedExpr(std::move(loc), Type::STRING), value(std::move(v)) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedDeclRefExpr : public ResolvedExpr {
public:
    ResolvedDecl *decl;                // non‑owning
    ResolvedDeclRefExpr(SourceLocation loc, ResolvedDecl *d)
        : ResolvedExpr(std::move(loc), d->type), decl(d) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

class ResolvedCallExpr : public ResolvedExpr {
public:
    ResolvedFunctionDecl *callee;                     // non‑owning
    std::vector<std::unique_ptr<ResolvedExpr>> arguments;
    ResolvedCallExpr(SourceLocation loc,
                     ResolvedFunctionDecl *c,
                     std::vector<std::unique_ptr<ResolvedExpr>> args)
        : ResolvedExpr(std::move(loc), c->type),
          callee(c),
          arguments(std::move(args)) {}
    void accept(ResolvedVisitor &visitor) override { visitor.visit(*this); }
};

/*====================================================================*/
/*  Concrete visitors – the old `dump()` functionality                */
/*====================================================================*/

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
    void visit(Stmt &node) override            { dumpHeader("Stmt"); }
    void visit(Block &node) override {
        dumpHeader("Block:");
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &s : node.statements) {
             // We need to manually pass the visitor or call accept.
             // But accept() takes a visitor.
             // If we call s->accept(*this), the visitor's state (currentLevel) is used.
             s->accept(*this);
        }
        currentLevel = oldLevel;
    }
    void visit(PrintExpr &node) override {
        dumpHeader("PrintExpr:");
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &a : node.args) a->accept(*this);
        currentLevel = oldLevel;
    }

    /* Declarations */
    void visit(Decl &node) override {
        dumpHeader("Decl: " + node.identifier);
    }
    void visit(ParamDecl &node) override {
        dumpHeader("Parameter Declaration : " + node.identifier +
                   " : " + node.type);
    }
    void visit(FunctionDecl &node) override {
        dumpHeader("FunctionDecl: " + node.identifier + " : " + node.funtype);
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &p : node.params) p->accept(*this);
        if (node.body) node.body->accept(*this);
        currentLevel = oldLevel;
    }

    /* Expressions */
    void visit(Expr &node) override            { dumpHeader("Expr"); }
    void visit(NumberLiteral &node) override   { dumpHeader("NumberLiteral " + node.value); }
    void visit(StringLiteral &node) override   { dumpHeader("StringLiteral : " + node.value); }
    void visit(DeclRefExpr &node) override     { dumpHeader("DeclRefExpr: " + node.identifier); }
    void visit(CallExpr &node) override {
        dumpHeader("CallExpr:");
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


class ResolvedDumpVisitor : public ResolvedVisitor {
public:
    void dumpHeader(const std::string &msg) const {
        indent(currentLevel);
        std::cerr << msg << "\n";
    }
    void indent(size_t level) const {
        for (size_t i = 0; i < level; ++i) std::cerr << "  ";
    }

    /* Statements */
    void visit(ResolvedStmt &node) override           { dumpHeader("Stmt"); }
    void visit(ResolvedBlock &node) override {
        dumpHeader("Block");
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &s : node.statements) s->accept(*this);
        currentLevel = oldLevel;
    }
    void visit(ResolvedPrintExpr &node) override {
        dumpHeader("PrintExprs:");
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &a : node.args) a->accept(*this);
        currentLevel = oldLevel;
    }

    /* Declarations */
    void visit(ResolvedDecl &node) override {
        dumpHeader("Decl: " + node.identifier);
    }
    void visit(ResolvedParamDecl &node) override {
        dumpHeader("ParamDecl: " + node.identifier);
    }
    void visit(ResolvedFunctionDecl &node) override {
        dumpHeader("FunctionDecl: " + node.identifier);
        size_t oldLevel = currentLevel;
        currentLevel++;
        for (auto &p : node.params) p->accept(*this);
        if (node.body) node.body->accept(*this);
        currentLevel = oldLevel;
    }

    /* Expressions */
    void visit(ResolvedExpr &node) override          { dumpHeader("Expr"); }
    void visit(ResolvedNumberLiteral &node) override { dumpHeader("NumberLiteral: " + node.value); }
    void visit(ResolvedStringLiteral &node) override { dumpHeader("StringLiteral: " + node.value); }
    void visit(ResolvedDeclRefExpr &node) override   { dumpHeader("DeclRefExpr: " + node.decl->identifier); }
    void visit(ResolvedCallExpr &node) override {
        dumpHeader("CallExpr");
        size_t oldLevel = currentLevel;
        currentLevel++;
        node.callee->accept(*this);
        for (auto &a : node.arguments) a->accept(*this);
        currentLevel = oldLevel;
    }
};

// Implement ResolvedNode::dump
inline void ResolvedNode::dump(size_t level) {
    ResolvedDumpVisitor visitor;
    visitor.setLevel(level);
    this->accept(visitor);
}


#endif // AST_H