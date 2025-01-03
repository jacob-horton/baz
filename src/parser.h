#pragma once

#include "scanner.h"
#include "stmt.h"
#include <optional>
#include <string>

class Parser {
private:
  Scanner scanner;

  std::optional<Token> prev;
  std::optional<Token> current;

  Stmt *declaration();

  FunDeclStmt *function_decl();
  Stmt *statement();

  Expr *expression();
  Expr *logical_or();
  Expr *logical_and();
  Expr *equality();
  Expr *comparison();
  Expr *term();
  Expr *factor();
  Expr *fallback();
  Expr *unary();
  Expr *call();
  Expr *primary();
  Expr *finish_call(Expr *expr);

  std::vector<Stmt *> block();
  Token typed_identifier();

  std::optional<Token> advance();
  std::optional<Token> peek();
  Token previous();

  bool match(TokenType t);
  bool check(TokenType t);
  Token consume(TokenType t, std::string error_message);

public:
  Parser(Scanner scanner);

  Stmt *parse_stmt();
};
