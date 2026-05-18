cmake_minimum_required(VERSION 3.10)
project(CVM_PlusPlus VERSION 1.1.0)

# Target C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build the main executable
add_executable(cvm
    src/main.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/compiler.cpp
    src/vm.cpp
    src/disasm.cpp
)

# Enforce strict warnings (Werror) across GCC/Clang/MSVC
target_compile_options(cvm PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wshadow -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)
