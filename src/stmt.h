#pragma once

#include "expr.h"
#include "token.h"
#include "typed_var.h"
#include <optional>
#include <ostream>
#include <vector>

struct Stmt {
    virtual ~Stmt() = default;
};

struct FunDeclStmt : public Stmt {
    Token name;
    std::vector<TypedVar> params;
    std::vector<std::unique_ptr<Stmt>> body;

    FunDeclStmt(Token name, std::vector<TypedVar> params, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), params(params), body(std::move(body)) {}
};

struct StructDeclStmt : public Stmt {
    Token name;
    std::vector<TypedVar> properties;
    std::vector<std::unique_ptr<FunDeclStmt>> methods;

    StructDeclStmt(Token name, std::vector<TypedVar> properties, std::vector<std::unique_ptr<FunDeclStmt>> methods)
        : name(name), properties(properties), methods(std::move(methods)) {}
};

struct VariableDeclStmt : public Stmt {
    TypedVar name;
    std::unique_ptr<Expr> value;

    VariableDeclStmt(TypedVar name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}
};

struct ExprStmt : public Stmt {
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> stmts;

    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts) : condition(std::move(condition)), stmts(std::move(stmts)) {}
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> stmts;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts) : condition(std::move(condition)), stmts(std::move(stmts)) {}
};

struct ForStmt : public Stmt {
    std::unique_ptr<Stmt> var;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> increment;
    std::vector<std::unique_ptr<Stmt>> stmts;

    ForStmt(std::unique_ptr<Stmt> var, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> increment, std::vector<std::unique_ptr<Stmt>> stmts)
        : var(std::move(var)), condition(std::move(condition)), increment(std::move(increment)), stmts(std::move(stmts)) {}
};

struct PrintStmt : public Stmt {
    std::optional<std::unique_ptr<Expr>> expr;

    PrintStmt(std::optional<std::unique_ptr<Expr>> expr) : expr(std::move(expr)) {}
};

struct ReturnStmt : public Stmt {
    std::optional<std::unique_ptr<Expr>> expr;

    ReturnStmt(std::optional<std::unique_ptr<Expr>> expr) : expr(std::move(expr)) {}
};
