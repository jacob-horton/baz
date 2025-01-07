#pragma once

#include "token.h"

#include <map>
#include <optional>

struct EqualSymbol {
    TokenType single;
    TokenType with_equal;
};

const std::map<std::string, TokenType> KEYWORDS = {
    {"interface", TokenType::INTERFACE},
    {"struct", TokenType::STRUCT},
    {"enum", TokenType::ENUM},
    {"fn", TokenType::FN},
    {"return", TokenType::RETURN},

    {"print", TokenType::PRINT},
    {"println", TokenType::PRINT},

    {"let", TokenType::LET},
    {"match", TokenType::MATCH},
    {"if", TokenType::IF},

    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},

    {"void", TokenType::TYPE},
    {"int", TokenType::TYPE},
    {"float", TokenType::TYPE},
    {"str", TokenType::TYPE},
    {"bool", TokenType::TYPE},

    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_VAL},
};

// All symbols that can be singular or have an equal after them e.g. `!=`
const std::map<char, EqualSymbol> EQUAL_SYMBOLS = {
    {'=', EqualSymbol{TokenType::EQUAL, TokenType::EQUAL_EQUAL}},
    {'!', EqualSymbol{TokenType::BANG, TokenType::BANG_EQUAL}},
    {'<', EqualSymbol{TokenType::LESS, TokenType::LESS_EQUAL}},
    {'>', EqualSymbol{TokenType::GREATER, TokenType::GREATER_EQUAL}},
};

// All symbols that are repeated twice e.g. "&&", "||"
const std::map<char, TokenType> DOUBLE_SYMBOLS = {
    {'&', TokenType::AND},
    {'|', TokenType::OR},
    {'?', TokenType::QUESTION_QUESTION},
};

const std::map<char, TokenType> SYMBOLS = {
    {'{', TokenType::L_CURLY_BRACKET},
    {'}', TokenType::R_CURLY_BRACKET},
    {'(', TokenType::L_BRACKET},
    {')', TokenType::R_BRACKET},
    {';', TokenType::SEMI_COLON},
    {':', TokenType::COLON},
    {',', TokenType::COMMA},
    {'?', TokenType::QUESTION},
    {'.', TokenType::DOT},
    {'+', TokenType::PLUS},
    {'-', TokenType::MINUS},
    {'*', TokenType::STAR},
    {'/', TokenType::SLASH},
};

class Scanner {
  public:
    virtual std::optional<Token> scan_token() = 0;
    virtual ~Scanner() = default;
};

class TextScanner : public Scanner {
  private:
    const char *token_start;
    const char *current;
    const char *end;
    long line;

    Token make_token(TokenType t);

    Token number();

    // Returns TokenType of keyword if the current token is one
    std::optional<TokenType> get_keyword_type();

    // If token is a keyword, return the keyword type,
    // otherwise it is an identifier
    Token identifier_or_keyword();

    Token symbol(char start);
    void skip_whitespace();

    bool match(char c);
    char advance();
    char peek();

  public:
    TextScanner(std::string &source);

    std::optional<Token> scan_token() override;
};
