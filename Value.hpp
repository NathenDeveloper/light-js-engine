#pragma once
#include <string>

enum class ValueType {
    Undefined,
    Boolean,
    Number,
    String
};

struct Object {
    ValueType type;
    bool marked;
    Object* next;
    std::string chars;
    
    Object(ValueType t, std::string s) : type(t), marked(false), next(nullptr), chars(s) {}
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Object* obj;
    } as;

    Value() : type(ValueType::Undefined) { as.number = 0; }
    Value(bool b) : type(ValueType::Boolean) { as.boolean = b; }
    Value(double n) : type(ValueType::Number) { as.number = n; }
    Value(Object* o) : type(ValueType::String) { as.obj = o; }

    bool isNumber() const { return type == ValueType::Number; }
    bool isString() const { return type == ValueType::String; }
};
