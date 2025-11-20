#ifndef LEXER_H
#define LEXER_H
#include<string>
#include <vector>
#include "token.h"
#include "utils.h"

class TheLexer{
  private:
  const SourceFile *sourceFile;
  size_t idx=0;
  int line =0;
  int column =0 ;
  char lastChar = ' ';
  SourceLocation getSourceLocation(){
    return SourceLocation{sourceFile->path, line, column};
  }
  char peekNextChar() {
     if (idx >= sourceFile->buffer.size()) return EOF;
     return sourceFile-> buffer[idx];}
  char getNextChar(){
    if (idx >= sourceFile->buffer.size()) return EOF;
    lastChar = sourceFile->buffer[idx++];
    if(lastChar == '\n'){ 
        ++line;
        column=0;
    }else {
        column++;
    }
    return lastChar;
  }
  public :
  explicit TheLexer (const SourceFile &sourceFile): sourceFile(&sourceFile){}
  Token getNextToken();

  void debugPrintAllTokens() {
        std::cout << "\n----------lexer----------\n" << std::endl;
        
        std::vector<Token> tokens;
        Token tok = getNextToken();
        
        while (tok.kind != TokenKind::eof) {
            tok.print();
            tokens.push_back(tok);
            tok = getNextToken();
        }
        
        // Print EOF token
        tok.print();
        
        std::cout << "\n-------- TOTAL TOKENS:" << tokens.size() + 1 
                  << "------------\n" << std::endl;
        
        // Reset lexer state for reuse
        idx = 0;
        line = 0;
        column = 0;
        std::cout << "\n--------AST BEFORE SEMANTIC ANALYSIS----------------"<< std::endl;
    }
};

#endif