#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>
#include "Chunk.hpp"
#include "Value.hpp"
#include "NativeBindings.hpp"

enum class InterpretResult {
    Ok,
    CompileError,
    RuntimeError
};

class VM {
private:
    Chunk* chunk;
    uint8_t* ip;

    static constexpr int kStackMax = 256;
    Value stack[kStackMax];
    Value* stackTop;

    Object* objects;
    NativeBindings::Globals globals;

    int instructionCount;
    const int maxInstructions = 10000;

    int callDepth = 0;
    const int maxCallDepth = 64; // matters once user-defined functions can recurse

    void resetStack() { stackTop = stack; }

    void push(Value value) {
        if (stackTop - stack >= kStackMax) {
            runtimeError("Stack overflow.");
            return;
        }
        *stackTop = value;
        stackTop++;
    }

    Value pop() {
        stackTop--;
        return *stackTop;
    }

    Value peek(int distanceFromTop) const {
        return stackTop[-1 - distanceFromTop];
    }

    int currentOffset() const {
        return static_cast<int>(ip - chunk->code.data()) - 1;
    }

    void runtimeError(const char* message) {
        std::cerr << "[line " << chunk->getLine(currentOffset())
            << "] Runtime error: " << message << "\n";
        resetStack();
    }

    bool binaryNumericOp(char op) {
        if (!peek(0).isNumber() || !peek(1).isNumber()) {
            runtimeError("Operands must be numbers.");
            return false;
        }
        double b = pop().asNumber();
        double a = pop().asNumber();
        switch (op) {
        case '+': push(Value(a + b)); break;
        case '-': push(Value(a - b)); break;
        case '*': push(Value(a * b)); break;
        case '/': push(Value(a / b)); break; // IEEE: a/0 -> inf/-inf/NaN, matches JS
        case '>': push(Value(a > b)); break;
        case '<': push(Value(a < b)); break;
        }
        return true;
    }

    static std::string stringify(const Value& v) {
        if (v.isString()) return v.asString();
        switch (v.type) {
        case ValueType::Undefined: return "undefined";
        case ValueType::Null:      return "null";
        case ValueType::Boolean:   return v.asBool() ? "true" : "false";
        case ValueType::Number: {
            double n = v.asNumber();
            if (n == static_cast<long long>(n)) return std::to_string(static_cast<long long>(n));
            return std::to_string(n);
        }
        default: return "<object>";
        }
    }

    // Only natives exist right now, so this is a thin dispatcher — but
    // it's the seam where user-defined function calls plug in later
    // without OP_CALL's handling in the main loop needing to change.
    bool callValue(Value callee, int argCount) {
        if (!callee.isNative()) {
            runtimeError("Can only call functions.");
            return false;
        }

        NativeObject* native = callee.asNative();
        if (native->arity >= 0 && native->arity != argCount) {
            runtimeError("Wrong number of arguments for native function.");
            return false;
        }

        if (++callDepth > maxCallDepth) {
            --callDepth;
            runtimeError("Call depth exceeded.");
            return false;
        }

        Value result = native->function(argCount, stackTop - argCount);
        --callDepth;

        stackTop -= argCount + 1; // pop args and the callee itself
        push(result);
        return true;
    }

public:
    VM() : chunk(nullptr), ip(nullptr), stackTop(stack), objects(nullptr), instructionCount(0) {
        NativeBindings::registerCoreBindings(globals, objects);
    }

    ~VM() {
        freeObjects();
    }

    StringObject* allocateString(std::string chars) {
        StringObject* obj = new StringObject(std::move(chars));
        obj->next = objects;
        objects = obj;
        return obj;
    }

    void markValue(const Value& value) {
        if (value.isObject()) value.asObject()->marked = true;
    }

    void markRoots() {
        for (Value* slot = stack; slot < stackTop; slot++) {
            markValue(*slot);
        }
        // Natives referenced from `globals` are intentionally not
        // marked/swept: they're allocated once at VM construction and
        // live for the VM's whole lifetime, same as the globals map
        // itself. Only heap objects a script creates at runtime
        // (currently: strings) go through mark-sweep.
    }

    void sweep() {
        Object* previous = nullptr;
        Object* current = objects;
        while (current != nullptr) {
            if (current->marked || current->type == ObjectType::Native) {
                current->marked = false;
                previous = current;
                current = current->next;
            }
            else {
                Object* unreached = current;
                current = current->next;
                if (previous == nullptr) objects = current;
                else previous->next = current;
                delete unreached;
            }
        }
    }

    void collectGarbage() {
        markRoots();
        sweep();
    }

    InterpretResult interpret(Chunk* c) {
        chunk = c;
        ip = chunk->code.data();
        resetStack();
        instructionCount = 0;
        callDepth = 0;
        NativeBindings::resetOutputBudget();

        for (;;) {
            if (++instructionCount > maxInstructions) {
                std::cerr << "[Sandbox Error] Execution quota exceeded (Infinite loop guard triggered).\n";
                return InterpretResult::RuntimeError;
            }

            uint8_t instruction = *ip++;
            switch (instruction) {
            case OP_CONSTANT: {
                Value constant = chunk->constants[*ip++];
                push(constant);
                break;
            }
            case OP_NULL:  push(Value::null()); break;
            case OP_TRUE:  push(Value(true));   break;
            case OP_FALSE: push(Value(false));  break;

            case OP_POP: pop(); break;

            case OP_GET_GLOBAL: {
                uint8_t nameIdx = *ip++;
                const std::string& name = chunk->constants[nameIdx].asString();
                auto it = globals.find(name);
                if (it == globals.end()) {
                    runtimeError(("Undefined variable '" + name + "'.").c_str());
                    return InterpretResult::RuntimeError;
                }
                push(it->second);
                break;
            }

            case OP_CALL: {
                uint8_t argCount = *ip++;
                if (!callValue(peek(argCount), argCount)) {
                    return InterpretResult::RuntimeError;
                }
                break;
            }

            case OP_ADD: {
                if (peek(0).isString() || peek(1).isString()) {
                    Value b = pop();
                    Value a = pop();
                    std::string result = stringify(a) + stringify(b);
                    push(Value(allocateString(result)));
                }
                else if (!binaryNumericOp('+')) {
                    return InterpretResult::RuntimeError;
                }
                break;
            }
            case OP_SUBTRACT:
                if (!binaryNumericOp('-')) return InterpretResult::RuntimeError;
                break;
            case OP_MULTIPLY:
                if (!binaryNumericOp('*')) return InterpretResult::RuntimeError;
                break;
            case OP_DIVIDE:
                if (!binaryNumericOp('/')) return InterpretResult::RuntimeError;
                break;

            case OP_NEGATE: {
                if (!peek(0).isNumber()) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RuntimeError;
                }
                push(Value(-pop().asNumber()));
                break;
            }
            case OP_NOT:
                push(Value(!pop().isTruthy()));
                break;

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value(a.strictEquals(b)));
                break;
            }
            case OP_GREATER:
                if (!binaryNumericOp('>')) return InterpretResult::RuntimeError;
                break;
            case OP_LESS:
                if (!binaryNumericOp('<')) return InterpretResult::RuntimeError;
                break;

            case OP_RETURN:
                collectGarbage();
                return InterpretResult::Ok;

            default:
                runtimeError("Unknown opcode.");
                return InterpretResult::RuntimeError;
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
        objects = nullptr;
    }
};
