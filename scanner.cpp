#include "token.h"
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <ostream>
#include <string>

struct EqualSymbol {
  TokenType single;
  TokenType with_equal;
};

const std::map<std::string, TokenType> KEYWORDS = {
    {"interface", TokenType::INTERFACE},
    {"struct", TokenType::STRUCT},
    {"enum", TokenType::ENUM},
    {"fn", TokenType::FN},

    {"let", TokenType::LET},
    {"match", TokenType::MATCH},
    {"if", TokenType::IF},

    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},

    {"void", TokenType::VOID_TYPE},
    {"int", TokenType::INT_TYPE},
    {"float", TokenType::FLOAT_TYPE},
    {"str", TokenType::STR_TYPE},
    {"bool", TokenType::BOOL_TYPE},

    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
};

// All symbols that can be singular or have an equal after them e.g. `!=`
const std::map<char, EqualSymbol> EQUAL_SYMBOLS = {
    {'=', EqualSymbol{TokenType::EQUAL, TokenType::EQUAL_EQUAL}},
    {'!', EqualSymbol{TokenType::BANG, TokenType::BANG_EQUAL}},
    {'<', EqualSymbol{TokenType::LESS, TokenType::LESS_EQUAL}},
    {'>', EqualSymbol{TokenType::GREATER, TokenType::GREATER_EQUAL}},
};

const std::map<char, TokenType> SYMBOLS = {
    {'{', TokenType::L_CURLY_BRACKET}, {'}', TokenType::R_CURLY_BRACKET},
    {'(', TokenType::L_BRACKET},       {')', TokenType::R_BRACKET},
    {';', TokenType::SEMI_COLON},      {':', TokenType::COLON},
    {',', TokenType::COMMA},           {'?', TokenType::QUESTION},
};

bool is_alpha(char c) {
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
    return true;

  return false;
}

bool is_digit(char c) {
  if (c >= '0' && c <= '9')
    return true;

  return false;
}

bool is_alphanum(char c) {
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
      (c >= '0' && c <= '9'))
    return true;

  return false;
}

class Scanner {
private:
  const char *token_start;
  const char *current;
  const char *end;
  long line;

  Token make_token(TokenType t) {
    return Token{
        t,
        this->token_start,
        this->current - this->token_start,
        line,
    };
  }

  Token number() {
    TokenType type = TokenType::INT_VAL;

    // Consume digits - we already know we've got an initial one
    while (is_digit(this->peek()))
      this->advance();

    // If reach a `.`, include it and continue matching digits
    // We know it is a float at this point
    if (this->match('.')) {
      type = TokenType::FLOAT_VAL;
      while (is_digit(this->peek()))
        this->advance();
    }

    return this->make_token(type);
  }

  // Returns TokenType of keyword if the current token is one
  std::optional<TokenType> get_keyword_type() {
    char word[this->current - this->token_start + 1];
    strncpy(word, this->token_start, this->current - this->token_start);
    word[this->current - this->token_start] = 0;

    auto token_type = KEYWORDS.find(word);
    if (token_type == KEYWORDS.end()) {
      return {};
    }

    return token_type->second;
  }

  // If token is a keyword, return the keyword type,
  // otherwise it is an identifier
  Token identifier_or_keyword() {
    // Consume alphanumeric - we've already consumed alpha
    while (is_alphanum(this->peek()))
      this->advance();

    std::optional<TokenType> keyword_type = this->get_keyword_type();
    if (keyword_type.has_value())
      return this->make_token(keyword_type.value());

    return this->make_token(TokenType::IDENTIFIER);
  }

  Token symbol(char start) {
    // TODO: strings in quotes
    TokenType type;

    auto equal_token_type = EQUAL_SYMBOLS.find(start);
    if (equal_token_type != EQUAL_SYMBOLS.end()) {
      if (this->match('='))
        return this->make_token(equal_token_type->second.with_equal);
      else
        return this->make_token(equal_token_type->second.single);
    }

    auto token_type = SYMBOLS.find(start);
    if (token_type == SYMBOLS.end()) {
      // TODO: handle error properly
      std::cerr << "Unrecognised symbol: '" << start << "'" << std::endl;
      exit(0);
    }

    return this->make_token(token_type->second);
  }

  void skip_whitespace() {
    // TODO: comments
    while (true) {
      switch (this->peek()) {
      case '\n':
        this->line++;
        this->advance();
        break;
      case ' ':
      case '\t':
      case '\r':
        this->advance();
        break;
      default:
        return;
      }
    }
  }

  bool match(char c) {
    if (*this->current == c) {
      this->current++;
      return true;
    }

    return false;
  }

  char advance() {
    char current = *this->current;
    this->current++;

    return current;
  }

  char peek() { return *this->current; }

public:
  Scanner(const char *source) {
    this->current = source;
    this->end = source + strlen(source);
    this->line = 1;
  }

  std::optional<Token> scan_token() {
    if (this->current >= this->end) {
      return {};
    }

    this->skip_whitespace();
    this->token_start = this->current;

    char c = this->advance();
    if (is_digit(c))
      return this->number();

    if (is_alpha(c))
      return this->identifier_or_keyword();

    return this->symbol(c);
  }
};
