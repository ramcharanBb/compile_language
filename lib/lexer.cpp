#include "lexer.h"
#include "utils.h"
#include <string>

namespace {
bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isNum(char c) {
    return c >= '0' && c <= '9';
}

bool isAlphanum(char c) {
    return (isAlpha(c) || isNum(c));
}

bool iswhitespace(char c) {
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}
}


Token TheLexer::getNextToken() {
    char c = lastChar;
    
    // Skip whitespace
    while (iswhitespace(c)) c = getNextChar();
    
    SourceLocation loc = getSourceLocation();


    if (c == '#') {
        do { c = getNextChar(); } while (c != EOF && c != '\n');
        return getNextToken();
    }


    if (c == '\'') {
        std::string value;
        getNextChar();
        c = lastChar;
        while (c != '\'' && c != EOF) {
            value += c;
            getNextChar();
            c = lastChar;
        }
        if (c == '\'') {
            getNextChar();
            return Token{loc, TokenKind::string_literal, std::move(value)};
        } else {
            std::cerr << loc.filepath << ":" << loc.line << ":" << loc.col 
                      << ": error: unterminated string literal\n";
            return Token{loc, TokenKind::string_literal, std::move(value)};
        }
        
    }

    if (isAlpha(c)) {
        std::string idStr;
        while (isAlphanum(c)){
            idStr += c;
            getNextChar();
            c=lastChar;
        }

        if (idStr == "func") return Token{loc, TokenKind::func};
        if (idStr == "print") return Token{loc, TokenKind::print};
        return Token{loc, TokenKind::identifier, idStr};
    }

    if (isNum(c) ||  (c == '.' && isNum(peekNextChar()))) {
        std::string numStr;
        while (isNum(c) || c == '.'){ 
            numStr += c; 
            getNextChar(); 
            c=lastChar;
        }
        return Token{loc, TokenKind::number, numStr};
    }

    switch (c) {
        case '(': getNextChar(); return Token{loc, TokenKind::lpar};
        case ')': getNextChar(); return Token{loc, TokenKind::rpar};
        case '{': getNextChar(); return Token{loc, TokenKind::lbrace};
        case '}': getNextChar(); return Token{loc, TokenKind::rbrace};
        case ':': getNextChar(); return Token{loc, TokenKind::colon};
        case ';': getNextChar(); return Token{loc, TokenKind::semi};
        case ',': getNextChar(); return Token{loc, TokenKind::comma};
        case EOF: return Token{loc, TokenKind::eof};
        default: break;
    }
    std::cerr << loc.filepath << ":" << loc.line << ":" << loc.col 
              << ": error: unknown character '" << c << "'\n";
    getNextChar();
    return Token{loc, TokenKind::unk};
}