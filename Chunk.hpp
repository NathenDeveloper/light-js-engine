#pragma once
#include <vector>
#include <cstdint>
#include <cstdio>
#include "Value.hpp"

enum OpCode : uint8_t {
    OP_CONSTANT,

    OP_NULL,
    OP_TRUE,
    OP_FALSE,

    OP_POP,

    OP_GET_GLOBAL,
    // OP_DEFINE_GLOBAL intentionally not added yet — that's for `var x = ...;`
    // script-level declarations, which need statement parsing we haven't
    // built. Globals right now are only the natives the VM pre-registers.

    OP_CALL,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,

    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,

    OP_RETURN
};

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<int> lines;
    std::vector<Value> constants;

    void write(uint8_t byte, int line) {
        code.push_back(byte);
        lines.push_back(line);
    }

    int addConstant(Value value) {
        constants.push_back(value);
        return static_cast<int>(constants.size() - 1);
    }

    int getLine(int offset) const {
        if (offset < 0 || offset >= static_cast<int>(lines.size())) return -1;
        return lines[offset];
    }

    void disassemble(const char* name) const {
        std::printf("== %s ==\n", name);
        for (size_t offset = 0; offset < code.size(); ) {
            offset = disassembleInstruction(static_cast<int>(offset));
        }
    }

    int disassembleInstruction(int offset) const {
        std::printf("%04d ", offset);
        if (offset > 0 && lines[offset] == lines[offset - 1]) {
            std::printf("   | ");
        }
        else {
            std::printf("%4d ", lines[offset]);
        }

        uint8_t instruction = code[offset];
        switch (instruction) {
        case OP_CONSTANT:    return constantInstruction("OP_CONSTANT", offset);
        case OP_NULL:        return simpleInstruction("OP_NULL", offset);
        case OP_TRUE:        return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:       return simpleInstruction("OP_FALSE", offset);
        case OP_POP:         return simpleInstruction("OP_POP", offset);
        case OP_GET_GLOBAL:  return constantInstruction("OP_GET_GLOBAL", offset);
        case OP_CALL:        return byteInstruction("OP_CALL", offset);
        case OP_ADD:         return simpleInstruction("OP_ADD", offset);
        case OP_SUBTRACT:    return simpleInstruction("OP_SUBTRACT", offset);
        case OP_MULTIPLY:    return simpleInstruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:      return simpleInstruction("OP_DIVIDE", offset);
        case OP_NEGATE:      return simpleInstruction("OP_NEGATE", offset);
        case OP_NOT:         return simpleInstruction("OP_NOT", offset);
        case OP_EQUAL:       return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER:     return simpleInstruction("OP_GREATER", offset);
        case OP_LESS:        return simpleInstruction("OP_LESS", offset);
        case OP_RETURN:      return simpleInstruction("OP_RETURN", offset);
        default:
            std::printf("Unknown opcode %d\n", instruction);
            return offset + 1;
        }
    }

private:
    int simpleInstruction(const char* name, int offset) const {
        std::printf("%s\n", name);
        return offset + 1;
    }

    int constantInstruction(const char* name, int offset) const {
        uint8_t constantIndex = code[offset + 1];
        std::printf("%-16s %4d '", name, constantIndex);
        printValue(constants[constantIndex]);
        std::printf("'\n");
        return offset + 2;
    }

    int byteInstruction(const char* name, int offset) const {
        uint8_t value = code[offset + 1];
        std::printf("%-16s %4d\n", name, value);
        return offset + 2;
    }
};
