#include "lexer.h"
#include "utils.h"
#include <string>
#include <iostream>

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

    if (c == EOF) return Token{loc, TokenKind::eof};

    // Comments
    if (c == '#') {
        do { c = getNextChar(); } while (c != EOF && c != '\n');
        return getNextToken();
    }
    
    if (c == '/') {
        if (peekNextChar() == '/') {
            do { c = getNextChar(); } while (c != EOF && c != '\n');
            return getNextToken();
        } else if (peekNextChar() == '*') {
            getNextChar(); // eat /
            getNextChar(); // eat *
            c = lastChar;
            while (c != EOF) {
                if (c == '*' && peekNextChar() == '/') {
                    getNextChar(); // eat *
                    getNextChar(); // eat /
                    c = lastChar;
                    break;
                }
                c = getNextChar();
            }
            return getNextToken();
        }
    }

    if (c == '"') {
        std::string value;
        getNextChar(); // eat "
        c = lastChar;
        while (c != '"' && c != EOF) {
            if (c == '\\') {
                c = getNextChar();
                switch (c) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    default: value += c; break;
                }
            } else {
                value += c;
            }
            getNextChar();
            c = lastChar;
        }
        if (c == '"') {
            getNextChar(); // eat closing "
            return Token{loc, TokenKind::string_literal, std::move(value)};
        } else {
            std::cerr << loc.filepath << ":" << loc.line << ":" << loc.col 
                      << ": error: unterminated string literal\n";
            return Token{loc, TokenKind::string_literal, std::move(value)};
        }
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
        if (idStr == "return") return Token{loc, TokenKind::cf_return};
        if (idStr == "if") return Token{loc, TokenKind::cf_if};
        if (idStr == "else") return Token{loc, TokenKind::cf_else};
        if (idStr == "while") return Token{loc, TokenKind::cf_while};
        if (idStr == "var") return Token{loc, TokenKind::cf_var};
        if (idStr == "true") return Token{loc, TokenKind::cf_true};
        if (idStr == "false") return Token{loc, TokenKind::cf_false};
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
        
        case '=': 
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc,TokenKind::doublequal}; }
            getNextChar(); return Token{loc,TokenKind::equal};
            
        case '+': 
            if (peekNextChar() == '+') { getNextChar(); getNextChar(); return Token{loc,TokenKind::increament}; }
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc, TokenKind::plus_equal}; }
            getNextChar(); return Token{loc,TokenKind::plus};
            
        case '-': 
            if (peekNextChar() == '-') { getNextChar(); getNextChar(); return Token{loc,TokenKind::decreament}; }
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc,TokenKind::minus_equal}; }
            getNextChar(); return Token{loc,TokenKind::minus};
            
        case '*': 
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc,TokenKind::mul_equal}; }
            getNextChar(); return Token{loc,TokenKind::mul};
            
        case '/':
            getNextChar(); return Token{loc, TokenKind::slash};
            
        case '%': getNextChar(); return Token{loc, TokenKind::percent};
        
        case '<':
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc,TokenKind::less_equal}; }
            getNextChar(); return Token{loc,TokenKind::lessthan};
            
        case '>':
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc,TokenKind::great_equal}; }
            getNextChar(); return Token{loc,TokenKind::greaterthan};
            
        case '!':
            if (peekNextChar() == '=') { getNextChar(); getNextChar(); return Token{loc,TokenKind::not_equal}; }
            getNextChar(); return Token{loc,TokenKind::negate};
            
        case '&':
            if (peekNextChar() == '&') { getNextChar(); getNextChar(); return Token{loc, TokenKind::amp_amp}; }
            break;
            
        case '|':
            if (peekNextChar() == '|') { getNextChar(); getNextChar(); return Token{loc, TokenKind::pipe_pipe}; }
            break;
    }

    std::cerr << loc.filepath << ":" << loc.line << ":" << loc.col 
              << ": error: unknown character '" << c << "'\n";
    getNextChar();
    return Token{loc, TokenKind::unk};
}