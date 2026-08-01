#pragma once
#include <vector>
#include <cstdint>
#include "Value.hpp"

enum OpCode {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_RETURN
};

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;

    void write(uint8_t byte) {
        code.push_back(byte);
    }

    int addConstant(Value value) {
        constants.push_back(value);
        return static_cast<int>(constants.size() - 1);
    }
};
