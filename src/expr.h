#pragma once

#include "token.h"
#include <memory>
#include <vector>

struct Expr {
    virtual ~Expr() = default;
};

struct AssignExpr : public Expr {
    Token name;
    std::unique_ptr<Expr> value;

    AssignExpr(Token name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}
};

struct VarExpr : public Expr {
    Token name;

    VarExpr(Token name) : name(name) {}
};

struct StructInitExpr : public Expr {
    Token name;
    std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties;

    StructInitExpr(Token name, std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties) : name(name), properties(std::move(properties)) {}
};

struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

// Smae as BinaryExpr, but short-circuits
struct LogicalBinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    LogicalBinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

struct UnaryExpr : public Expr {
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
};

struct GetExpr : public Expr {
    std::unique_ptr<Expr> value;
    Token name;

    GetExpr(std::unique_ptr<Expr> value, Token name) : value(std::move(value)), name(name) {}
};

struct CallExpr : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(callee)), args(std::move(args)) {}
};

struct GroupingExpr : public Expr {
    std::unique_ptr<Expr> expr;

    GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct LiteralExpr : public Expr {
    Token literal;

    LiteralExpr(Token literal) : literal(literal) {}
};
