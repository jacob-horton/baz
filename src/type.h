#pragma once

#include "token.h"
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

    virtual ~Type() = default;

    virtual bool can_coerce_to(TypeClass tc);
    virtual bool is_equal(const Type &other);
    virtual std::string to_string() = 0;
};

struct IntType : public Type {
    IntType() : Type(TypeClass::INT) {}

    std::string to_string() override;
};

struct FloatType : public Type {
    FloatType() : Type(TypeClass::FLOAT) {}

    std::string to_string() override;
};

struct BoolType : public Type {
    BoolType() : Type(TypeClass::BOOL) {}

    std::string to_string() override;
};

struct NullType : public Type {
    NullType() : Type(TypeClass::NULL_) {}

    std::string to_string() override;
};

struct StrType : public Type {
    StrType() : Type(TypeClass::STR) {}

    std::string to_string() override;
};

bool operator==(const Type &lhs, const Type &rhs);
bool operator!=(const Type &lhs, const Type &rhs);
