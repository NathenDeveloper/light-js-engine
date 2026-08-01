#pragma once
#include <string>
#include <cctype>

enum TokenType {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_EOF,
    TOKEN_ERROR
};

struct Token {
    TokenType type;
    const char* start;
    int length;
};

class Lexer {
private:
    const char* start;
    const char* current;

public:
    Lexer(const char* source) : start(source), current(source) {}

    Token nextToken() {
        while (*current != '\0' && std::isspace(*current)) {
            current++;
        }

        start = current;

        if (*current == '\0') return {TOKEN_EOF, start, 0};
        if (*current == '+') { current++; return {TOKEN_PLUS, start, 1}; }
        if (*current == '-') { current++; return {TOKEN_MINUS, start, 1}; }

        if (std::isdigit(*current)) {
            while (std::isdigit(*current)) current++;
            return {TOKEN_NUMBER, start, static_cast<int>(current - start)};
        }

        return {TOKEN_ERROR, start, 1};
    }
};
