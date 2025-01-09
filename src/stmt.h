#pragma once

#include "expr.h"
#include "token.h"
#include <ostream>
#include <vector>

struct Stmt {
    virtual ~Stmt() = default;
};

struct FunDeclStmt : public Stmt {
    Token name;
    std::vector<Token> params;
    std::vector<std::unique_ptr<Stmt>> body;

    FunDeclStmt(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), params(params), body(std::move(body)) {}
};

struct StructDeclStmt : public Stmt {
    Token name;
    std::vector<Token> properties;
    std::vector<std::unique_ptr<FunDeclStmt>> methods;

    StructDeclStmt(Token name, std::vector<Token> properties, std::vector<std::unique_ptr<FunDeclStmt>> methods)
        : name(name), properties(properties), methods(std::move(methods)) {}
};

struct VariableDeclStmt : public Stmt {
    Token name;
    std::unique_ptr<Expr> value;

    VariableDeclStmt(Token name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}
};

struct ExprStmt : public Stmt {
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}
};
