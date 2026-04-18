#include "lexer.h"
#include <cctype>
#include <unordered_map>
#include <stdexcept>
#include <string>

Lexer::Lexer(const std::string& source) : source(source), pos(0), line(1), column(1) {}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[pos];
}

char Lexer::peekNext() const {
    if (pos + 1 >= source.length()) return '\0';
    return source[pos + 1];
}

char Lexer::advance() {
    char c = source[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return pos >= source.length();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance();
        } else if (c == '/' && peekNext() == '/') {
            // Comment
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::createToken(TokenType type, const std::string& value) {
    return {type, value, line, column - static_cast<int>(value.length())};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"let", TokenType::LET},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"print", TokenType::PRINT},
        {"input", TokenType::INPUT},
        {"true", TokenType::TRUE_LIT},
        {"false", TokenType::FALSE_LIT}
    };

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        char c = peek();
        
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string ident;
            ident.reserve(16);
            while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
                ident += advance();
            }
            auto it = keywords.find(ident);
            if (it != keywords.end()) {
                tokens.push_back(createToken(it->second, ident));
            } else {
                tokens.push_back(createToken(TokenType::IDENTIFIER, ident));
            }
            continue;
        }

        if (std::isdigit(c)) {
            std::string num;
            num.reserve(16);
            while (!isAtEnd() && std::isdigit(peek())) {
                num += advance();
            }
            tokens.push_back(createToken(TokenType::NUMBER, num));
            continue;
        }

        char advanced = advance();
        
        switch (advanced) {
            case '+': tokens.push_back(createToken(TokenType::PLUS, "+")); break;
            case '-': tokens.push_back(createToken(TokenType::MINUS, "-")); break;
            case '*': tokens.push_back(createToken(TokenType::STAR, "*")); break;
            case '/': tokens.push_back(createToken(TokenType::SLASH, "/")); break;
            case '%': tokens.push_back(createToken(TokenType::MOD, "%")); break;
            case '(': tokens.push_back(createToken(TokenType::LPAREN, "(")); break;
            case ')': tokens.push_back(createToken(TokenType::RPAREN, ")")); break;
            case '{': tokens.push_back(createToken(TokenType::LBRACE, "{")); break;
            case '}': tokens.push_back(createToken(TokenType::RBRACE, "}")); break;
            case ';': tokens.push_back(createToken(TokenType::SEMICOLON, ";")); break;
            case '^': tokens.push_back(createToken(TokenType::BIT_XOR, "^")); break;
            case '&':
                if (peek() == '&') {
                    advance();
                    tokens.push_back(createToken(TokenType::AND_AND, "&&"));
                } else {
                    tokens.push_back(createToken(TokenType::BIT_AND, "&"));
                }
                break;
            case '|':
                if (peek() == '|') {
                    advance();
                    tokens.push_back(createToken(TokenType::OR_OR, "||"));
                } else {
                    tokens.push_back(createToken(TokenType::BIT_OR, "|"));
                }
                break;
            case '~': tokens.push_back(createToken(TokenType::BIT_NOT, "~")); break;
            case '=':
                if (peek() == '=') {
                    advance();
                    tokens.push_back(createToken(TokenType::EQ_EQ, "=="));
                } else {
                    tokens.push_back(createToken(TokenType::EQUAL, "="));
                }
                break;
            case '<':
                if (peek() == '=') {
                    advance();
                    tokens.push_back(createToken(TokenType::LESS_EQ, "<="));
                } else if (peek() == '<') {
                    advance();
                    tokens.push_back(createToken(TokenType::SHL, "<<"));
                } else {
                    tokens.push_back(createToken(TokenType::LESS, "<"));
                }
                break;
            case '>':
                if (peek() == '=') {
                    advance();
                    tokens.push_back(createToken(TokenType::GREATER_EQ, ">="));
                } else if (peek() == '>') {
                    advance();
                    tokens.push_back(createToken(TokenType::SHR, ">>"));
                } else {
                    tokens.push_back(createToken(TokenType::GREATER, ">"));
                }
                break;
            case '!':
                if (peek() == '=') {
                    advance();
                    tokens.push_back(createToken(TokenType::BANG_EQ, "!="));
                } else {
                    tokens.push_back(createToken(TokenType::NOT, "!"));
                }
                break;
            default: {
                std::string errText(1, advanced);
                throw std::runtime_error("Lexer Error [line " + std::to_string(line) + "]: Unexpected character '" + errText + "'");
            }
        }
    }
    
    tokens.push_back(createToken(TokenType::END_OF_FILE, ""));
    return tokens;
}
