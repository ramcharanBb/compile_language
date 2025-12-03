#ifndef PARSER_H
#define PARSER_H
 
#include<vector>
#include<memory>

#include "lexer.h"
#include "utils.h"
#include "ast.h"

class Parser{
    TheLexer *lexer;
    Token nextToken;
    std::vector<std::string> diagnostics;
    void skipToken(){ nextToken=lexer->getNextToken();}
    bool skipUntil(std::initializer_list<TokenKind> kinds);
    void error(SourceLocation location,std::string_view message);
    

    std::unique_ptr<Expr> parseParenExpr();
    std::unique_ptr<Expr> parseNumberExpr() ;
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseStringExpr();
    std::unique_ptr<Expr> parsePrintExpr();
    std::unique_ptr<Expr> parseIdentifierExpr();
    std::unique_ptr<Stmt> parseReturnStmt();
    
    int getTokPrecedence();
    std::unique_ptr<Expr> parsePrimaryExpr();
    std::unique_ptr<Expr> parseBinaryExpr(int exprPrec, std::unique_ptr<Expr> lhs);
    
    std::unique_ptr<ParamDecl> parseParams(); 
    std::unique_ptr<Block> parseBlock();
    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<VariableDecl> parseVariableDecl();
    std::unique_ptr<FunctionDecl> parseFunction();
    std::unique_ptr<Stmt> parseIfStmt();
    std::unique_ptr<Stmt> parseWhileStmt();
    std::unique_ptr<Expr> parseBooleanExpr();
    
    
   public:
    explicit Parser(TheLexer &lexer): lexer(&lexer), nextToken(lexer.getNextToken()){}
    std::vector<std::unique_ptr<FunctionDecl>> parseProgram();
};


#endif