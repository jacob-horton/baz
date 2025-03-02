#include "type.h"
#include <algorithm>

bool Type::can_coerce_to(TypeClass tc) { return tc == this->type_class; }

std::string IntType::to_string() { return "int"; }
std::string FloatType::to_string() { return "float"; }
std::string BoolType::to_string() { return "bool"; }
std::string NullType::to_string() { return "null"; }
std::string StrType::to_string() { return "str"; }
std::string VoidType::to_string() { return "void"; }

// TODO: is_equal and can_coerce_to for these
std::string StructType::to_string() { return this->name.lexeme; }
std::string FunctionType::to_string() { return this->name.lexeme; }

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
