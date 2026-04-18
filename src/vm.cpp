#include "vm.h"
#include <iostream>
#include <stdexcept>
#include <limits>

VM::VM() : sp(0) {}

void VM::push(int32_t value) {
    if (sp >= STACK_MAX) throw std::runtime_error("Stack overflow");
    stack[sp++] = value;
}

int32_t VM::pop() {
    if (sp == 0) throw std::runtime_error("Stack underflow");
    return stack[--sp];
}

int32_t VM::peekStack() const {
    if (sp == 0) return 0;
    return stack[sp - 1];
}

void VM::execute(const Chunk& chunk) {
    size_t savedSp = sp;
    try {
        size_t ip = 0; // instruction pointer
        while (ip < chunk.code.size()) {
        Opcode instruction = static_cast<Opcode>(chunk.code[ip++]);

        switch (instruction) {
            case Opcode::PUSH_INT: {
                int32_t val = chunk.readInt(ip);
                ip += 4;
                push(val);
                break;
            }
            case Opcode::PUSH_BOOL: {
                int32_t val = chunk.code[ip++];
                push(val);
                break;
            }
            case Opcode::ADD: {
                int32_t b = pop();
                int32_t a = pop();
                int64_t result = static_cast<int64_t>(a) + static_cast<int64_t>(b);
                if (result > INT32_MAX || result < INT32_MIN) throw std::runtime_error("Integer overflow in addition");
                push(static_cast<int32_t>(result));
                break;
            }
            case Opcode::SUB: {
                int32_t b = pop();
                int32_t a = pop();
                int64_t result = static_cast<int64_t>(a) - static_cast<int64_t>(b);
                if (result > INT32_MAX || result < INT32_MIN) throw std::runtime_error("Integer overflow in subtraction");
                push(static_cast<int32_t>(result));
                break;
            }
            case Opcode::MUL: {
                int32_t b = pop();
                int32_t a = pop();
                int64_t result = static_cast<int64_t>(a) * static_cast<int64_t>(b);
                if (result > INT32_MAX || result < INT32_MIN) throw std::runtime_error("Integer overflow in multiplication");
                push(static_cast<int32_t>(result));
                break;
            }
            case Opcode::DIV: {
                int32_t b = pop();
                int32_t a = pop();
                if (b == 0) throw std::runtime_error("Division by zero");
                if (a == INT32_MIN && b == -1) throw std::runtime_error("Integer overflow in division");
                push(a / b);
                break;
            }
            case Opcode::MOD: {
                int32_t b = pop();
                int32_t a = pop();
                if (b == 0) throw std::runtime_error("Modulo by zero");
                if (a == INT32_MIN && b == -1) throw std::runtime_error("Integer overflow in modulo");
                push(a % b);
                break;
            }
            case Opcode::EQ: {
                int32_t b = pop();
                int32_t a = pop();
                push(a == b ? 1 : 0);
                break;
            }
            case Opcode::NEQ: {
                int32_t b = pop();
                int32_t a = pop();
                push(a != b ? 1 : 0);
                break;
            }
            case Opcode::LT: {
                int32_t b = pop();
                int32_t a = pop();
                push(a < b ? 1 : 0);
                break;
            }
            case Opcode::LTE: {
                int32_t b = pop();
                int32_t a = pop();
                push(a <= b ? 1 : 0);
                break;
            }
            case Opcode::GT: {
                int32_t b = pop();
                int32_t a = pop();
                push(a > b ? 1 : 0);
                break;
            }
            case Opcode::GTE: {
                int32_t b = pop();
                int32_t a = pop();
                push(a >= b ? 1 : 0);
                break;
            }
            case Opcode::BIT_XOR: {
                int32_t b = pop();
                int32_t a = pop();
                push(a ^ b);
                break;
            }
            case Opcode::BIT_AND: {
                int32_t b = pop();
                int32_t a = pop();
                push(a & b);
                break;
            }
            case Opcode::BIT_OR: {
                int32_t b = pop();
                int32_t a = pop();
                push(a | b);
                break;
            }
            case Opcode::SHL: {
                int32_t b = pop();
                int32_t a = pop();
                if (b < 0 || b >= 32) throw std::runtime_error("Invalid shift amount");
                push(static_cast<int32_t>(static_cast<uint32_t>(a) << b));
                break;
            }
            case Opcode::SHR: {
                int32_t b = pop();
                int32_t a = pop();
                if (b < 0 || b >= 32) throw std::runtime_error("Invalid shift amount");
                push(a >> b);
                break;
            }
            case Opcode::BIT_NOT: {
                int32_t a = pop();
                push(~a);
                break;
            }
            case Opcode::NOT: {
                int32_t a = pop();
                push(a == 0 ? 1 : 0);
                break;
            }
            case Opcode::NORMALIZE: {
                int32_t a = pop();
                push(a == 0 ? 0 : 1);
                break;
            }
            case Opcode::NEG: {
                int32_t a = pop();
                if (a == INT32_MIN) throw std::runtime_error("Integer overflow in negation");
                push(-a);
                break;
            }
            case Opcode::POP: {
                pop();
                break;
            }
            case Opcode::SET_VAR: {
                int32_t id = chunk.readInt(ip);
                ip += 4;
                int32_t val = pop();
                if (id < 0) throw std::runtime_error("Negative variable ID");
                if (id >= static_cast<int32_t>(globals.size())) {
                    if (id > 100000) throw std::runtime_error("Too many variables");
                    globals.resize(static_cast<size_t>(id) + 1, 0);
                }
                globals[id] = val;
                break;
            }
            case Opcode::SET_VAR_PUSH: {
                int32_t id = chunk.readInt(ip);
                ip += 4;
                int32_t val = pop();
                if (id < 0) throw std::runtime_error("Negative variable ID");
                if (id >= static_cast<int32_t>(globals.size())) {
                    if (id > 100000) throw std::runtime_error("Too many variables");
                    globals.resize(static_cast<size_t>(id) + 1, 0);
                }
                globals[id] = val;
                push(val);
                break;
            }
            case Opcode::GET_VAR: {
                int32_t id = chunk.readInt(ip);
                ip += 4;
                if (id < 0 || id >= static_cast<int32_t>(globals.size())) {
                    throw std::runtime_error("Undefined variable read");
                }
                push(globals[id]);
                break;
            }
            case Opcode::JMP: {
                int32_t offset = chunk.readInt(ip);
                ip += 4;
                size_t newIp = static_cast<size_t>(static_cast<int64_t>(ip) + offset);
                if (newIp > chunk.code.size()) throw std::runtime_error("Jump target out of bounds");
                ip = newIp;
                break;
            }
            case Opcode::JMP_IF_FALSE: {
                int32_t offset = chunk.readInt(ip);
                ip += 4;
                int32_t cond = pop();
                if (cond == 0) {
                    size_t newIp = static_cast<size_t>(static_cast<int64_t>(ip) + offset);
                    if (newIp > chunk.code.size()) throw std::runtime_error("Jump target out of bounds");
                    ip = newIp;
                }
                break;
            }
            case Opcode::PRINT: {
                int32_t val = pop();
                std::cout << val << std::endl;
                break;
            }
            case Opcode::INPUT: {
                int32_t val;
                if (!(std::cin >> val)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    throw std::runtime_error("Invalid input: expected an integer");
                }
                push(val);
                break;
            }
            case Opcode::HALT: {
                return;
            }
            default:
                throw std::runtime_error("Unknown Opcode");
        }
    }
    throw std::runtime_error("Execution reached end of chunk without HALT");
    } catch (...) {
        sp = savedSp;
        throw;
    }
}
