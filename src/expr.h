#pragma once

#include "token.h"
#include "type.h"

#include <memory>
#include <vector>

// Forward declaration - actual implementation will import the visitor
class ExprVisitor;

struct Expr {
    virtual void accept(ExprVisitor &visitor) = 0;

    virtual ~Expr() = default;
};

struct VarExpr : public Expr {
    Token name;
    std::shared_ptr<Type> type;

    VarExpr(Token name);

    void accept(ExprVisitor &visitor) override;
};

struct StructInitExpr : public Expr {
    Token name;
    std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties;

    StructInitExpr(Token name, std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties);

    void accept(ExprVisitor &visitor) override;
};

struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right);

    void accept(ExprVisitor &visitor) override;
};

// Smae as BinaryExpr, but short-circuits
struct LogicalBinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    LogicalBinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right);

    void accept(ExprVisitor &visitor) override;
};

struct UnaryExpr : public Expr {
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token op, std::unique_ptr<Expr> right);

    void accept(ExprVisitor &visitor) override;
};

struct GetExpr : public Expr {
    std::unique_ptr<Expr> value;
    Token name;

    GetExpr(std::unique_ptr<Expr> value, Token name);

    void accept(ExprVisitor &visitor) override;
};

struct CallExpr : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args);

    void accept(ExprVisitor &visitor) override;
};

struct GroupingExpr : public Expr {
    std::unique_ptr<Expr> expr;

    GroupingExpr(std::unique_ptr<Expr> expr);

    void accept(ExprVisitor &visitor) override;
};

struct LiteralExpr : public Expr {
    Token literal;

    LiteralExpr(Token literal);

    void accept(ExprVisitor &visitor) override;
    std::unique_ptr<Type> get_type();
};
