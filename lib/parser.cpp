#include<iostream>
#include <sstream>
#include <initializer_list> 
#include "utils.h"
#include "parser.h"

void Parser::error(SourceLocation location, std::string_view message) {
  const auto& [file, line, col] = location;
  std::ostringstream oss;
  oss << file << ':' << line << ':' << col << ": error: " << message;
  diagnostics.push_back(oss.str());
  std::cerr << oss.str() << "\n";
}

bool Parser::skipUntil(std::initializer_list<TokenKind> kinds){
    while (true) {
        for (auto k : kinds) {
            if (nextToken.kind == k) return true;
        }

        if (nextToken.kind == TokenKind::eof)
            return false;
        skipToken();
    }
}

std::unique_ptr<Expr> Parser::parseIdentifierExpr(){
    SourceLocation location = nextToken.location;
    std::string identifier = nextToken.value.value();

    skipToken();

    if (nextToken.kind != TokenKind::lpar)
        return std::make_unique<DeclRefExpr>(location, identifier);
    
    skipToken(); // skip '('
    
    std::vector<std::unique_ptr<Expr>> arguments;
    while(nextToken.kind != TokenKind::rpar && nextToken.kind != TokenKind::eof){
        
        if(std::unique_ptr<Expr> expr = parseExpr()){
            arguments.push_back(std::move(expr));
        } else {
            if (!skipUntil({TokenKind::comma, TokenKind::rpar}))
                break;
            if (nextToken.kind == TokenKind::comma)
                skipToken();
            continue;
        }
        
        if (nextToken.kind == TokenKind::comma) {
            skipToken();
        } else if (nextToken.kind != TokenKind::rpar) {
            error(nextToken.location, "expected ',' or ')' in argument list");
            skipToken();
        }
    }
    
    if (nextToken.kind == TokenKind::rpar)
        skipToken();
    else {
        error(location, "expected ')' to close argument list");
    }
    return std::make_unique<CallExpr>(location,std::move(identifier), std::move(arguments));
}

std::unique_ptr<Expr> Parser::parsePrintExpr(){
    SourceLocation location = nextToken.location;
    skipToken(); // skip 'print'
    
    if (nextToken.kind != TokenKind::lpar){
        error(nextToken.location, "expected '(' after 'print'");
        return nullptr;
    }
    skipToken();
    
    std::vector<std::unique_ptr<Expr>> arguments;
    while(nextToken.kind != TokenKind::rpar && nextToken.kind != TokenKind::eof){
        
        if(std::unique_ptr<Expr> expr = parseExpr()){
            arguments.push_back(std::move(expr));
        } else {
            if (!skipUntil({TokenKind::comma, TokenKind::rpar}))
                break;
            if (nextToken.kind == TokenKind::comma)
                skipToken();
            continue;
        }
        
        if (nextToken.kind == TokenKind::comma) {
            skipToken();
        } else if (nextToken.kind != TokenKind::rpar) {
            error(nextToken.location, "expected ',' or ')' in argument list");
            skipToken();
        }
    }
    
    if (nextToken.kind == TokenKind::rpar)
        skipToken();
    else {
        error(location, "expected ')' to close argument list");
    }
    
    if(arguments.empty()){
        error(location, "print requires at least one argument");
        return nullptr;
    }
    
    return std::make_unique<PrintExpr>(std::move(location), std::move(arguments));
}

std::unique_ptr<Expr> Parser::parseNumberExpr() {
    auto literal = std::make_unique<NumberLiteral>(nextToken.location, nextToken.value.value());
    skipToken();
    return literal;
}

std::unique_ptr<Expr> Parser::parseStringExpr() {
    auto strliteral = std::make_unique<StringLiteral>(nextToken.location, nextToken.value.value());
    skipToken();
    return strliteral;
}

std::unique_ptr<Expr> Parser::parseParenExpr() {
    skipToken();
    auto v = parseExpr();
    if(!v)
        return nullptr;
    if(nextToken.kind != TokenKind::rpar){
        error(nextToken.location, "expected ')' to close expression with parentheses");
        return nullptr; 
    }
    skipToken();
    return v;
}

std::unique_ptr<Stmt> Parser::parseReturnStmt(){
    SourceLocation location = nextToken.location;
    skipToken(); // skip return token
    
    std::unique_ptr<Expr> exp;
    if (nextToken.kind != TokenKind::semi) {
        exp = parseExpr();
        if (!exp) {
            error(nextToken.location, "expected expression in the return statement");
            return nullptr; 
        }
    }
    
    if (nextToken.kind != TokenKind::semi) {
        error(nextToken.location, "expected ';' at the end of the return statement");
        return nullptr; 
    }
    skipToken();
    return std::make_unique<ReturnStmt>(location, std::move(exp));
} 


std::unique_ptr<VariableDecl> Parser::parseVariableDecl(){
    SourceLocation location = nextToken.location;
    skipToken(); // skip 'int' token
    
    if (nextToken.kind != TokenKind::identifier) {
        error(nextToken.location, "expected identifier after type in variable declaration");
        return nullptr;
    }
    
    std::string varName = nextToken.value.value();
    skipToken(); // skip identifier
    
    std::unique_ptr<Expr> initializer;
    
    // Check for optional initializer
    if (nextToken.kind == TokenKind::equal) {
        skipToken(); // skip '='
        initializer = parseExpr();
        if (!initializer) {
            error(nextToken.location, "expected expression after '=' in variable declaration");
            return nullptr;
        }
    }
    
    // We now support 'int' type
    return std::make_unique<VariableDecl>(location, varName, "int", std::move(initializer));
}


int Parser::getTokPrecedence() {
    switch (nextToken.kind) {
        case TokenKind::plus:
        case TokenKind::minus:
            return 10;
        case TokenKind::mul:
        case TokenKind::slash:
        case TokenKind::percent:
            return 20;
        case TokenKind::lessthan:
        case TokenKind::greaterthan:
        case TokenKind::less_equal:
        case TokenKind::great_equal:
        case TokenKind::doublequal:
        case TokenKind::not_equal:
            return 5;
        case TokenKind::amp_amp:
            return 3;
        case TokenKind::pipe_pipe:
            return 2;
        default:
            return -1;
    }
}

std::unique_ptr<Expr> Parser::parseBooleanExpr() {
    bool value = (nextToken.kind == TokenKind::cf_true);
    auto literal = std::make_unique<BooleanLiteral>(nextToken.location, value);
    skipToken();
    return literal;
}

std::unique_ptr<Expr> Parser::parsePrimaryExpr() {
    switch (nextToken.kind){
        default:
            error(nextToken.location, "expected expression");
            skipToken();
            return nullptr; 
        case TokenKind::number:
            return parseNumberExpr();
        case TokenKind::string_literal:
            return parseStringExpr();
        case TokenKind::lpar:
            return parseParenExpr();
        case TokenKind::identifier:
            return parseIdentifierExpr();
        case TokenKind::print:
            return parsePrintExpr();
        case TokenKind::cf_true:
        case TokenKind::cf_false:
            return parseBooleanExpr();
    }
}

std::unique_ptr<Expr> Parser::parseBinaryExpr(int exprPrec, std::unique_ptr<Expr> lhs) {
    while (true) {
        int tokPrec = getTokPrecedence();
        
        if (tokPrec < exprPrec)
            return lhs;
        
        TokenKind binOp = nextToken.kind;
        SourceLocation opLoc = nextToken.location;
        skipToken();
        
        auto rhs = parsePrimaryExpr();
        if (!rhs)
            return nullptr;
        
        int nextPrec = getTokPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinaryExpr(tokPrec + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }
        
        lhs = std::make_unique<BinaryExpr>(opLoc, std::move(lhs), binOp, std::move(rhs));
    }
}

std::unique_ptr<Expr> Parser::parseExpr(){
    auto lhs = parsePrimaryExpr();
    if (!lhs)
        return nullptr;
    
    if (auto declRef = dynamic_cast<DeclRefExpr*>(lhs.get())) {
        if (nextToken.kind == TokenKind::equal) {
            SourceLocation assignLoc = nextToken.location;
            std::string target = declRef->identifier;
            skipToken(); // skip '='
            
            auto rhs = parseExpr();
            if (!rhs) {
                error(nextToken.location, "expected expression after '=' in assignment");
                return nullptr;
            }
            
            return std::make_unique<AssignmentExpr>(assignLoc, target, std::move(rhs));
        }
    }
    
    return parseBinaryExpr(0, std::move(lhs));
}


std::unique_ptr<Stmt> Parser::parseIfStmt() {
    SourceLocation location = nextToken.location;
    skipToken(); // skip 'if'
    
    if (nextToken.kind != TokenKind::lpar) {
        error(nextToken.location, "expected '(' after 'if'");
        return nullptr;
    }
    skipToken(); // skip '('
    
    auto condition = parseExpr();
    if (!condition) {
        error(nextToken.location, "expected condition expression in if statement");
        return nullptr;
    }
    
    if (nextToken.kind != TokenKind::rpar) {
        error(nextToken.location, "expected ')' after if condition");
        return nullptr;
    }
    skipToken(); // skip ')'
    
    if (nextToken.kind != TokenKind::lbrace) {
        error(nextToken.location, "expected '{' after if condition");
        return nullptr;
    }
    
    auto thenBlock = parseBlock();
    if (!thenBlock) {
        return nullptr;
    }
    
    std::unique_ptr<Block> elseBlock;
    if (nextToken.kind == TokenKind::cf_else) {
        skipToken(); // skip 'else'
        
        if (nextToken.kind != TokenKind::lbrace) {
            error(nextToken.location, "expected '{' after 'else'");
            return nullptr;
        }
        
        elseBlock = parseBlock();
        if (!elseBlock) {
            return nullptr;
        }
    }
    
    return std::make_unique<IfStmt>(location, std::move(condition), std::move(thenBlock), std::move(elseBlock));
}

std::unique_ptr<Stmt> Parser::parseWhileStmt() {
    SourceLocation location = nextToken.location;
    skipToken(); // skip 'while'
    
    if (nextToken.kind != TokenKind::lpar) {
        error(nextToken.location, "expected '(' after 'while'");
        return nullptr;
    }
    skipToken(); // skip '('
    
    auto condition = parseExpr();
    if (!condition) {
        error(nextToken.location, "expected condition expression in while statement");
        return nullptr;
    }
    
    if (nextToken.kind != TokenKind::rpar) {
        error(nextToken.location, "expected ')' after while condition");
        return nullptr;
    }
    skipToken(); // skip ')'
    
    if (nextToken.kind != TokenKind::lbrace) {
        error(nextToken.location, "expected '{' after while condition");
        return nullptr;
    }
    
    auto body = parseBlock();
    if (!body) {
        return nullptr;
    }
    
    return std::make_unique<WhileStmt>(location, std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseStmt() {
    if (nextToken.kind == TokenKind::cf_return)
            return parseReturnStmt();
    
    if (nextToken.kind == TokenKind::cf_if)
            return parseIfStmt();
    
    if (nextToken.kind == TokenKind::cf_while)
            return parseWhileStmt();
    
    if (nextToken.kind == TokenKind::cf_int) {
        auto varDecl = parseVariableDecl();
        if (!varDecl) {
            if (!skipUntil({TokenKind::semi, TokenKind::rbrace}))
                skipToken();
            else if (nextToken.kind == TokenKind::semi)
                skipToken();
            return nullptr;
        }
        
        if (nextToken.kind != TokenKind::semi) {
            error(nextToken.location, "expected ';' after variable declaration");
            skipToken();
            return nullptr;
        }
        skipToken(); // skip ';'
        return varDecl;
    }
            
    auto expr = parseExpr();
    if (!expr) {
        if (!skipUntil({TokenKind::semi, TokenKind::rbrace}))
            skipToken();
        else if (nextToken.kind == TokenKind::semi)
            skipToken();// skips 'semi' token
        return nullptr;
    }

    if (nextToken.kind != TokenKind::semi) {
        error(nextToken.location, "expected ';' after statement");
        skipToken(); // skips 'semi' token
        return nullptr;
    }

    skipToken();
    return expr;
}

std::unique_ptr<Block> Parser::parseBlock() {
    SourceLocation location = nextToken.location;
    skipToken(); // skip '{'
    
    std::vector<std::unique_ptr<Stmt>> statements;

    while (nextToken.kind != TokenKind::rbrace && nextToken.kind != TokenKind::eof) {
        auto stmt = parseStmt();
        if (stmt) {
            statements.push_back(std::move(stmt));
        } else {
            // Always advance at least once on failure
            if (nextToken.kind != TokenKind::rbrace && nextToken.kind != TokenKind::eof)
                skipToken();
        }
    }

    if (nextToken.kind != TokenKind::rbrace) {
        error(location, "expected '}' at the end of block");
        return nullptr;
    }

    skipToken(); // skips '}' token
    return std::make_unique<Block>(location, std::move(statements));
}

std::unique_ptr<ParamDecl> Parser::parseParams(){
    SourceLocation paramloc = nextToken.location;
    std::string paramname = nextToken.value.value();
    skipToken(); // skips 'pameter identifier' token
    
    if(nextToken.kind != TokenKind::colon){
        error(nextToken.location, "expected ':' after the parameter name");
        return nullptr;
    }
    skipToken(); // skips ':' token
    
    if(nextToken.kind != TokenKind::identifier){
        error(nextToken.location, "expected type name after ':'");
        return nullptr;
    }
    std::string typname(nextToken.value.value());
    skipToken();// skips 'parameter type' token
    return std::make_unique<ParamDecl>(paramloc, paramname, typname);
}

std::unique_ptr<FunctionDecl> Parser::parseFunction() {
    SourceLocation funcLoc = nextToken.location;
    skipToken(); // skips 'func' token

    if (nextToken.kind != TokenKind::identifier){
        error(nextToken.location, "expected function name after 'func'");
        return nullptr;
    }
    std::string funcName = nextToken.value.value();
    skipToken(); // skips func name token

    if (nextToken.kind != TokenKind::lpar){
        error(nextToken.location, "expected '(' after function name");
        return nullptr;
    }
    skipToken(); // skips '(' token
    
    std::vector<std::unique_ptr<ParamDecl>> params;

    while (nextToken.kind != TokenKind::rpar && nextToken.kind != TokenKind::eof) {
        if (nextToken.kind != TokenKind::identifier){
            error(nextToken.location, "expected parameter name");
            if (!skipUntil({TokenKind::comma, TokenKind::rpar}))
                break;
            if (nextToken.kind == TokenKind::comma)
                skipToken();// skips ',' token
            continue;
        }
        
        if (auto param = parseParams())
            params.push_back(std::move(param));
        else {
            if (!skipUntil({TokenKind::comma, TokenKind::rpar}))
                break;
        }

        if (nextToken.kind == TokenKind::comma)
            skipToken(); // skips ',' token
    }

    if (nextToken.kind == TokenKind::rpar)
        skipToken(); // skips ')' token

    // Expect ':' before return type
    if (nextToken.kind != TokenKind::colon){
        error(nextToken.location, "expected ':' after ')'");
        return nullptr;
    }
    skipToken();// skips ':' token
    
    if (nextToken.kind != TokenKind::identifier){
        error(nextToken.location, "expected return type after ':'");
        return nullptr;
    }
    std::string funcType = nextToken.value.value();
    skipToken(); // skips func type token

    if (nextToken.kind != TokenKind::lbrace){
        error(nextToken.location, "expected '{' to begin function body");
        return nullptr;
    }
    auto body = parseBlock();
    if (!body)
        return nullptr;

    return std::make_unique<FunctionDecl>(
        funcLoc, funcName, funcType, std::move(params), std::move(body));
}

std::vector<std::unique_ptr<FunctionDecl>> Parser::parseProgram(){
    std::vector<std::unique_ptr<FunctionDecl>> functions;

    while(nextToken.kind != TokenKind::eof){
        if(nextToken.kind != TokenKind::func){
            error(nextToken.location, "only 'func' declarations allowed at top level");
            if(!skipUntil({TokenKind::func, TokenKind::eof})) 
                skipToken(); 
            continue;
        }
        if (auto func = parseFunction()){
            functions.push_back(std::move(func));
        } else {
            if(!skipUntil({TokenKind::func, TokenKind::eof})) 
                skipToken(); 
        }
    }
    return functions;
}