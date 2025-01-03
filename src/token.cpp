#include "token.h"

const std::string get_token_type_str(TokenType t) {
    switch (t) {
        case L_CURLY_BRACKET:   return "L_CURLY_BRACKET";
        case R_CURLY_BRACKET:   return "R_CURLY_BRACKET";
        case L_BRACKET:         return "L_BRACKET";
        case R_BRACKET:         return "R_BRACKET";
        case SEMI_COLON:        return "SEMI_COLON";
        case COLON:             return "COLON";
        case COMMA:             return "COMMA";
        case DOT:               return "DOT";
        case QUESTION:          return "QUESTION";
        case LET:               return "LET";
        case MATCH:             return "MATCH";
        case FN:                return "FN";
        case IF:                return "IF";
        case FOR:               return "FOR";
        case WHILE:             return "WHILE";
        case STRUCT:            return "STRUCT";
        case ENUM:              return "ENUM";
        case INTERFACE:         return "INTERFACE";
        case TYPE:              return "TYPE";
        case INT_VAL:           return "INT_VAL";
        case FLOAT_VAL:         return "FLOAT_VAL";
        case STR_VAL:           return "STR_VAL";
        case BOOL_VAL:          return "BOOL_VAL";
        case NULL_VAL:          return "NULL";
        case TRUE:              return "TRUE";
        case FALSE:             return "FALSE";
        case EQUAL:             return "EQUAL";
        case EQUAL_EQUAL:       return "EQUAL_EQUAL";
        case BANG:              return "BANG";
        case BANG_EQUAL:        return "BANG_EQUAL";
        case LESS:              return "LESS";
        case LESS_EQUAL:        return "LESS_EQUAL";
        case GREATER:           return "GREATER";
        case GREATER_EQUAL:     return "GREATER_EQUAL";
        case IDENTIFIER:        return "IDENTIFIER";
        case RETURN:            return "RETURN";
        case PRINT:             return "PRINT";
        case PLUS:              return "PLUS";
        case MINUS:             return "MINUS";
        case STAR:              return "STAR";
        case SLASH:             return "SLASH";
        case AND:               return "AND";
        case OR:                return "OR";
        case QUESTION_QUESTION: return "QUESTION_QUESTION";
        default:                return "unknown";
    }
}

const std::string Token::get_raw_token() {
    return std::string(this->start, this->length);
}

std::ostream &operator<<(std::ostream &os, const Token &t) {
    os << " ";

    os << std::setw(4);
    os << std::setfill(' ');
    os << std::right;
    os << t.line;

    os << " | ";

    os << std::setw(22);
    os << std::left;
    os << get_token_type_str(t.t);

    std::string word(t.start, t.length);
    os << word;

    return os;
}
