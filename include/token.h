#ifndef TOKEN_H
#define TOKEN_H

#include <optional>
#include <string>
#include <iostream>

#include "utils.h"

enum class TokenKind{
     eof,unk,negate,

     lpar,rpar,

     lbrace,rbrace,
     
     colon,semi,comma,
     
     doublequal,equal,
     
     lessthan,greaterthan,
     increament,decreament,

     plus_equal,minus_equal,mul_equal,less_equal,great_equal,not_equal,

     plus,minus,mul,slash,percent,
     
     amp_amp, pipe_pipe,

     func,identifier, 
     string_literal, number,print,
     
     cf_return, cf_if, cf_else, cf_while, cf_var, cf_true, cf_false,
};

struct Token{
    SourceLocation location;
    TokenKind kind;
    std::optional <std::string> value;


    Token(SourceLocation location, TokenKind kind, std::string value = "")
        : location(location), kind(kind), value(value) {}
    
    static std::string kindToString(TokenKind kind) {
        switch(kind) {
            case TokenKind::eof: return "EOF";
            case TokenKind::lpar: return "LPAR";
            case TokenKind::rpar: return "RPAR";
            case TokenKind::lbrace: return "LBRACE";
            case TokenKind::rbrace: return "RBRACE";
            case TokenKind::colon: return "COLON";
            case TokenKind::semi: return "SEMI";
            case TokenKind::comma: return "COMMA";
            case TokenKind::func: return "FUNCTION";
            case TokenKind::identifier: return "IDENTIFIER";
            case TokenKind::string_literal: return "STRING";
            case TokenKind::number: return "NUMBER";
            case TokenKind::unk: return "UNKNOWN";
            case TokenKind::print: return "PRINT";
            case TokenKind::cf_return: return "RETURN";
            case TokenKind::cf_if: return "IF";
            case TokenKind::cf_else: return "ELSE";
            case TokenKind::cf_while: return "WHILE";
            case TokenKind::cf_var: return "VAR";
            case TokenKind::cf_true: return "TRUE";
            case TokenKind::cf_false: return "FALSE";
            case TokenKind::slash: return "SLASH";
            case TokenKind::percent: return "PERCENT";
            case TokenKind::amp_amp: return "AMP_AMP";
            case TokenKind::pipe_pipe: return "PIPE_PIPE";
            case TokenKind::doublequal: return "DOUBLE_EQUAL";
            case TokenKind::equal: return "EQUAL";
            case TokenKind::lessthan: return "LESS_THAN";
            case TokenKind::greaterthan: return "GREATER_THAN";
            case TokenKind::increament: return "INCREMENT";
            case TokenKind::decreament: return "DECREMENT";
            case TokenKind::plus_equal: return "PLUS_EQUAL";
            case TokenKind::minus_equal: return "MINUS_EQUAL";
            case TokenKind::mul_equal: return "MUL_EQUAL";
            case TokenKind::less_equal: return "LESS_EQUAL";
            case TokenKind::great_equal: return "GREAT_EQUAL";
            case TokenKind::not_equal: return "NOT_EQUAL";
            case TokenKind::plus: return "PLUS";
            case TokenKind::minus: return "MINUS";
            case TokenKind::mul: return "MUL";
            case TokenKind::negate: return "NEGATE";
            default: return "INVALID";
        }
    }
    
    void print() const {
        std::cout << "[" << location.filepath << ":" << location.line << ":" << location.col << "] "
                  << kindToString(kind);
        if (value.has_value()) {
            std::cout << " \"" << value.value() << "\"";
        }
        std::cout << std::endl;
    }
};


#endif