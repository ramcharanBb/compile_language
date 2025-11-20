#ifndef TOKEN_H
#define TOKEN_H

#include <optional>
#include <string>
#include <iostream>

#include "utils.h"

enum class TokenKind{
     eof,unk,lpar,rpar,
     lbrace,rbrace,
     colon,semi,comma,
     func,identifier, 
     string_literal, number,print,
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
            default: return "INVALID";
        }
    }
    
    // Print token information
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