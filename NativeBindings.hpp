#pragma once
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>
#include "Value.hpp"

// Sandboxing contract for everything in this file:
//   - No filesystem, network, process, environment, or clock access.
//   - No pointers/handles leak out to script-visible Values.
//   - No OS entropy (no std::random_device, no time-based seeding) —
//     that's a covert channel and a source of non-replayable behavior.
//   - Every function is pure w.r.t. the outside world except the one
//     sanctioned exception, `print`, which writes to stdout and is
//     itself budget-capped below.
//   - Never throw C++ exceptions across this boundary — a native runs
//     inside the VM's opcode loop; an uncaught throw skips its cleanup.
namespace NativeBindings {

    using Globals = std::unordered_map<std::string, Value>;

    inline Value nativeError(const char* fnName, const char* message) {
        std::cerr << "[native] " << fnName << ": " << message << "\n";
        return Value(); // undefined
    }

    inline bool checkArity(const char* name, int expected, int got) {
        if (expected >= 0 && got != expected) {
            nativeError(name, "wrong number of arguments");
            return false;
        }
        return true;
    }

    // --- output budget, backing native_print --------------------------
    // Caps total bytes printed per interpret() run, independent of the
    // instruction quota. A tight `while(true) print("x")` loop still
    // burns instructions (and gets stopped by that quota eventually),
    // but this stops the output itself from being unbounded in the
    // meantime.

    constexpr size_t kMaxOutputBytes = 1 << 16; // 64 KB per run

    inline size_t& outputBudgetUsed() {
        static size_t used = 0;
        return used;
    }

    inline bool& outputBudgetWarned() {
        static bool warned = false;
        return warned;
    }

    inline void resetOutputBudget() {
        outputBudgetUsed() = 0;
        outputBudgetWarned() = false;
    }

    // --- console.log-equivalent -----------------------------------------

    inline Value native_print(int argCount, Value* args) {
        if (outputBudgetUsed() >= kMaxOutputBytes) {
            if (!outputBudgetWarned()) {
                std::cerr << "[Sandbox Error] print() output budget exceeded.\n";
                outputBudgetWarned() = true;
            }
            return Value();
        }

        std::string line;
        for (int i = 0; i < argCount; i++) {
            if (i > 0) line += ' ';
            if (args[i].isString()) {
                line += args[i].asString();
            }
            else {
                // reuse the same textual form printValue would emit
                switch (args[i].type) {
                case ValueType::Undefined: line += "undefined"; break;
                case ValueType::Null:      line += "null"; break;
                case ValueType::Boolean:   line += args[i].asBool() ? "true" : "false"; break;
                case ValueType::Number: {
                    double n = args[i].asNumber();
                    line += (n == static_cast<long long>(n))
                        ? std::to_string(static_cast<long long>(n))
                        : std::to_string(n);
                    break;
                }
                default: line += "<object>"; break;
                }
            }
        }
        line += '\n';

        outputBudgetUsed() += line.size();
        std::cout << line;
        return Value(); // undefined, matches console.log's return
    }

    // --- Math.* --------------------------------------------------------

    inline Value native_sqrt(int argCount, Value* args) {
        if (!checkArity("sqrt", 1, argCount)) return Value();
        if (!args[0].isNumber()) return nativeError("sqrt", "argument must be a number");
        return Value(std::sqrt(args[0].asNumber()));
    }

    inline Value native_abs(int argCount, Value* args) {
        if (!checkArity("abs", 1, argCount)) return Value();
        if (!args[0].isNumber()) return nativeError("abs", "argument must be a number");
        return Value(std::fabs(args[0].asNumber()));
    }

    inline Value native_pow(int argCount, Value* args) {
        if (!checkArity("pow", 2, argCount)) return Value();
        if (!args[0].isNumber() || !args[1].isNumber())
            return nativeError("pow", "arguments must be numbers");
        return Value(std::pow(args[0].asNumber(), args[1].asNumber()));
    }

    inline Value native_floor(int argCount, Value* args) {
        if (!checkArity("floor", 1, argCount)) return Value();
        if (!args[0].isNumber()) return nativeError("floor", "argument must be a number");
        return Value(std::floor(args[0].asNumber()));
    }

    inline Value native_ceil(int argCount, Value* args) {
        if (!checkArity("ceil", 1, argCount)) return Value();
        if (!args[0].isNumber()) return nativeError("ceil", "argument must be a number");
        return Value(std::ceil(args[0].asNumber()));
    }

    inline Value native_max(int argCount, Value* args) {
        if (argCount == 0) return Value(-INFINITY);
        double best = -INFINITY;
        for (int i = 0; i < argCount; i++) {
            if (!args[i].isNumber()) return nativeError("max", "arguments must be numbers");
            best = std::max(best, args[i].asNumber());
        }
        return Value(best);
    }

    inline Value native_min(int argCount, Value* args) {
        if (argCount == 0) return Value(INFINITY);
        double best = INFINITY;
        for (int i = 0; i < argCount; i++) {
            if (!args[i].isNumber()) return nativeError("min", "arguments must be numbers");
            best = std::min(best, args[i].asNumber());
        }
        return Value(best);
    }

    // Deliberately not bound: Math.random / any RNG. See file header —
    // OS entropy is a covert channel out of a sandbox. Add only with an
    // explicit, seeded, deterministic PRNG if a script actually needs one.

    // --- registration ---------------------------------------------------

    inline void define(Globals& globals, Object*& objectList,
        const std::string& name, int arity, NativeFn fn) {
        NativeObject* obj = new NativeObject(name, arity, fn);
        obj->next = objectList;
        objectList = obj;
        globals[name] = Value(obj);
    }

    // objectList should be the VM's `objects` head so natives are freed
    // (and GC-tracked) the same way any other heap object is.
    inline void registerCoreBindings(Globals& globals, Object*& objectList) {
        define(globals, objectList, "print", -1, native_print);

        define(globals, objectList, "sqrt", 1, native_sqrt);
        define(globals, objectList, "abs", 1, native_abs);
        define(globals, objectList, "pow", 2, native_pow);
        define(globals, objectList, "floor", 1, native_floor);
        define(globals, objectList, "ceil", 1, native_ceil);
        define(globals, objectList, "max", -1, native_max);
        define(globals, objectList, "min", -1, native_min);
    }
}
