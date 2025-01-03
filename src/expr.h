#pragma once

#include "token.h"
#include <memory>
#include <vector>

struct Expr {
    virtual ~Expr() = default;
};

struct AssignExpr : Expr {
    Token name;
    std::unique_ptr<Expr> value;

    AssignExpr(Token name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}
};

struct VarExpr : Expr {
    Token name;

    VarExpr(Token name) : name(name) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

// BinaryExpr, but short-circuits
struct LogicalBinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    LogicalBinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

struct UnaryExpr : Expr {
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
};

struct GetExpr : Expr {
    std::unique_ptr<Expr> value;
    Token name;

    GetExpr(std::unique_ptr<Expr> value, Token name) : value(std::move(value)), name(name) {}
};

struct CallExpr : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(callee)), args(std::move(args)) {}
};

struct GroupingExpr : Expr {
    std::unique_ptr<Expr> expr;

    GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct PrimaryExpr : Expr {
    Token primary;

    PrimaryExpr(Token primary) : primary(primary) {}
};
