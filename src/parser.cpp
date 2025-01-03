#include "parser.h"
#include "expr.h"
#include "stmt.h"
#include "token.h"
#include <iostream>
#include <ostream>
#include <vector>

Parser::Parser(Scanner scanner) : scanner(scanner) { this->advance(); }

Stmt *Parser::parse_stmt() { return this->declaration(); }

Stmt *Parser::declaration() {
  // if (this->match(TokenType::STRUCT))
  //   return this->struct_decl();

  if (this->match(TokenType::FN))
    return this->function_decl();

  // TODO: only allow statements inside functions or structs or enums
  return this->statement();
}

Stmt *Parser::statement() {
  Expr *expr = this->expression();

  // Assignment
  if (this->match(TokenType::EQUAL)) {
    Expr *value = this->expression();
    this->consume(TokenType::SEMI_COLON, "Expected ';' after assignment.");

    if (VarExpr *var = dynamic_cast<VarExpr *>((Expr *)&expr)) {
      Token name = var->name;
      return new ExprStmt(new AssignExpr{name, value});
    }

    std::cerr << "Invalid assignment target." << std::endl;
    exit(2);
  }

  // Expression statement
  this->consume(TokenType::SEMI_COLON, "Expected ';' after expression.");
  return new ExprStmt(expr);
}

std::vector<Stmt *> Parser::block() {
  std::vector<Stmt *> stmts;
  while (!this->check(TokenType::R_CURLY_BRACKET)) {
    stmts.push_back(this->statement());
  }

  this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after block.");

  return stmts;
}

Token Parser::typed_identifier() {
  Token id = this->consume(TokenType::IDENTIFIER, "Expected identifier.");

  consume(TokenType::COLON, "Expected type for identifier.");
  Token type = this->consume(TokenType::TYPE, "Expected type after ':'.");

  // TODO: do something with type
  return id;
}

FunDeclStmt *Parser::function_decl() {
  // TODO: method or function
  Token name = this->consume(TokenType::IDENTIFIER, "Expected function name.");
  std::vector<Token> params;

  this->consume(TokenType::L_BRACKET, "Expected '(' after function name.");
  if (!this->check(TokenType::R_BRACKET)) {
    // TODO: allow trailing comma
    do {
      params.push_back(this->typed_identifier());
    } while (this->match(TokenType::COMMA));
  }

  this->consume(TokenType::R_BRACKET, "Expected ')' after parameter list.");

  this->consume(TokenType::COLON, "Expected return type.");
  Token type =
      this->consume(TokenType::TYPE, "Expected return type after ':'.");

  this->consume(TokenType::L_CURLY_BRACKET,
                "Expected '{' before function body.");

  std::vector<Stmt *> body = this->block();

  return new FunDeclStmt(name, params, body);
}

Expr *Parser::expression() { return this->logical_or(); }

Expr *Parser::logical_or() {
  Expr *expr = this->logical_and();

  while (this->match(TokenType::OR)) {
    Token op = this->previous();
    Expr *rhs = this->logical_and();
    expr = new LogicalBinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::logical_and() {
  Expr *expr = this->equality();

  while (this->match(TokenType::AND)) {
    Token op = this->previous();
    Expr *rhs = this->equality();
    expr = new LogicalBinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::equality() {
  Expr *expr = this->comparison();

  if (this->match(TokenType::EQUAL_EQUAL) ||
      this->match(TokenType::BANG_EQUAL)) {
    Token op = this->previous();
    Expr *rhs = this->comparison();
    expr = new BinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::comparison() {
  Expr *expr = this->term();

  if (this->match(TokenType::LESS) || this->match(TokenType::LESS_EQUAL) ||
      this->match(TokenType::GREATER) ||
      this->match(TokenType::GREATER_EQUAL)) {
    Token op = this->previous();
    Expr *rhs = this->term();
    expr = new BinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::term() {
  Expr *expr = this->factor();

  while (this->match(TokenType::PLUS) || this->match(TokenType::MINUS)) {
    Token op = this->previous();
    Expr *rhs = this->factor();
    expr = new BinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::factor() {
  Expr *expr = this->fallback();

  while (this->match(TokenType::STAR) || this->match(TokenType::SLASH)) {
    Token op = this->previous();
    Expr *rhs = this->fallback();
    expr = new BinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::fallback() {
  Expr *expr = this->unary();

  if (this->match(TokenType::QUESTION_QUESTION)) {
    Token op = this->previous();
    Expr *rhs = this->unary();
    expr = new BinaryExpr(expr, op, rhs);
  }

  return expr;
}

Expr *Parser::unary() {
  if (this->match(TokenType::BANG) || this->match(TokenType::MINUS)) {
    return new UnaryExpr(this->previous(), this->unary());
  }

  return this->call();
}

Expr *Parser::call() {
  Expr *expr = this->primary();

  while (true) {
    if (this->match(TokenType::L_BRACKET)) {
      expr = this->finish_call(expr);
    } else if (this->match(TokenType::DOT)) {
      Token name = this->consume(TokenType::IDENTIFIER,
                                 "Expect property name after '.'.");
      expr = new GetExpr(expr, name);
    } else {
      break;
    }
  }

  return expr;
}

Expr *Parser::finish_call(Expr *callee) {
  std::vector<Expr *> args;

  if (!this->check(TokenType::R_BRACKET)) {
    do {
      // TODO: allow trailing comma
      args.push_back(this->expression());
    } while (this->match(TokenType::COMMA));
  }

  return new CallExpr(callee, args);
}

Expr *Parser::primary() {
  // TODO: strings
  if (this->match(TokenType::TRUE) || this->match(TokenType::FALSE) ||
      this->match(TokenType::NULL_VAL) || this->match(TokenType::INT_VAL) ||
      this->match(TokenType::FLOAT_VAL) || this->match(TokenType::IDENTIFIER)) {
    return new PrimaryExpr(this->previous());
  }

  if (this->match(TokenType::L_BRACKET)) {
    Expr *expr = this->expression();
    this->consume(TokenType::R_BRACKET, "Expected closing ')'.");
    return new GroupingExpr(expr);
  }

  std::cerr << "Expected expression." << std::endl;
  exit(2);
}

std::optional<Token> Parser::advance() {
  this->prev = this->current;
  this->current = scanner.scan_token();

  return this->prev;
}

std::optional<Token> Parser::peek() { return this->current; }
Token Parser::previous() {
  if (!this->prev.has_value()) {
    std::cerr << "[BUG] Expected previous token to exist." << std::endl;
    exit(3);
  }

  return this->prev.value();
}

bool Parser::match(TokenType t) {
  if (this->check(t)) {
    this->advance();
    return true;
  }

  return false;
}

bool Parser::check(TokenType t) {
  std::optional<Token> curr = this->peek();
  if (!curr.has_value())
    return false;

  return curr->t == t;
}

Token Parser::consume(TokenType t, std::string error_message) {
  if (this->check(t)) {
    auto curr = this->advance();
    if (curr.has_value())
      return curr.value();
  }

  std::cerr << error_message << std::endl;
  exit(2);
}
