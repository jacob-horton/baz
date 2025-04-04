#pragma once

#include "../ast/enum_variant.h"
#include "../ast/typed_var.h"
#include "../scanner/token.h"

#include <memory>
#include <optional>
#include <tuple>
#include <vector>

// TODO: please remove
struct Temp {
    Token type;
    bool optional;
};

enum TypeClass {
    INT,
    FLOAT,
    BOOL,
    NULL_,
    STR,
    VOID,
    STRUCT_,
    ENUM_,
    FUNC,
};

struct Type {
    TypeClass type_class;

    Type(TypeClass tc) : type_class(tc) {}

    virtual ~Type() = default;

    virtual bool can_coerce_to(std::shared_ptr<Type> t);
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

struct VoidType : public Type {
    VoidType() : Type(TypeClass::VOID) {}

    std::string to_string() override;
};

// TODO: store this on the function?
struct FunctionType : public Type {
    Token name;
    std::vector<TypedVar> params;
    Token return_type;
    bool return_type_optional;

    FunctionType(Token name, std::vector<TypedVar> params, Token return_type, bool return_type_optional) : Type(TypeClass::FUNC), name(name), params(params), return_type(return_type), return_type_optional(return_type_optional) {}

    std::string to_string() override;
};

struct StructType : public Type {
    Token name;
    std::vector<TypedVar> props;
    // TODO: just store identifiers rather than types?
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods;

    StructType(Token name, std::vector<TypedVar> props, std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods) : Type(TypeClass::STRUCT_), name(name), props(props), methods(methods) {}

    std::optional<std::shared_ptr<Type>> get_method_type(std::string name);
    std::optional<Temp> get_prop_type(std::string name);

    std::string to_string() override;
};

struct EnumType : public Type {
    Token name;
    std::vector<EnumVariant> variants;
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods;

    EnumType(Token name, std::vector<EnumVariant> variants, std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods) : Type(TypeClass::ENUM_), name(name), variants(variants), methods(methods) {}

    std::optional<std::shared_ptr<Type>> get_method_type(std::string name);
    std::optional<Temp> get_variant_payload_type(std::string name);

    std::string to_string() override;
};
