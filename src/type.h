#pragma once

#include "typed_var.h"
#include <memory>

enum TypeClass {
    INT,
    FLOAT,
    BOOL,
    NULL_,
    STR,
};

struct Type {
    TypeClass type_class;

    Type(TypeClass tc) : type_class(tc) {}

    virtual bool can_coerce_to(Type type) {
        return false;
    };

    virtual ~Type() = default;
};

struct IntType : public Type {
    IntType() : Type(TypeClass::INT) {}
};

struct FloatType : public Type {
    FloatType() : Type(TypeClass::FLOAT) {}
};

struct BoolType : public Type {
    BoolType() : Type(TypeClass::BOOL) {}
};

struct NullType : public Type {
    NullType() : Type(TypeClass::NULL_) {}
};

struct StrType : public Type {
    StrType() : Type(TypeClass::STR) {}
};

// TODO: better naming
std::unique_ptr<Type> from_literal(Token literal);
std::unique_ptr<Type> from_typed_var(TypedVar var);
