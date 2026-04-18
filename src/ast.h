#pragma once
#include "token.h"
#include <memory>
#include <vector>

class ASTVisitor;

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

struct Expression : public ASTNode {};
struct Statement : public ASTNode {};

struct BinaryExpr : public Expression {
    std::unique_ptr<Expression> left;
    Token op;
    std::unique_ptr<Expression> right;
    
    BinaryExpr(std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    void accept(ASTVisitor& visitor) override;
};

struct UnaryExpr : public Expression {
    Token op;
    std::unique_ptr<Expression> right;

    UnaryExpr(Token op, std::unique_ptr<Expression> right)
        : op(std::move(op)), right(std::move(right)) {}
    void accept(ASTVisitor& visitor) override;
};

struct LogicalExpr : public Expression {
    std::unique_ptr<Expression> left;
    Token op;
    std::unique_ptr<Expression> right;

    LogicalExpr(std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    void accept(ASTVisitor& visitor) override;
};

struct LiteralExpr : public Expression {
    Token value;

    explicit LiteralExpr(Token value) : value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override;
};

struct VariableExpr : public Expression {
    Token name;

    explicit VariableExpr(Token name) : name(std::move(name)) {}
    void accept(ASTVisitor& visitor) override;
};

struct InputExpr : public Expression {
    InputExpr() = default;
    void accept(ASTVisitor& visitor) override;
};

struct AssignExpr : public Expression {
    Token name;
    std::unique_ptr<Expression> value;

    AssignExpr(Token name, std::unique_ptr<Expression> value)
        : name(std::move(name)), value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override;
};

struct BlockStmt : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;

    explicit BlockStmt(std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements)) {}
    void accept(ASTVisitor& visitor) override;
};

struct ExpressionStmt : public Statement {
    std::unique_ptr<Expression> expr;

    explicit ExpressionStmt(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
    void accept(ASTVisitor& visitor) override;
};

struct LetStmt : public Statement {
    Token name;
    std::unique_ptr<Expression> initializer;

    LetStmt(Token name, std::unique_ptr<Expression> initializer)
        : name(std::move(name)), initializer(std::move(initializer)) {}
    void accept(ASTVisitor& visitor) override;
};

struct IfStmt : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;

    IfStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenBranch, std::unique_ptr<Statement> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
    void accept(ASTVisitor& visitor) override;
};

struct WhileStmt : public Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileStmt(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override;
};

struct PrintStmt : public Statement {
    std::unique_ptr<Expression> expr;

    explicit PrintStmt(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}
    void accept(ASTVisitor& visitor) override;
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual void visitLiteralExpr(LiteralExpr& expr) = 0;
    virtual void visitLogicalExpr(LogicalExpr& expr) = 0;
    virtual void visitVariableExpr(VariableExpr& expr) = 0;
    virtual void visitInputExpr(InputExpr& expr) = 0;
    virtual void visitAssignExpr(AssignExpr& expr) = 0;

    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitExpressionStmt(ExpressionStmt& stmt) = 0;
    virtual void visitLetStmt(LetStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitWhileStmt(WhileStmt& stmt) = 0;
    virtual void visitPrintStmt(PrintStmt& stmt) = 0;
};
