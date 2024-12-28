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
  FOR,
  WHILE,
  STRUCT,
  ENUM,
  INTERFACE,
  VOID_TYPE,
  INT_TYPE,
  FLOAT_TYPE,
  STR_TYPE,
  BOOL_TYPE,
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

  RETURN,
  PRINT,

  IDENTIFIER,
};

struct Token {
  TokenType t;
  const char *start;
  long length;
  long line;

public:
  const std::string get_raw_token() {
    return std::string(this->start, this->length);
  }

  const std::string get_token_type_str() const {
    switch (this->t) {
    case L_CURLY_BRACKET:
      return "L_CURLY_BRACKET";
    case R_CURLY_BRACKET:
      return "R_CURLY_BRACKET";
    case L_BRACKET:
      return "L_BRACKET";
    case R_BRACKET:
      return "R_BRACKET";
    case SEMI_COLON:
      return "SEMI_COLON";
    case COLON:
      return "COLON";
    case COMMA:
      return "COMMA";
    case DOT:
      return "DOT";
    case QUESTION:
      return "QUESTION";
    case LET:
      return "LET";
    case MATCH:
      return "MATCH";
    case FN:
      return "FN";
    case IF:
      return "IF";
    case FOR:
      return "FOR";
    case WHILE:
      return "WHILE";
    case STRUCT:
      return "STRUCT";
    case ENUM:
      return "ENUM";
    case INTERFACE:
      return "INTERFACE";
    case VOID_TYPE:
      return "VOID_TYPE";
    case INT_TYPE:
      return "INT_TYPE";
    case FLOAT_TYPE:
      return "FLOAT_TYPE";
    case STR_TYPE:
      return "STR_TYPE";
    case BOOL_TYPE:
      return "BOOL_TYPE";
    case INT_VAL:
      return "INT_VAL";
    case FLOAT_VAL:
      return "FLOAT_VAL";
    case STR_VAL:
      return "STR_VAL";
    case BOOL_VAL:
      return "BOOL_VAL";
    case NULL_VAL:
      return "NULL";
    case TRUE:
      return "TRUE";
    case FALSE:
      return "FALSE";
    case EQUAL:
      return "EQUAL";
    case EQUAL_EQUAL:
      return "EQUAL_EQUAL";
    case BANG:
      return "BANG";
    case BANG_EQUAL:
      return "BANG_EQUAL";
    case LESS:
      return "LESS";
    case LESS_EQUAL:
      return "LESS_EQUAL";
    case GREATER:
      return "GREATER";
    case GREATER_EQUAL:
      return "GREATER_EQUAL";
    case IDENTIFIER:
      return "IDENTIFIER";
    case RETURN:
      return "RETURN";
    case PRINT:
      return "PRINT";
    default:
      return "unknown";
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Token &t) {
    os << " ";

    os << std::setw(4);
    os << std::setfill(' ');
    os << std::right;
    os << t.line;

    os << " | ";

    os << std::setw(22);
    os << std::left;
    os << t.get_token_type_str();

    std::string word(t.start, t.length);
    os << word;

    return os;
  }
};
