#pragma once

#include "../scanner/token.h"
#include <memory>
#include <tuple>
#include <vector>

enum TypeClass {
    INT,
    FLOAT,
    BOOL,
    NULL_,
    STR,
    VOID,
    STRUCT_,
    FUNC,
};

struct Type {
    TypeClass type_class;

    Type(TypeClass tc) : type_class(tc) {}

    virtual ~Type() = default;

    virtual bool can_coerce_to(TypeClass tc);
    virtual bool is_equal(const Type &other) const;
    virtual std::string to_string() = 0;
};

// TODO: singletons, and equality is just pointer check?
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

struct VoidType : public Type {
    VoidType() : Type(TypeClass::VOID) {}

    std::string to_string() override;
};

struct FunctionType : public Type {
    Token name;
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> params;
    std::shared_ptr<Type> return_type;

    FunctionType(Token name, std::vector<std::tuple<Token, std::shared_ptr<Type>>> params, std::shared_ptr<Type> return_type) : Type(TypeClass::FUNC), name(name), params(params), return_type(return_type) {}

    std::string to_string() override;
};

// TODO: support methods
struct StructType : public Type {
    Token name;
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> props;

    StructType(Token name, std::vector<std::tuple<Token, std::shared_ptr<Type>>> props) : Type(TypeClass::STRUCT_), name(name), props(props) {}

    std::string to_string() override;
};

bool operator==(const Type &lhs, const Type &rhs);
bool operator!=(const Type &lhs, const Type &rhs);
