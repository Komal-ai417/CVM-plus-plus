#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

enum class Opcode : uint8_t {
    PUSH_INT,
    PUSH_BOOL,
    ADD, SUB, MUL, DIV, MOD,
    EQ, NEQ, LT, GT, LTE, GTE,
    BIT_XOR, BIT_AND, BIT_OR, SHL, SHR, BIT_NOT,
    NOT, NORMALIZE, NEG, POP,
    SET_VAR,
    GET_VAR,
    SET_VAR_PUSH,
    JMP,
    JMP_IF_FALSE,
    PRINT,
    INPUT,
    HALT
};

struct Chunk {
    std::vector<uint8_t> code;
    
    void write(uint8_t byte) {
        code.push_back(byte);
    }
    
    void writeInt(int32_t value) {
        code.push_back((value >> 24) & 0xFF);
        code.push_back((value >> 16) & 0xFF);
        code.push_back((value >> 8) & 0xFF);
        code.push_back(value & 0xFF);
    }

    int32_t readInt(size_t offset) const {
        if (code.size() < 4 || offset > code.size() - 4) {
            throw std::runtime_error("Bytecode truncated: attempted read past end");
        }
        return static_cast<int32_t>(
               (static_cast<uint32_t>(code[offset]) << 24) |
               (static_cast<uint32_t>(code[offset + 1]) << 16) |
               (static_cast<uint32_t>(code[offset + 2]) << 8) |
               (static_cast<uint32_t>(code[offset + 3]))
        );
    }
};
