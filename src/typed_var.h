#pragma once

#include "token.h"

struct TypedVar {
    Token name;
    Token type;
    bool is_optional;

    TypedVar(Token name, Token type, bool is_optional) : name(name), type(type), is_optional(is_optional) {}
};
