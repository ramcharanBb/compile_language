#ifndef UTILS_H
#define UTILS_H
#include "llvm/ADT/StringRef.h"
#include <iostream>
#include <string>

struct Dumpable{
    void indent(size_t level){
        for(size_t i =0;i<level;i++){
            std::cerr << " ";
        }
    }
    virtual void dump(size_t level =0) =0;
};
struct SourceLocation{
    std::string filepath;
    int line;
    int col;
};

struct SourceFile{
    std::string path;
    std::string buffer;
};


#endif