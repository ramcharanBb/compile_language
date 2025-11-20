cmake_minimum_required(VERSION 3.20)
project(ram-compiler LANGUAGES CXX C)  # Add C language here

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find LLVM (auto-detects LLVMConfig.cmake from your LLVM repo/build)
find_package(LLVM REQUIRED CONFIG)

# Optional: debug output for paths
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM CMake dir: ${LLVM_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library dirs: ${LLVM_BUILD_LIBRARY_DIR}")

# Add LLVM CMake modules for modern targets
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
include(HandleLLVMOptions)

# Include directories
include_directories(
    ${LLVM_INCLUDE_DIRS}             # LLVM headers
    ${PROJECT_SOURCE_DIR}/include    # Your headers
    ${PROJECT_BINARY_DIR}/include 
    ${CMAKE_CURRENT_BINARY_DIR}/include   # Generated headers (if any)
)

# Link directories (helps CMake locate LLVM libs if needed)
link_directories(${LLVM_BUILD_LIBRARY_DIR})

# Add LLVM compile definitions
add_definitions(${LLVM_DEFINITIONS})

# Add subdirectory
add_subdirectory(lib)