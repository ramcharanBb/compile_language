add_executable(ram-compiler
    compiler.cpp
    lexer.cpp
    parser.cpp
)

# Map LLVM components to the actual libraries you need
llvm_map_components_to_libnames(llvm_libs 
    Core
    Support
    # Add other components as needed: IRReader, Passes, etc.
)

target_link_libraries(ram-compiler PRIVATE ${llvm_libs})

target_include_directories(ram-compiler PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
)