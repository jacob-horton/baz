#pragma once

#include "../ast/enum_variant.h"
#include "../ast/expr.h"
#include "../ast/stmt.h"
#include "../scanner/scanner.h"

#include <memory>
#include <optional>
#include <string>

class Parser {
  private:
    std::unique_ptr<Scanner> scanner;

    std::optional<Token> prev;
    Token current;

    std::optional<std::unique_ptr<Stmt>> top_level_decl();
    std::unique_ptr<Stmt> nested_decl();

    std::unique_ptr<FunDeclStmt> function_decl(FunType fun_type);
    std::unique_ptr<StructDeclStmt> struct_decl();
    std::unique_ptr<EnumDeclStmt> enum_decl();
    std::unique_ptr<VariableDeclStmt> variable_decl();

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> assignment(Expr &lhs);
    std::unique_ptr<IfStmt> if_statement();
    std::unique_ptr<MatchStmt> match_statement();
    std::unique_ptr<ForStmt> for_statement();
    std::unique_ptr<WhileStmt> while_statement();
    std::unique_ptr<PrintStmt> print_statement();
    std::unique_ptr<PanicStmt> panic_statement();
    std::unique_ptr<ReturnStmt> return_statement();

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
    std::unique_ptr<Expr> finish_struct_init(Token name);

    MatchPattern match_pattern();

    std::vector<std::unique_ptr<Stmt>> block();
    TypedVar typed_identifier();
    EnumVariant enum_variant_decl();
    Token type();

    Token advance();
    Token peek();
    Token previous();

    bool match(TokenType t);
    bool check(TokenType t);
    Token consume(TokenType t, std::string error_message);

    void error(Token error_token, std::string message);

  public:
    Parser(std::unique_ptr<Scanner> scanner);

    std::optional<std::unique_ptr<Stmt>> parse_stmt();
};
