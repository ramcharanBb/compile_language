add_executable(ram-compiler
    compiler.cpp
    lexer.cpp
    parser.cpp
    codegen.cpp
    Mypass.cpp
    MyPassBBmerge.cpp 
)

# Map LLVM components to the actual libraries you need
llvm_map_components_to_libnames(llvm_libs 
    Core
    Support
    Passes
    TransformUtils
    Analysis
)

target_link_libraries(ram-compiler PRIVATE ${llvm_libs})

target_include_directories(ram-compiler PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
)

# Build MyPass as a loadable plugin (shared library)
# add_library(MyPass MODULE
#     Mypass.cpp
# )

# # Link MyPass against LLVM libraries
# target_link_libraries(MyPass PRIVATE ${llvm_libs})

# target_include_directories(MyPass PRIVATE
#     ${PROJECT_SOURCE_DIR}/include
#     ${PROJECT_BINARY_DIR}/include
# )

# # Set proper flags for the plugin
# set_target_properties(MyPass PROPERTIES
#     PREFIX ""  # Don't add 'lib' prefix
#     SUFFIX ".so"  # Use .so extension
# )