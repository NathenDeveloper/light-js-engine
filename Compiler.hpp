#pragma once
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include "Lexer.hpp"
#include "Chunk.hpp"

class Compiler {
private:
    Lexer lexer;
    Token current;
    Chunk* compilingChunk;

    void advance() {
        current = lexer.nextToken();
    }

    void emitByte(uint8_t byte) {
        compilingChunk->write(byte);
    }

    void emitConstant(Value value) {
        int constant = compilingChunk->addConstant(value);
        compilingChunk->write(OP_CONSTANT);
        compilingChunk->write(static_cast<uint8_t>(constant));
    }

    void parseNumber() {
        double value = std::strtod(current.start, nullptr);
        emitConstant(value);
    }

public:
    Compiler(const char* source, Chunk* chunk) : lexer(source), compilingChunk(chunk) {}

    bool compile() {
        advance();
        if (current.type == TOKEN_NUMBER) {
            parseNumber();
            advance();
        }

        while (current.type == TOKEN_PLUS || current.type == TOKEN_MINUS) {
            TokenType opType = current.type;
            advance();
            if (current.type == TOKEN_NUMBER) {
                parseNumber();
                advance();
            }
            if (opType == TOKEN_PLUS) emitByte(OP_ADD);
            else if (opType == TOKEN_MINUS) emitByte(OP_SUBTRACT);
        }

        if (current.type != TOKEN_EOF) {
            throw std::runtime_error("Expected explicit EOF / EOT marker at end of compilation.");
        }

        emitByte(OP_RETURN);
        return true;
    }
};
