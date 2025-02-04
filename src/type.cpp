#include "type.h"
#include <iostream>

std::unique_ptr<Type> from_literal(Token literal) {
    // TODO: "this" and user defined types
    switch (literal.t) {
        case TokenType::TRUE:
        case TokenType::FALSE:
            return std::make_unique<BoolType>();
        case TokenType::NULL_VAL:
            return std::make_unique<NullType>();
        case TokenType::INT_VAL:
            return std::make_unique<IntType>();
        case TokenType::FLOAT_VAL:
            return std::make_unique<FloatType>();
        case TokenType::STR_VAL:
            return std::make_unique<StrType>();
        default:
            std::cerr << "[BUG] Token type '" << get_token_type_str(literal.t) << "' not handled." << std::endl;
            exit(3);
    }
}

std::unique_ptr<Type> from_typed_var(TypedVar var) {
    switch (var.type.t) {
        case TokenType::TYPE:
            if (var.type.lexeme == "str")
                return std::make_unique<StrType>();

            if (var.type.lexeme == "int")
                return std::make_unique<IntType>();

            if (var.type.lexeme == "float")
                return std::make_unique<FloatType>();

            if (var.type.lexeme == "bool")
                return std::make_unique<BoolType>();

            // TODO: void type
            // TODO: user defined types
        default:
            // TODO: better log
            std::cerr << "[BUG] Unrecognised type " << var.type.t << ": '" << var.type.lexeme << "'." << std::endl;
            exit(3);
    }
}
