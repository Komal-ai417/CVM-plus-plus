# CVM++

**CVM++** is a high-performance, stack-based Virtual Machine and Compiler engineered entirely from scratch in modern C++. It's designed to be a robust, memory-safe, and blazingly fast execution engine.

Featuring strict architectural protections, CVM++ executes custom typed instructions securely-handling lexical block scoping, runtime boundary mitigations, constant folding optimizations, and standard execution operations smoothly.

## Key Features

*   **Custom Stack-Based VM**: Clean `size_t` instruction pointer execution block, wrapping Two's Complement architectures naturally.
*   **Single-Pass Compiler Pipeline**: Translates AST chunks linearly into tightly packed `std::vector<uint8_t>` executable bytecode dynamically!
*   **Deep Block Scoping**: Deep-mapped variable shadowing and block scope bindings. Nested scopes clean up sequentially, dropping variable collisions gracefully.
*   **Compiler Optimization Layer**: 
    *   Dynamic **Constant Folding** recursively reducing binary footprint structures drastically.
    *   **Short-Circuiting** logical operand evaluators (`&&`, `||`) reducing trace overhead.
*   **Secure & Hardened Run-time**:
    *   Integer boundaries trap native `<cstdint>` overflows explicitly resolving division drops without bounds-crashing!
    *   Lexical syntax parsing failures trace recursive scopes natively discarding memory leaks cleanly.
    *   Undefined Behavior (UB) mitigated deeply through execution verification limits.
*   **Interactive REPL Layer**: Build code, drop blocks, execute line-by-line dynamically tracking states!

---

## Building the Project

### Prerequisites

*   A C++14 (or newer) compatible compiler (GCC, Clang, or MSVC).
*   CMake 3.10+

### One-Command Build and Run (No CMake)

Use this single line in environments where `cmake` is unavailable:

```bash
g++ -std=c++14 -Wall -Wextra -O2 src/main.cpp src/lexer.cpp src/ast.cpp src/parser.cpp src/compiler.cpp src/vm.cpp -o cvm && ./cvm tests/fibonacci.cvm
```

### Build Instructions

You can build CVM++ directly using **CMake**:

```bash
git clone https://github.com/yourusername/CVM_PlusPlus.git
cd CVM_PlusPlus

# Create Build Directory and configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile target executable
cmake --build build --config Release
```

---

## Running CVM++

You can run the executable in **REPL mode** or by supplying a `.cvm` **Script File**.

### REPL Mode

Launch the `cvm` application without arguments to interact with the engine sequentially:

```bash
./build/cvm
> let x = 10;
> if (x > 5) { print x * 20; }
200
```

### Script Execution Mode

Pass your script sequentially! Included in `/tests/` are reference scripts like Fibonacci closures and scope bounds tests:

```bash
./build/cvm tests/scope.cvm
./build/cvm tests/fibonacci.cvm
```

---

## The CVM++ Language

### Overview

A quick intro to the semantics governing execution logic internally:

*   **Variables**: Defined organically with `let`. (Variables track values locally out to deep nested scopes natively).
*   **Scopes**: Wrapped dynamically in brackets `{ }`.
*   **Conditionals/Loops**: `if`, `else`, and `while`.
*   **IO Tools**: `print` evaluating expressions against standard streams. `input` bindings taking numeric streams dynamically.

### Operators Supported

| Type | Operators | 
| :--- | :--- | 
| **Arithmetic** | `+`, `-`, `*`, `/`, `%` |
| **Logic/Boolean** | `&&`, `||`, `!`, `==`, `!=`, `<`, `>`, `<=`, `>=` |
| **Bitwise** | `&`, `|`, `^`, `<<`, `>>`, `~` |

### Code Example

```javascript
let limit = 10;
let x = 0;
while (x < limit) {
    if (x % 2 == 0) {
        print x;
    }
    x = x + 1;
}
```

---

## Architecture Layers

```text
[ Lexer ] ->  [ Parser (AST) ]   ->    [ Compiler ]    ->     [ Virtual Machine ]
  (Text)       (Syntax Tree)      (Bytecode Chunks)        (Stack Execution)
```

1.  **Lexer/Scanner**: Implements highly optimized static String referencing traversing source text natively into atomic `Token` sets dynamically safely parsing constraints without memory bloating allocations. 
2.  **Parser**: Standard Recursive Descent / Pratt architecture generating securely integrated AST implementations securely isolating block syntax.
3.  **Compiler**: Emits nested bytecode chunks iteratively routing logic bindings tracking variable allocations cleanly isolated internally across native pools via `std::vector` mapping!
4.  **Virtual Machine**: Master interpreter loop bounds-checking execution tracing directly returning safe outputs trapping exceptions natively.

---

## License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for more information.
