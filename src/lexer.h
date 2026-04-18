#pragma once
#include "token.h"
#include <vector>
#include <string>

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    const std::string& source;
    size_t pos;
    int line;
    int column;

    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;

    void skipWhitespace();
    Token createToken(TokenType type, const std::string& value);
};
