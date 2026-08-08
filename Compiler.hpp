#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include "Lexer.hpp"
#include "Chunk.hpp"
#include "VM.hpp"

class Compiler {
private:
    enum Precedence {
        PREC_NONE,
        PREC_ASSIGNMENT,
        PREC_OR,
        PREC_AND,
        PREC_EQUALITY,
        PREC_COMPARISON,
        PREC_TERM,
        PREC_FACTOR,
        PREC_UNARY,
        PREC_CALL,
        PREC_PRIMARY
    };

    using ParseFn = void (Compiler::*)(bool canAssign);

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    Lexer lexer;
    Token previous;
    Token current;
    Chunk* compilingChunk;
    VM& vm; // needed to allocate identifier-name strings (GC-tracked)
    bool hadError = false;
    bool panicMode = false;

    void advance() {
        previous = current;
        for (;;) {
            current = lexer.nextToken();
            if (current.type != TOKEN_ERROR) break;
            errorAtCurrent(current.start);
        }
    }

    void consume(TokenType type, const char* message) {
        if (current.type == type) { advance(); return; }
        errorAtCurrent(message);
    }

    bool check(TokenType type) const { return current.type == type; }

    bool match(TokenType type) {
        if (!check(type)) return false;
        advance();
        return true;
    }

    void errorAt(const Token& token, const char* message) {
        if (panicMode) return;
        panicMode = true;
        hadError = true;

        std::string msg = "[line " + std::to_string(token.line) + "] Error";
        if (token.type == TOKEN_EOF) msg += " at end";
        else if (token.type != TOKEN_ERROR) msg += " at '" + std::string(token.start, token.length) + "'";
        msg += ": ";
        msg += message;
        fprintf(stderr, "%s\n", msg.c_str());
    }

    void errorAtCurrent(const char* message) { errorAt(current, message); }
    void error(const char* message) { errorAt(previous, message); }

    void synchronize() {
        panicMode = false;
        while (current.type != TOKEN_EOF) {
            if (previous.type == TOKEN_SEMICOLON) return;
            switch (current.type) {
            case TOKEN_VAR: case TOKEN_LET: case TOKEN_CONST:
            case TOKEN_FUNCTION: case TOKEN_IF: case TOKEN_WHILE:
            case TOKEN_FOR: case TOKEN_RETURN:
                return;
            default: break;
            }
            advance();
        }
    }

    void emitByte(uint8_t byte) { compilingChunk->write(byte, previous.line); }
    void emitBytes(uint8_t a, uint8_t b) { emitByte(a); emitByte(b); }

    void emitConstant(Value value) {
        int constant = compilingChunk->addConstant(value);
        if (constant > UINT8_MAX) {
            error("Too many constants in one chunk.");
            constant = 0;
        }
        emitBytes(OP_CONSTANT, static_cast<uint8_t>(constant));
    }

    uint8_t identifierConstant(const Token& name) {
        std::string text(name.start, name.length);
        StringObject* obj = vm.allocateString(text);
        int constant = compilingChunk->addConstant(Value(obj));
        if (constant > UINT8_MAX) {
            error("Too many constants in one chunk.");
            return 0;
        }
        return static_cast<uint8_t>(constant);
    }

    const ParseRule& getRule(TokenType type) const {
        static const ParseRule rules[] = {
            /* TOKEN_LEFT_PAREN    */ { &Compiler::grouping, &Compiler::call, PREC_CALL },
            /* TOKEN_RIGHT_PAREN   */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_LEFT_BRACE    */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_RIGHT_BRACE   */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_COMMA         */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_DOT           */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_SEMICOLON     */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_PLUS          */ { nullptr,              &Compiler::binary, PREC_TERM },
            /* TOKEN_MINUS         */ { &Compiler::unary,     &Compiler::binary, PREC_TERM },
            /* TOKEN_STAR          */ { nullptr,              &Compiler::binary, PREC_FACTOR },
            /* TOKEN_SLASH         */ { nullptr,              &Compiler::binary, PREC_FACTOR },
            /* TOKEN_BANG          */ { &Compiler::unary,     nullptr,        PREC_NONE },
            /* TOKEN_BANG_EQUAL    */ { nullptr,              &Compiler::binary, PREC_EQUALITY },
            /* TOKEN_EQUAL         */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_EQUAL_EQUAL   */ { nullptr,              &Compiler::binary, PREC_EQUALITY },
            /* TOKEN_GREATER       */ { nullptr,              &Compiler::binary, PREC_COMPARISON },
            /* TOKEN_GREATER_EQUAL */ { nullptr,              &Compiler::binary, PREC_COMPARISON },
            /* TOKEN_LESS          */ { nullptr,              &Compiler::binary, PREC_COMPARISON },
            /* TOKEN_LESS_EQUAL    */ { nullptr,              &Compiler::binary, PREC_COMPARISON },
            /* TOKEN_AND           */ { nullptr,              nullptr,        PREC_NONE }, // wire up when adding jumps
            /* TOKEN_OR            */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_IDENTIFIER    */ { &Compiler::variable,  nullptr,        PREC_NONE },
            /* TOKEN_STRING        */ { &Compiler::string,    nullptr,        PREC_NONE },
            /* TOKEN_NUMBER        */ { &Compiler::number,    nullptr,        PREC_NONE },
            /* TOKEN_VAR           */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_LET           */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_CONST         */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_FUNCTION      */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_RETURN        */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_IF            */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_ELSE          */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_WHILE         */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_FOR           */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_TRUE          */ { &Compiler::literal,   nullptr,        PREC_NONE },
            /* TOKEN_FALSE         */ { &Compiler::literal,   nullptr,        PREC_NONE },
            /* TOKEN_NULL          */ { &Compiler::literal,   nullptr,        PREC_NONE },
            /* TOKEN_EOF           */ { nullptr,              nullptr,        PREC_NONE },
            /* TOKEN_ERROR         */ { nullptr,              nullptr,        PREC_NONE },
        };
        // NOTE: this array's order must exactly match the TokenType enum
        // in Lexer.hpp. That positional coupling is fragile — if you add
        // a token type, add its row here in the same position, or switch
        // this to a designated-initializer / std::unordered_map lookup
        // keyed on TokenType to remove the ordering hazard entirely.
        return rules[type];
    }

    void parsePrecedence(Precedence precedence) {
        advance();
        ParseFn prefixRule = getRule(previous.type).prefix;
        if (prefixRule == nullptr) {
            error("Expected expression.");
            return;
        }

        bool canAssign = precedence <= PREC_ASSIGNMENT;
        (this->*prefixRule)(canAssign);

        while (precedence <= getRule(current.type).precedence) {
            advance();
            ParseFn infixRule = getRule(previous.type).infix;
            (this->*infixRule)(canAssign);
        }
    }

    void expression() { parsePrecedence(PREC_ASSIGNMENT); }

    // --- statements ------------------------------------------------------

    void expressionStatement() {
        expression();
        match(TOKEN_SEMICOLON); // tolerant: consume if present, don't require it
        emitByte(OP_POP);       // statements discard their expression's value
    }

    void statement() {
        // Only expression statements exist until if/while/var/function
        // parsing is added; everything else falls through here for now.
        expressionStatement();
    }

    // --- grammar rule bodies ----------------------------------------

    void number(bool) {
        char* end = nullptr;
        double value = std::strtod(previous.start, &end);
        if (end == previous.start) { error("Invalid number literal."); return; }
        emitConstant(Value(value));
    }

    void string(bool) {
        // previous.start/length include the surrounding quotes; strip them.
        std::string text(previous.start + 1, previous.length - 2);
        StringObject* obj = vm.allocateString(text);
        emitConstant(Value(obj));
    }

    void variable(bool /*canAssign*/) {
        // Assignment (x = ...) intentionally not wired yet — that's
        // where canAssign starts mattering, alongside OP_SET_GLOBAL.
        emitBytes(OP_GET_GLOBAL, identifierConstant(previous));
    }

    void grouping(bool) {
        expression();
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
    }

    void unary(bool) {
        TokenType opType = previous.type;
        parsePrecedence(PREC_UNARY);
        switch (opType) {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        case TOKEN_BANG:  emitByte(OP_NOT);    break;
        default: return;
        }
    }

    void binary(bool) {
        TokenType opType = previous.type;
        const ParseRule& rule = getRule(opType);
        parsePrecedence(static_cast<Precedence>(rule.precedence + 1));

        switch (opType) {
        case TOKEN_PLUS:          emitByte(OP_ADD);      break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE);   break;
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL);    break;
        case TOKEN_GREATER:       emitByte(OP_GREATER);  break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS);     break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        default: return;
        }
    }

    void literal(bool) {
        switch (previous.type) {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_TRUE:  emitByte(OP_TRUE);  break;
        case TOKEN_NULL:  emitByte(OP_NULL);  break;
        default: return;
        }
    }

    uint8_t argumentList() {
        uint8_t argCount = 0;
        if (!check(TOKEN_RIGHT_PAREN)) {
            do {
                expression();
                if (argCount == 255) {
                    error("Can't have more than 255 arguments.");
                }
                else {
                    argCount++;
                }
            } while (match(TOKEN_COMMA));
        }
        consume(TOKEN_RIGHT_PAREN, "Expected ')' after arguments.");
        return argCount;
    }

    void call(bool) {
        uint8_t argCount = argumentList();
        emitBytes(OP_CALL, argCount);
    }

public:
    Compiler(const char* source, Chunk* chunk, VM& vmRef)
        : lexer(source), compilingChunk(chunk), vm(vmRef) {
    }

    bool compile() {
        advance();
        while (!match(TOKEN_EOF)) {
            statement();
            if (panicMode) synchronize();
        }
        emitByte(OP_RETURN);
        return !hadError;
    }
};
