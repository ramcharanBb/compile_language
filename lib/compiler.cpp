#include <iostream>
#include <memory>
#include <system_error>
#include <string>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "sema.h"

namespace cl = llvm::cl;

static cl::opt<std::string> inputFilename(
    cl::Positional,
    cl::desc("<input file>"),
    cl::init("-"),
    cl::value_desc("filename")
);

int main(int argc ,const char **argv){
    cl::ParseCommandLineOptions(argc, argv, "My Compiler\n");
    std::cout << "Input file: " << inputFilename << "\n";
    
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
      llvm::MemoryBuffer::getFileOrSTDIN(inputFilename);
    if(std::error_code ec = fileOrErr.getError()){
        llvm::errs() << "Could not open input file: " << ec.message() << "\n"; 
    }
    auto buffer = fileOrErr.get()->getBuffer();
    
    SourceFile sourceFile{inputFilename,buffer.str()};

    TheLexer lexer{sourceFile};

    lexer.debugPrintAllTokens();

    Parser parse{lexer};
    auto parsedprogram = parse.parseProgram();
    for (auto &&fn : parsedprogram) {
      fn->dump();
     }
    std::cerr << "\n\n";
    std::cerr << "------------------AST After Semantic Analysis-----------------------"<<std::endl;
    SemanticAnalysis sema(std::move(parsedprogram));
    auto resolvedAST = sema.resolve();
    for (auto &&fn : resolvedAST)
      fn.dump();


    std::cerr << "\n\n";
    return 0;
}