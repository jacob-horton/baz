#pragma once

#include "../scanner/token.h"

#include <optional>

struct EnumVariant {
    Token name;

    // TODO: have type + is_optional in a "type" struct - reduce duplication with "typed_var"
    std::optional<Token> payload_type;
    bool is_optional;

    EnumVariant(Token name, std::optional<Token> payload_type, bool is_optional) : name(name), payload_type(payload_type), is_optional(is_optional) {}
};
