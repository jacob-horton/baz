#include "type.h"
#include <algorithm>

bool Type::can_coerce_to(TypeClass tc) { return tc == this->type_class; }

std::string IntType::to_string() { return "int"; }
std::string FloatType::to_string() { return "float"; }
std::string BoolType::to_string() { return "bool"; }
std::string NullType::to_string() { return "null"; }
std::string StrType::to_string() { return "str"; }
std::string VoidType::to_string() { return "void"; }

// TODO: can_coerce_to for these
std::string StructType::to_string() { return this->name.lexeme; }
std::string FunctionType::to_string() { return this->name.lexeme; }
std::string EnumType::to_string() { return this->name.lexeme; }

std::optional<std::shared_ptr<Type>> StructType::get_member_type(std::string name) {
    // TODO: should these be a hashmap?
    auto p = std::find_if(this->props.begin(), this->props.end(), [name](const auto &t) {
        return std::get<0>(t).lexeme == name;
    });

    if (p != this->props.end())
        return std::get<1>(*p);

    auto m = std::find_if(this->methods.begin(), this->methods.end(), [name](const auto &t) {
        return std::get<0>(t).lexeme == name;
    });

    if (m != this->methods.end())
        return std::get<1>(*m);

    return {};
}

std::optional<std::shared_ptr<Type>> EnumType::get_method_type(std::string name) {
    auto m = std::find_if(this->methods.begin(), this->methods.end(), [name](const auto &t) {
        return std::get<0>(t).lexeme == name;
    });

    if (m != this->methods.end())
        return std::get<1>(*m);

    return {};
}

std::optional<Token> EnumType::get_variant_payload_type(std::string name) {
    auto m = std::find_if(this->variants.begin(), this->variants.end(), [name](const auto &v) {
        return v.name.lexeme == name;
    });

    if (m != this->variants.end())
        return m->payload_type;

    return {};
}
