#include "ast.h"

void BinaryExpr::accept(ASTVisitor& visitor) { visitor.visitBinaryExpr(*this); }
void UnaryExpr::accept(ASTVisitor& visitor) { visitor.visitUnaryExpr(*this); }
void LogicalExpr::accept(ASTVisitor& visitor) { visitor.visitLogicalExpr(*this); }
void LiteralExpr::accept(ASTVisitor& visitor) { visitor.visitLiteralExpr(*this); }
void VariableExpr::accept(ASTVisitor& visitor) { visitor.visitVariableExpr(*this); }
void InputExpr::accept(ASTVisitor& visitor) { visitor.visitInputExpr(*this); }
void AssignExpr::accept(ASTVisitor& visitor) { visitor.visitAssignExpr(*this); }

void BlockStmt::accept(ASTVisitor& visitor) { visitor.visitBlockStmt(*this); }
void ExpressionStmt::accept(ASTVisitor& visitor) { visitor.visitExpressionStmt(*this); }
void LetStmt::accept(ASTVisitor& visitor) { visitor.visitLetStmt(*this); }
void IfStmt::accept(ASTVisitor& visitor) { visitor.visitIfStmt(*this); }
void WhileStmt::accept(ASTVisitor& visitor) { visitor.visitWhileStmt(*this); }
void PrintStmt::accept(ASTVisitor& visitor) { visitor.visitPrintStmt(*this); }
