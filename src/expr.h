#pragma once

#include "token.h"
#include <vector>

struct Expr {
  virtual ~Expr() = default;
};

struct AssignExpr : Expr {
  Token name;
  Expr *value;

  AssignExpr(Token name, Expr *value) : name(name), value(value) {}
};

struct VarExpr : Expr {
  Token name;

  VarExpr(Token name) : name(name) {}
};

struct BinaryExpr : Expr {
  Expr *left;
  Token op;
  Expr *right;

  BinaryExpr(Expr *left, Token op, Expr *right)
      : left(left), op(op), right(right) {}
};

// BinaryExpr, but short-circuits
struct LogicalBinaryExpr : Expr {
  Expr *left;
  Token op;
  Expr *right;

  LogicalBinaryExpr(Expr *left, Token op, Expr *right)
      : left(left), op(op), right(right) {}
};

struct UnaryExpr : Expr {
  Token op;
  Expr *right;

  UnaryExpr(Token op, Expr *right) : op(op), right(right) {}
};

struct GetExpr : Expr {
  Expr *value;
  Token name;

  GetExpr(Expr *value, Token name) : value(value), name(name) {}
};

struct CallExpr : Expr {
  Expr *callee;
  std::vector<Expr *> args;

  CallExpr(Expr *callee, std::vector<Expr *> args)
      : callee(callee), args(args) {}
};

struct GroupingExpr : Expr {
  Expr *expr;

  GroupingExpr(Expr *expr) : expr(expr) {}
};

struct PrimaryExpr : Expr {
  Token primary;

  PrimaryExpr(Token primary) : primary(primary) {}
};
