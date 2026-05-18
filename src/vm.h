#pragma once
#include "opcode.h"
#include "chunk.h"
#include <array>
#include <vector>

static_assert(sizeof(int32_t) == 4, "int32_t must be 4 bytes");

class VM {
public:
    static constexpr int32_t MAX_VARIABLES = 65536;

    VM();
    void execute(const Chunk& chunk);
    void reset() { sp = 0; variables.clear(); }

    // For testing/debugging, we can inspect stack or variables
    [[nodiscard]] int32_t peekStack() const;

private:
    static constexpr size_t STACK_MAX = 2048;
    std::array<int32_t, STACK_MAX> stack;
    size_t sp; // Stack pointer

    std::vector<int32_t> variables;  // flat-addressable variable slots

    void push(int32_t value);
    int32_t pop();
};
