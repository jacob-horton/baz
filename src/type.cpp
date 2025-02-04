#include "type.h"

bool operator==(const Type &lhs, const Type &rhs) {
    return lhs.type_class == rhs.type_class;
}

bool operator!=(const Type &lhs, const Type &rhs) {
    return !(lhs.type_class == rhs.type_class);
}

bool Type::can_coerce_to(TypeClass tc) { return tc == this->type_class; }
bool Type::is_equal(const Type &other) { return this->type_class == other.type_class; }

std::string IntType::to_string() { return "int"; }
std::string FloatType::to_string() { return "float"; }
std::string BoolType::to_string() { return "bool"; }
std::string NullType::to_string() { return "null"; }
std::string StrType::to_string() { return "str"; }
