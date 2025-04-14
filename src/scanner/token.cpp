#include "token.h"

const std::string get_token_type_str(TokenType t) {
    switch (t) {
        case AND:               return "AND";
        case BANG:              return "BANG";
        case BANG_EQUAL:        return "BANG_EQUAL";
        case BOOL_VAL:          return "BOOL_VAL";
        case COLON:             return "COLON";
        case COLON_COLON:       return "COLON_COLON";
        case COMMA:             return "COMMA";
        case DOT:               return "DOT";
        case ELSE:              return "ELSE";
        case ENUM:              return "ENUM";
        case EOF_:              return "EOF";
        case EQUAL:             return "EQUAL";
        case EQUAL_EQUAL:       return "EQUAL_EQUAL";
        case FALSE:             return "FALSE";
        case FLOAT_VAL:         return "FLOAT_VAL";
        case FN:                return "FN";
        case FOR:               return "FOR";
        case GREATER:           return "GREATER";
        case GREATER_EQUAL:     return "GREATER_EQUAL";
        case IDENTIFIER:        return "IDENTIFIER";
        case IF:                return "IF";
        case INTERFACE:         return "INTERFACE";
        case INT_VAL:           return "INT_VAL";
        case LESS:              return "LESS";
        case LESS_EQUAL:        return "LESS_EQUAL";
        case LET:               return "LET";
        case L_BRACKET:         return "L_BRACKET";
        case L_CURLY_BRACKET:   return "L_CURLY_BRACKET";
        case MATCH:             return "MATCH";
        case MINUS:             return "MINUS";
        case NULL_VAL:          return "NULL";
        case OR:                return "OR";
        case PANIC:             return "PANIC";
        case PLUS:              return "PLUS";
        case PRINT:             return "PRINT";
        case QUESTION:          return "QUESTION";
        case QUESTION_DOT:      return "QUESTION_DOT";
        case QUESTION_QUESTION: return "QUESTION_QUESTION";
        case RETURN:            return "RETURN";
        case R_BRACKET:         return "R_BRACKET";
        case R_CURLY_BRACKET:   return "R_CURLY_BRACKET";
        case SEMI_COLON:        return "SEMI_COLON";
        case SLASH:             return "SLASH";
        case STAR:              return "STAR";
        case STRUCT:            return "STRUCT";
        case STR_VAL:           return "STR_VAL";
        case TRUE:              return "TRUE";
        case TYPE:              return "TYPE";
        case WHILE:             return "WHILE";
        default:                return "unknown";
    }
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

    os << t.lexeme;

    return os;
}
