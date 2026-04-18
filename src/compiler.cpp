#include "compiler.h"
#include <stdexcept>

Compiler::Compiler() : variablesCount(0), currentDepth(0) {}

Chunk Compiler::compile(const std::vector<std::unique_ptr<Statement>>& statements) {
    chunk = Chunk();
    chunk.code.reserve(1024);
    // Do not clear variables here; REPL relies on persisting local variables table.
    for (const auto& stmt : statements) {
        stmt->accept(*this);
    }
    chunk.write(static_cast<uint8_t>(Opcode::HALT));
    return chunk;
}

int32_t Compiler::resolveVariable(const std::string& name, bool declare) {
    if (declare) {
        for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
            if (it->depth < currentDepth) break;
            if (it->name == name) throw std::runtime_error("Variable '" + name + "' already declared in this scope.");
        }
        // Needs declaring
        int32_t id;
        if (!freeIds.empty()) {
            id = freeIds.back();
            freeIds.pop_back();
        } else {
            id = variablesCount++;
        }
        locals.push_back({name, currentDepth, id});
        return id;
    }
    for (auto it = locals.rbegin(); it != locals.rend(); ++it) {
        if (it->name == name) return it->id;
    }
    throw std::runtime_error("Undeclared variable '" + name + "'");
}

void Compiler::beginScope() {
    currentDepth++;
}

void Compiler::endScope() {
    currentDepth--;
    while (!locals.empty() && locals.back().depth > currentDepth) {
        freeIds.push_back(locals.back().id);
        locals.pop_back();
    }
}

int Compiler::emitJmp(Opcode instruction) {
    chunk.write(static_cast<uint8_t>(instruction));
    chunk.writeInt(0); 
    return chunk.code.size() - 4;
}

void Compiler::patchJmp(int offsetIndex) {
    int32_t jump = chunk.code.size() - offsetIndex - 4;
    chunk.code[offsetIndex] = (jump >> 24) & 0xFF;
    chunk.code[offsetIndex + 1] = (jump >> 16) & 0xFF;
    chunk.code[offsetIndex + 2] = (jump >> 8) & 0xFF;
    chunk.code[offsetIndex + 3] = jump & 0xFF;
}

void Compiler::emitLoop(int loopStart) {
    chunk.write(static_cast<uint8_t>(Opcode::JMP));
    // After the VM reads the 4-byte operand, ip will be at (chunk.code.size() + 4).
    // We need to jump back to loopStart, so offset = loopStart - (here + 4).
    int32_t offset = chunk.code.size() - loopStart + 4;
    chunk.writeInt(-offset);
}

void Compiler::visitBinaryExpr(BinaryExpr& expr) {
    if (auto* leftLit = dynamic_cast<LiteralExpr*>(expr.left.get())) {
        if (auto* rightLit = dynamic_cast<LiteralExpr*>(expr.right.get())) {
            if (leftLit->value.type == TokenType::NUMBER && rightLit->value.type == TokenType::NUMBER) {
                int32_t a = std::stoi(leftLit->value.value);
                int32_t b = std::stoi(rightLit->value.value);
                int64_t result;
                switch (expr.op.type) {
                    case TokenType::PLUS:  result = static_cast<int64_t>(a) + static_cast<int64_t>(b); break;
                    case TokenType::MINUS: result = static_cast<int64_t>(a) - static_cast<int64_t>(b); break;
                    case TokenType::STAR:  result = static_cast<int64_t>(a) * static_cast<int64_t>(b); break;
                    case TokenType::SLASH:
                        if (b == 0) throw std::runtime_error("Division by zero in constant expression");
                        if (a == INT32_MIN && b == -1) throw std::runtime_error("Integer overflow in constant division");
                        result = static_cast<int64_t>(a) / b; break;
                    case TokenType::MOD:
                        if (b == 0) throw std::runtime_error("Modulo by zero in constant expression");
                        if (a == INT32_MIN && b == -1) throw std::runtime_error("Integer overflow in constant modulo");
                        result = static_cast<int64_t>(a) % b; break;
                    case TokenType::BIT_XOR: result = a ^ b; break;
                    case TokenType::BIT_AND: result = a & b; break;
                    case TokenType::BIT_OR:  result = a | b; break;
                    case TokenType::SHL:     
                        if (b < 0 || b >= 32) throw std::runtime_error("Invalid shift amount");
                        result = static_cast<int32_t>(static_cast<uint32_t>(a) << b); break;
                    case TokenType::SHR:     
                        if (b < 0 || b >= 32) throw std::runtime_error("Invalid shift amount");
                        result = a >> b; break;
                    case TokenType::EQ_EQ:   result = (a == b) ? 1 : 0; break;
                    case TokenType::BANG_EQ: result = (a != b) ? 1 : 0; break;
                    case TokenType::LESS:    result = (a < b)  ? 1 : 0; break;
                    case TokenType::LESS_EQ: result = (a <= b) ? 1 : 0; break;
                    case TokenType::GREATER: result = (a > b)  ? 1 : 0; break;
                    case TokenType::GREATER_EQ: result = (a >= b) ? 1 : 0; break;
                    default: goto no_fold;
                }
                if (result > INT32_MAX || result < INT32_MIN) throw std::runtime_error("Integer overflow in constant expression");
                chunk.write(static_cast<uint8_t>(Opcode::PUSH_INT));
                chunk.writeInt(static_cast<int32_t>(result));
                return;
            }
        }
    }
no_fold:
    expr.left->accept(*this);
    expr.right->accept(*this);

    switch (expr.op.type) {
        case TokenType::PLUS: chunk.write(static_cast<uint8_t>(Opcode::ADD)); break;
        case TokenType::MINUS: chunk.write(static_cast<uint8_t>(Opcode::SUB)); break;
        case TokenType::STAR: chunk.write(static_cast<uint8_t>(Opcode::MUL)); break;
        case TokenType::SLASH: chunk.write(static_cast<uint8_t>(Opcode::DIV)); break;
        case TokenType::MOD: chunk.write(static_cast<uint8_t>(Opcode::MOD)); break;
        case TokenType::EQ_EQ: chunk.write(static_cast<uint8_t>(Opcode::EQ)); break;
        case TokenType::BANG_EQ: chunk.write(static_cast<uint8_t>(Opcode::NEQ)); break;
        case TokenType::LESS: chunk.write(static_cast<uint8_t>(Opcode::LT)); break;
        case TokenType::LESS_EQ: chunk.write(static_cast<uint8_t>(Opcode::LTE)); break;
        case TokenType::GREATER: chunk.write(static_cast<uint8_t>(Opcode::GT)); break;
        case TokenType::GREATER_EQ: chunk.write(static_cast<uint8_t>(Opcode::GTE)); break;
        case TokenType::BIT_XOR: chunk.write(static_cast<uint8_t>(Opcode::BIT_XOR)); break;
        case TokenType::BIT_AND: chunk.write(static_cast<uint8_t>(Opcode::BIT_AND)); break;
        case TokenType::SHL: chunk.write(static_cast<uint8_t>(Opcode::SHL)); break;
        case TokenType::SHR: chunk.write(static_cast<uint8_t>(Opcode::SHR)); break;
        case TokenType::BIT_OR: chunk.write(static_cast<uint8_t>(Opcode::BIT_OR)); break;
        default: throw std::runtime_error("Unknown binary operator");
    }
}

void Compiler::visitLogicalExpr(LogicalExpr& expr) {
    expr.left->accept(*this);
    
    if (expr.op.type == TokenType::AND_AND) {
        int endJump = emitJmp(Opcode::JMP_IF_FALSE);
        
        expr.right->accept(*this);
        chunk.write(static_cast<uint8_t>(Opcode::NORMALIZE));
        int skipJump = emitJmp(Opcode::JMP);
        
        patchJmp(endJump);
        chunk.write(static_cast<uint8_t>(Opcode::PUSH_INT));
        chunk.writeInt(0);
        
        patchJmp(skipJump);
    } else if (expr.op.type == TokenType::OR_OR) {
        int elseJump = emitJmp(Opcode::JMP_IF_FALSE);
        
        chunk.write(static_cast<uint8_t>(Opcode::PUSH_INT));
        chunk.writeInt(1); // pushes true 
        int endJump = emitJmp(Opcode::JMP);
        
        patchJmp(elseJump);
        expr.right->accept(*this);
        chunk.write(static_cast<uint8_t>(Opcode::NORMALIZE));
        
        patchJmp(endJump);
    } else {
        throw std::runtime_error("Unknown logical operator");
    }
}

void Compiler::visitUnaryExpr(UnaryExpr& expr) {
    expr.right->accept(*this);
    if (expr.op.type == TokenType::BIT_NOT) {
        chunk.write(static_cast<uint8_t>(Opcode::BIT_NOT));
    } else if (expr.op.type == TokenType::NOT) {
        chunk.write(static_cast<uint8_t>(Opcode::NOT));
    } else if (expr.op.type == TokenType::MINUS) {
        chunk.write(static_cast<uint8_t>(Opcode::NEG));
    } else {
        throw std::runtime_error("Unknown unary operator");
    }
}

void Compiler::visitLiteralExpr(LiteralExpr& expr) {
    if (expr.value.type == TokenType::NUMBER) {
        chunk.write(static_cast<uint8_t>(Opcode::PUSH_INT));
        try {
            long long val = std::stoll(expr.value.value);
            if (val > INT32_MAX || val < INT32_MIN) {
                throw std::runtime_error("Integer literal out of range: " + expr.value.value);
            }
            chunk.writeInt(static_cast<int32_t>(val));
        } catch (const std::out_of_range&) {
            throw std::runtime_error("Integer literal out of range: " + expr.value.value);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Invalid integer literal: " + expr.value.value);
        }
    } else if (expr.value.type == TokenType::TRUE_LIT) {
        chunk.write(static_cast<uint8_t>(Opcode::PUSH_BOOL));
        chunk.write(1);
    } else if (expr.value.type == TokenType::FALSE_LIT) {
        chunk.write(static_cast<uint8_t>(Opcode::PUSH_BOOL));
        chunk.write(0);
    } else {
        throw std::runtime_error("Unknown literal type");
    }
}

void Compiler::visitVariableExpr(VariableExpr& expr) {
    int32_t id = resolveVariable(expr.name.value, false);
    chunk.write(static_cast<uint8_t>(Opcode::GET_VAR));
    chunk.writeInt(id);
}

void Compiler::visitInputExpr(InputExpr&) {
    chunk.write(static_cast<uint8_t>(Opcode::INPUT));
}

void Compiler::visitAssignExpr(AssignExpr& expr) {
    expr.value->accept(*this);
    int32_t id = resolveVariable(expr.name.value, false);
    chunk.write(static_cast<uint8_t>(Opcode::SET_VAR_PUSH));
    chunk.writeInt(id);
}

void Compiler::visitBlockStmt(BlockStmt& stmt) {
    beginScope();
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
    endScope();
}

void Compiler::visitExpressionStmt(ExpressionStmt& stmt) {
    stmt.expr->accept(*this);
    chunk.write(static_cast<uint8_t>(Opcode::POP)); 
}

void Compiler::visitLetStmt(LetStmt& stmt) {
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
    } else {
        chunk.write(static_cast<uint8_t>(Opcode::PUSH_INT));
        chunk.writeInt(0);
    }
    int32_t id = resolveVariable(stmt.name.value, true);
    chunk.write(static_cast<uint8_t>(Opcode::SET_VAR));
    chunk.writeInt(id);
}

void Compiler::visitIfStmt(IfStmt& stmt) {
    stmt.condition->accept(*this);
    int thenJmp = emitJmp(Opcode::JMP_IF_FALSE);
    
    stmt.thenBranch->accept(*this);
    
    if (stmt.elseBranch) {
        int elseJmp = emitJmp(Opcode::JMP);
        patchJmp(thenJmp);
        stmt.elseBranch->accept(*this);
        patchJmp(elseJmp);
    } else {
        patchJmp(thenJmp);
    }
}

void Compiler::visitWhileStmt(WhileStmt& stmt) {
    int loopStart = chunk.code.size();
    stmt.condition->accept(*this);
    
    int exitJmp = emitJmp(Opcode::JMP_IF_FALSE);
    stmt.body->accept(*this);
    
    emitLoop(loopStart);
    patchJmp(exitJmp);
}

void Compiler::visitPrintStmt(PrintStmt& stmt) {
    stmt.expr->accept(*this);
    chunk.write(static_cast<uint8_t>(Opcode::PRINT));
}
