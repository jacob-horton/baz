#pragma once

#include <cstring>
#include <iomanip>
#include <ios>
#include <ostream>
#include <stddef.h>
#include <string>

enum TokenType {
    L_CURLY_BRACKET,
    R_CURLY_BRACKET,
    L_BRACKET,
    R_BRACKET,
    SEMI_COLON,
    COLON,
    COMMA,
    DOT,
    QUESTION,
    LET,
    MATCH,
    FN,
    IF,
    ELSE,
    FOR,
    WHILE,
    STRUCT,
    ENUM,
    INTERFACE,

    // TODO: should this be combined with IDENTIFIER? Since a type can be built
    // in or user-defined Or do we check for TYPE or IDENTIFIER in the parser
    TYPE,

    INT_VAL,
    FLOAT_VAL,
    STR_VAL,
    BOOL_VAL,
    NULL_VAL,
    TRUE,
    FALSE,

    EQUAL,
    EQUAL_EQUAL,
    BANG,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,

    OR,
    AND,

    PLUS,
    MINUS,
    STAR,
    SLASH,

    QUESTION_QUESTION,

    RETURN,
    PRINT,

    IDENTIFIER,
};

const std::string get_token_type_str(TokenType t);

struct Token {
    TokenType t;
    std::string literal;
    long line;

  public:
    friend std::ostream &operator<<(std::ostream &os, const Token &t);
};
