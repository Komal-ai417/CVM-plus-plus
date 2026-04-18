#include "parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;
    while (!isAtEnd()) {
        auto decl = declaration();
        if (decl) {
            statements.push_back(std::move(decl));
        }
    }
    return statements;
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    if (current == 0) throw std::runtime_error("Internal: previous() called at start");
    return tokens[current - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error("Parser Error [" + std::to_string(peek().line) + "]: " + message + " (got '" + peek().value + "')");
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::LET:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
                return;
            default:
                break;
        }
        advance();
    }
}

std::unique_ptr<Statement> Parser::declaration() {
    try {
        if (match({TokenType::LET})) return letDeclaration();
        return statement();
    } catch (const std::runtime_error& error) {
        // Simple error reporting and recovery
        std::cerr << error.what() << '\n';
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<Statement> Parser::letDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::unique_ptr<Expression> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<LetStmt>(std::move(name), std::move(initializer));
}

std::unique_ptr<Statement> Parser::statement() {
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::LBRACE})) return blockStatement();
    return expressionStatement();
}

std::unique_ptr<Statement> Parser::ifStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    std::unique_ptr<Expression> condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after if condition.");

    std::unique_ptr<Statement> thenBranch = statement();
    std::unique_ptr<Statement> elseBranch = nullptr;

    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::whileStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'while'.");
    std::unique_ptr<Expression> condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after while condition.");
    std::unique_ptr<Statement> body = statement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::printStatement() {
    std::unique_ptr<Expression> value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Statement> Parser::blockStatement() {
    std::vector<std::unique_ptr<Statement>> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto decl = declaration();
        if (decl) {
            statements.push_back(std::move(decl));
        }
    }
    consume(TokenType::RBRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Statement> Parser::expressionStatement() {
    std::unique_ptr<Expression> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expression> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expression> Parser::assignment() {
    std::unique_ptr<Expression> expr = logicalOr();

    if (match({TokenType::EQUAL})) {
        std::unique_ptr<Expression> value = assignment();

        if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = varExpr->name;
            return std::make_unique<AssignExpr>(std::move(name), std::move(value));
        }

        throw std::runtime_error("Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expression> Parser::logicalOr() {
    std::unique_ptr<Expression> expr = logicalAnd();
    while (match({TokenType::OR_OR})) {
        Token op = previous();
        std::unique_ptr<Expression> right = logicalAnd();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::logicalAnd() {
    std::unique_ptr<Expression> expr = bitwiseOr();
    while (match({TokenType::AND_AND})) {
        Token op = previous();
        std::unique_ptr<Expression> right = bitwiseOr();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::bitwiseOr() {
    std::unique_ptr<Expression> expr = bitwiseXor();
    while (match({TokenType::BIT_OR})) {
        Token op = previous();
        std::unique_ptr<Expression> right = bitwiseXor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::bitwiseXor() {
    std::unique_ptr<Expression> expr = bitwiseAnd();
    while (match({TokenType::BIT_XOR})) {
        Token op = previous();
        std::unique_ptr<Expression> right = bitwiseAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::bitwiseAnd() {
    std::unique_ptr<Expression> expr = equality();
    while (match({TokenType::BIT_AND})) {
        Token op = previous();
        std::unique_ptr<Expression> right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::equality() {
    std::unique_ptr<Expression> expr = comparison();
    while (match({TokenType::EQ_EQ, TokenType::BANG_EQ})) {
        Token op = previous();
        std::unique_ptr<Expression> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::comparison() {
    std::unique_ptr<Expression> expr = shift();
    while (match({TokenType::GREATER, TokenType::GREATER_EQ, TokenType::LESS, TokenType::LESS_EQ})) {
        Token op = previous();
        std::unique_ptr<Expression> right = shift();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::shift() {
    std::unique_ptr<Expression> expr = term();
    while (match({TokenType::SHL, TokenType::SHR})) {
        Token op = previous();
        std::unique_ptr<Expression> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::term() {
    std::unique_ptr<Expression> expr = factor();
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::unique_ptr<Expression> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::factor() {
    std::unique_ptr<Expression> expr = unary();
    while (match({TokenType::SLASH, TokenType::STAR, TokenType::MOD})) {
        Token op = previous();
        std::unique_ptr<Expression> right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::unary() {
    if (match({TokenType::MINUS, TokenType::BIT_NOT, TokenType::NOT})) {
        Token op = previous();
        std::unique_ptr<Expression> right = unary();
        return std::make_unique<UnaryExpr>(std::move(op), std::move(right));
    }
    return primary();
}

std::unique_ptr<Expression> Parser::primary() {
    if (match({TokenType::FALSE_LIT, TokenType::TRUE_LIT})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({TokenType::NUMBER})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }
    if (match({TokenType::INPUT})) {
        return std::make_unique<InputExpr>();
    }
    if (match({TokenType::LPAREN})) {
        std::unique_ptr<Expression> expr = expression();
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        return expr;
    }
    throw std::runtime_error("Parser Error [" + std::to_string(peek().line) + "]: Expect expression. Got '" + peek().value + "'");
}
