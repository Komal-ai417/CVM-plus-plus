#pragma once
#include <string>
#include <cstdint>

enum class TokenType {
    // Keywords
    LET, IF, ELSE, WHILE, PRINT, INPUT, TRUE_LIT, FALSE_LIT,

    // Operators
    PLUS, MINUS, STAR, SLASH, MOD,
    EQUAL, EQ_EQ, BANG_EQ, LESS, GREATER, LESS_EQ, GREATER_EQ,
    BIT_XOR, BIT_AND, BIT_OR, SHL, SHR, BIT_NOT, NOT,
    AND_AND, OR_OR,

    // Symbols
    LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON,

    // Literals & Identifiers
    NUMBER, IDENTIFIER,

    // End of file
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};
