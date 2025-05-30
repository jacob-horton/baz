#pragma once

#include <cstring>
#include <iomanip>
#include <ios>
#include <ostream>
#include <stddef.h>
#include <string>

// All possible token types
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
    QUESTION_DOT,
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
    COLON_COLON,

    RETURN,
    PRINT,
    PANIC,

    IDENTIFIER,

    EOF_,
};

const std::string get_token_type_str(TokenType t);

struct Token {
    TokenType t;
    std::string lexeme;
    long line;

  public:
    friend std::ostream &operator<<(std::ostream &os, const Token &t);
};
