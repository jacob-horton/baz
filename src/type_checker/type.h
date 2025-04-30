#pragma once

#include "../ast/enum_variant.h"
#include "../ast/typed_var.h"
#include "../scanner/token.h"

#include <memory>
#include <optional>
#include <tuple>
#include <vector>

struct OptionalTypeInfo {
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

//// All possible types within Baz

// Primitive types will be unique
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

// There may be many function types with different parameters/return types
struct FunctionType : public Type {
    Token name;
    std::vector<TypedVar> params;
    Token return_type;
    bool return_type_optional;

    FunctionType(Token name, std::vector<TypedVar> params, Token return_type, bool return_type_optional) : Type(TypeClass::FUNC), name(name), params(params), return_type(return_type), return_type_optional(return_type_optional) {}

    std::string to_string() override;
};

// There may be many struct types with different properties/methods
struct StructType : public Type {
    Token name;
    std::vector<TypedVar> props;
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods;

    StructType(Token name, std::vector<TypedVar> props, std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods) : Type(TypeClass::STRUCT_), name(name), props(props), methods(methods) {}

    std::optional<std::shared_ptr<Type>> get_method_type(std::string name);
    std::optional<OptionalTypeInfo> get_prop_type(std::string name);

    std::string to_string() override;
};

// There may be many enum types with different properties/methods
struct EnumType : public Type {
    Token name;
    std::vector<EnumVariant> variants;
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods;

    EnumType(Token name, std::vector<EnumVariant> variants, std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods) : Type(TypeClass::ENUM_), name(name), variants(variants), methods(methods) {}

    std::optional<std::shared_ptr<Type>> get_method_type(std::string name);
    std::optional<OptionalTypeInfo> get_variant_payload_type(std::string name);

    std::string to_string() override;
};
