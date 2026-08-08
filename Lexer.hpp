#pragma once
#include <cstring>
#include <cctype>

enum TokenType {
    // single-character
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_SEMICOLON,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,

    // one or two character
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    TOKEN_AND, TOKEN_OR, // && ||

    // literals
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // keywords
    TOKEN_VAR, TOKEN_LET, TOKEN_CONST,
    TOKEN_FUNCTION, TOKEN_RETURN,
    TOKEN_IF, TOKEN_ELSE,
    TOKEN_WHILE, TOKEN_FOR,
    TOKEN_TRUE, TOKEN_FALSE, TOKEN_NULL,

    TOKEN_EOF,
    TOKEN_ERROR
};

struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;
};

class Lexer {
private:
    const char* start;
    const char* current;
    int line = 1;

    bool isAtEnd() const { return *current == '\0'; }

    char advance() {
        current++;
        return current[-1];
    }

    char peek() const { return *current; }

    char peekNext() const {
        if (isAtEnd()) return '\0';
        return current[1];
    }

    bool match(char expected) {
        if (isAtEnd() || *current != expected) return false;
        current++;
        return true;
    }

    Token makeToken(TokenType type) const {
        return { type, start, static_cast<int>(current - start), line };
    }

    Token errorToken(const char* message) const {
        return { TOKEN_ERROR, message, static_cast<int>(std::strlen(message)), line };
    }

    void skipWhitespaceAndComments() {
        for (;;) {
            char c = peek();
            switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    // line comment
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                else if (peekNext() == '*') {
                    // block comment
                    advance(); advance();
                    while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
                        if (peek() == '\n') line++;
                        advance();
                    }
                    if (!isAtEnd()) { advance(); advance(); } // consume */
                }
                else {
                    return;
                }
                break;
            default:
                return;
            }
        }
    }

    static bool isAlpha(char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_' || c == '$';
    }

    static bool isDigit(char c) {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    TokenType identifierType() const {
        // Simple keyword table; switch on first char to keep it cheap.
        struct Keyword { const char* text; TokenType type; };
        static const Keyword keywords[] = {
            {"var", TOKEN_VAR}, {"let", TOKEN_LET}, {"const", TOKEN_CONST},
            {"function", TOKEN_FUNCTION}, {"return", TOKEN_RETURN},
            {"if", TOKEN_IF}, {"else", TOKEN_ELSE},
            {"while", TOKEN_WHILE}, {"for", TOKEN_FOR},
            {"true", TOKEN_TRUE}, {"false", TOKEN_FALSE}, {"null", TOKEN_NULL},
        };

        int len = static_cast<int>(current - start);
        for (const auto& kw : keywords) {
            if (static_cast<int>(std::strlen(kw.text)) == len &&
                std::memcmp(start, kw.text, len) == 0) {
                return kw.type;
            }
        }
        return TOKEN_IDENTIFIER;
    }

    Token identifier() {
        while (isAlpha(peek()) || isDigit(peek())) advance();
        return makeToken(identifierType());
    }

    Token number() {
        while (isDigit(peek())) advance();

        // fractional part
        if (peek() == '.' && isDigit(peekNext())) {
            advance(); // consume '.'
            while (isDigit(peek())) advance();
        }

        // exponent part: 1e10, 1.5e-3
        if (peek() == 'e' || peek() == 'E') {
            char next = peekNext();
            if (isDigit(next) || ((next == '+' || next == '-') && isDigit(current[2]))) {
                advance(); // consume 'e'/'E'
                if (peek() == '+' || peek() == '-') advance();
                while (isDigit(peek())) advance();
            }
        }

        return makeToken(TOKEN_NUMBER);
    }

    Token string() {
        char quote = start[0]; // ' or "
        while (peek() != quote && !isAtEnd()) {
            if (peek() == '\n') line++;
            if (peek() == '\\' && !isAtEnd()) advance(); // skip escaped char
            advance();
        }

        if (isAtEnd()) return errorToken("Unterminated string.");

        advance(); // closing quote
        return makeToken(TOKEN_STRING);
    }

public:
    Lexer(const char* source) : start(source), current(source) {}

    Token nextToken() {
        skipWhitespaceAndComments();
        start = current;

        if (isAtEnd()) return makeToken(TOKEN_EOF);

        char c = advance();

        if (isAlpha(c)) return identifier();
        if (isDigit(c)) return number();

        switch (c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case '+': return makeToken(TOKEN_PLUS);
        case '-': return makeToken(TOKEN_MINUS);
        case '*': return makeToken(TOKEN_STAR);
        case '/': return makeToken(TOKEN_SLASH);

        case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        case '&':
            if (match('&')) return makeToken(TOKEN_AND);
            return errorToken("Unexpected character '&'.");
        case '|':
            if (match('|')) return makeToken(TOKEN_OR);
            return errorToken("Unexpected character '|'.");

        case '"':
        case '\'':
            return string();
        }

        return errorToken("Unexpected character.");
    }
};
