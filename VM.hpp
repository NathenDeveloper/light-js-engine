#pragma once
#include <cstdint>
#include <iostream>
#include "Chunk.hpp"

class VM {
private:
    Chunk* chunk;
    uint8_t* ip;
    Value stack[256];
    Value* stackTop;
    Object* objects;
    int instructionCount;
    const int maxInstructions = 10000; // Sandboxing instruction quota

    void resetStack() {
        stackTop = stack;
    }

    void push(Value value) {
        *stackTop = value;
        stackTop++;
    }

    Value pop() {
        stackTop--;
        return *stackTop;
    }

public:
    VM() : chunk(nullptr), ip(nullptr), stackTop(stack), objects(nullptr), instructionCount(0) {}

    ~VM() {
        freeObjects();
    }

    Object* allocateString(std::string chars) {
        Object* obj = new Object(ValueType::String, chars);
        obj->next = objects;
        objects = obj;
        return obj;
    }

    void collectGarbage() {
        Object* previous = nullptr;
        Object* current = objects;
        while (current != nullptr) {
            if (!current->marked) {
                Object* unreached = current;
                if (previous == nullptr) {
                    objects = current->next;
                } else {
                    previous->next = current->next;
                }
                current = current->next;
                delete unreached;
            } else {
                current->marked = false;
                previous = current;
                current = current->next;
            }
        }
    }

    void interpret(Chunk* c) {
        chunk = c;
        ip = chunk->code.data();

        for (;;) {
            // Sandbox instruction quota check
            if (++instructionCount > maxInstructions) {
                std::cerr << "[Sandbox Error] Execution quota exceeded (Infinite loop guard triggered).\n";
                return;
            }

            uint8_t instruction = *ip++;
            switch (instruction) {
                case OP_CONSTANT: {
                    Value constant = chunk->constants[*ip++];
                    push(constant);
                    break;
                }
                case OP_ADD: {
                    double b = pop().as.number;
                    double a = pop().as.number;
                    push(a + b);
                    break;
                }
                case OP_SUBTRACT: {
                    double b = pop().as.number;
                    double a = pop().as.number;
                    push(a - b);
                    break;
                }
                case OP_RETURN: {
                    if (stackTop > stack) {
                        std::cout << "Result: " << pop().as.number << "\n";
                    }
                    collectGarbage();
                    return;
                }
            }
        }
    }

    void freeObjects() {
        Object* current = objects;
        while (current != nullptr) {
            Object* next = current->next;
            delete current;
            current = next;
        }
    }
};
