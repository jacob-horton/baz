#pragma once

#include "scanner.h"
#include "stmt.h"
#include <memory>
#include <optional>
#include <string>

class Parser {
  private:
    std::unique_ptr<Scanner> scanner;

    std::optional<Token> prev;
    std::optional<Token> current;

    std::unique_ptr<Stmt> declaration();

    std::unique_ptr<FunDeclStmt> function_decl();
    std::unique_ptr<StructDeclStmt> struct_decl();
    std::unique_ptr<Stmt> statement();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> logical_or();
    std::unique_ptr<Expr> logical_and();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> fallback();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> finish_call(std::unique_ptr<Expr> expr);

    std::vector<std::unique_ptr<Stmt> > block();
    Token typed_identifier();

    std::optional<Token> advance();
    std::optional<Token> peek();
    Token previous();

    bool match(TokenType t);
    bool check(TokenType t);
    Token consume(TokenType t, std::string error_message);

  public:
    Parser(std::unique_ptr<Scanner> scanner);

    std::unique_ptr<Stmt> parse_stmt();
};
