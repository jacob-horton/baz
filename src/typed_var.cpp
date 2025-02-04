#include "typed_var.h"
#include "token.h"

#include <iostream>

std::unique_ptr<Type> TypedVar::get_type() {
    switch (this->type.t) {
        case TokenType::TYPE:
            if (this->type.lexeme == "str")
                return std::make_unique<StrType>();

            if (this->type.lexeme == "int")
                return std::make_unique<IntType>();

            if (this->type.lexeme == "float")
                return std::make_unique<FloatType>();

            if (this->type.lexeme == "bool")
                return std::make_unique<BoolType>();

            // TODO: void type
            // TODO: user defined types
        default:
            std::cerr << "[BUG] Unrecognised type of token (" << get_token_type_str(this->type.t) << ", '" << this->type.lexeme << "')." << std::endl;
            exit(3);
    }
}
