cmake_minimum_required(VERSION 3.10)
project(CVM_PlusPlus)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cvm
    src/main.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/compiler.cpp
    src/vm.cpp
)

target_compile_options(cvm PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
