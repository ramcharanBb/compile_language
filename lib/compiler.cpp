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
#include "codegen.h"

namespace cl = llvm::cl;

static cl::opt<std::string> inputFilename(
    cl::Positional,
    cl::desc("<input file>"),
    cl::init("-"),
    cl::value_desc("filename")
);

int main(int argc, const char **argv) {
    cl::ParseCommandLineOptions(argc, argv, "My Compiler\n");
    std::cout << "Input file: " << inputFilename << "\n";
    
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrErr =
      llvm::MemoryBuffer::getFileOrSTDIN(inputFilename);
    if (std::error_code ec = fileOrErr.getError()) {
        llvm::errs() << "Could not open input file: " << ec.message() << "\n"; 
        return 1;
    }
    auto buffer = fileOrErr.get()->getBuffer();
    
    SourceFile sourceFile{inputFilename, buffer.str()};

    TheLexer lexer{sourceFile};

    // lexer.debugPrintAllTokens();

    Parser parse{lexer};
    auto parsedprogram = parse.parseProgram();
    
    // std::cerr << "\n------------------AST Before Semantic Analysis-----------------------\n";
    // for (auto &&fn : parsedprogram) {
    //     fn->dump();
    // }
    SemanticAnalysis sema(parsedprogram);
    bool success = sema.resolve();
    
    if (!success) {
        std::cerr << "\nSemantic analysis failed!\n";
        return 1;
    }
    
    std::cerr << "\n------------------AST After Semantic Analysis------------------------\n";
    for (auto &&fn : parsedprogram) {
        fn->dump();
    }
    std::cerr << "\n------------------LLVM IR------------------------\n";
    Codegen codegen; 
    codegen.generate(parsedprogram);
    std::cerr << "\n\n";
    
    // Generate object file
    std::cerr << "------------------Generating Object File------------------------\n";
    if (codegen.GenerateObjectFile("output.o")) {
        std::cerr << "Object file generated successfully: output.o\n";
    } else {
        std::cerr << "Failed to generate object file\n";
        return 1;
    }
    
    return 0;
}