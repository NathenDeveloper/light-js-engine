#pragma once
#include <string>
#include <cstdio>
#include <cmath>

enum class ValueType {
    Undefined,
    Null,
    Boolean,
    Number,
    Object      // strings, natives, and (later) functions/arrays
};

enum class ObjectType {
    String,
    Native      // native (C++-backed) function exposed to JS
};

struct Object {
    ObjectType type;
    bool marked;   // GC mark bit
    Object* next;  // intrusive linked list for the GC to walk all objects

    explicit Object(ObjectType t) : type(t), marked(false), next(nullptr) {}
    virtual ~Object() = default;
};

struct StringObject : Object {
    std::string chars;
    explicit StringObject(std::string s)
        : Object(ObjectType::String), chars(std::move(s)) {
    }
};

struct Value; // forward declaration for NativeFn's signature

// Signature every native binding must match: receives a raw pointer to
// its arguments on the VM stack and how many were passed, returns the
// JS-visible result.
using NativeFn = Value(*)(int argCount, Value* args);

struct NativeObject : Object {
    NativeFn function;
    int arity;        // expected arg count; -1 means variadic
    std::string name; // for error messages ("sqrt expects 1 argument")

    NativeObject(std::string n, int a, NativeFn fn)
        : Object(ObjectType::Native), function(fn), arity(a), name(std::move(n)) {
    }
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Object* obj;
    } as;

    Value() : type(ValueType::Undefined) { as.number = 0; }

    struct NullTag {};
    Value(NullTag) : type(ValueType::Null) { as.number = 0; }
    static Value null() { return Value(NullTag{}); }

    Value(bool b) : type(ValueType::Boolean) { as.boolean = b; }
    Value(double n) : type(ValueType::Number) { as.number = n; }
    Value(int n) : type(ValueType::Number) { as.number = static_cast<double>(n); }

    // Wrapping an Object* is only ever valid for object subtypes (string,
    // native, ...). Kept explicit so a bare Object* can't silently become
    // a Value via implicit conversion from something unrelated.
    explicit Value(Object* o) : type(ValueType::Object) { as.obj = o; }

    bool isUndefined() const { return type == ValueType::Undefined; }
    bool isNull() const { return type == ValueType::Null; }
    bool isBool() const { return type == ValueType::Boolean; }
    bool isNumber() const { return type == ValueType::Number; }
    bool isObject() const { return type == ValueType::Object; }
    bool isString() const { return isObject() && as.obj->type == ObjectType::String; }
    bool isNative() const { return isObject() && as.obj->type == ObjectType::Native; }

    // Unchecked accessors: caller must verify the type first (via is*()).
    bool asBool() const { return as.boolean; }
    double asNumber() const { return as.number; }
    Object* asObject() const { return as.obj; }

    const std::string& asString() const {
        return static_cast<StringObject*>(as.obj)->chars;
    }

    NativeObject* asNative() const {
        return static_cast<NativeObject*>(as.obj);
    }

    // JS truthiness: false, 0, NaN, "", null, undefined are falsy.
    bool isTruthy() const {
        switch (type) {
        case ValueType::Undefined: return false;
        case ValueType::Null:      return false;
        case ValueType::Boolean:   return as.boolean;
        case ValueType::Number:    return as.number != 0.0 && !std::isnan(as.number);
        case ValueType::Object:
            if (isString()) return !asString().empty();
            return true; // natives/functions are always truthy
        }
        return true;
    }

    // Strict equality (===), not JS's coercing ==. Matches OP_EQUAL's
    // typical semantics in a clox-style VM; add a separate loose-equals
    // helper later if the language needs `==` coercion too.
    bool strictEquals(const Value& other) const {
        if (type != other.type) return false;
        switch (type) {
        case ValueType::Undefined: return true;
        case ValueType::Null:      return true;
        case ValueType::Boolean:   return as.boolean == other.as.boolean;
        case ValueType::Number:    return as.number == other.as.number;
        case ValueType::Object:
            if (isString() && other.isString()) {
                return asString() == other.asString();
            }
            return as.obj == other.as.obj; // reference equality otherwise
        }
        return false;
    }
};

inline void printValue(const Value& value) {
    switch (value.type) {
    case ValueType::Undefined:
        std::printf("undefined");
        break;
    case ValueType::Null:
        std::printf("null");
        break;
    case ValueType::Boolean:
        std::printf(value.asBool() ? "true" : "false");
        break;
    case ValueType::Number: {
        double n = value.asNumber();
        if (n == static_cast<long long>(n)) {
            std::printf("%lld", static_cast<long long>(n));
        }
        else {
            std::printf("%g", n);
        }
        break;
    }
    case ValueType::Object:
        if (value.isString()) {
            std::printf("%s", value.asString().c_str());
        }
        else if (value.isNative()) {
            std::printf("<native fn %s>", value.asNative()->name.c_str());
        }
        else {
            std::printf("<object>");
        }
        break;
    }
}
