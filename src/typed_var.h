#pragma once

#include "token.h"
#include "type.h"

#include <memory>

struct TypedVar {
    Token name;
    Token type;
    bool is_optional;

    TypedVar(Token name, Token type, bool is_optional) : name(name), type(type), is_optional(is_optional) {}

    std::unique_ptr<Type> get_type();
};
