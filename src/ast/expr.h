#pragma once

#include "../scanner/token.h"
#include "../type_checker/type.h"

#include <map>
#include <memory>
#include <vector>

// Forward declaration - actual implementation will import the visitor
class ExprVisitor;

struct Expr {
    // TODO: make this optional, and then have get_type which will error and quit if not set
    // NOTE: this gets set during resolving
    std::shared_ptr<Type> type;

    virtual void accept(ExprVisitor &visitor) = 0;

    virtual ~Expr() = default;
};

struct VarExpr : public Expr {
    Token name;

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

struct EnumInitExpr : public Expr {
    Token name;
    std::unique_ptr<VarExpr> enum_namespace;
    std::optional<std::unique_ptr<Expr>> payload;

    EnumInitExpr(Token name, std::unique_ptr<VarExpr> enum_namespace, std::optional<std::unique_ptr<Expr>> payload);

    void accept(ExprVisitor &visitor) override;
};

struct CallExpr : public Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;

    Token bracket;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args, Token bracket);

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
};
