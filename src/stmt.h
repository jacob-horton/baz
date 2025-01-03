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
    std::vector<Stmt *> body;

    FunDeclStmt(Token name, std::vector<Token> params, std::vector<Stmt *> body)
        : name(name), params(params), body(body) {}
};

struct StructDeclStmt : public Stmt {
    Token name;
    std::vector<Token> properties;
    std::vector<FunDeclStmt *> methods;

    StructDeclStmt(Token name, std::vector<Token> properties, std::vector<FunDeclStmt *> methods)
        : name(name), methods(methods) {}
};

struct ExprStmt : public Stmt {
    Expr *expr;

    ExprStmt(Expr *expr) : expr(expr) {}
};

struct BlockStmt : public Stmt {
    std::vector<Stmt *> stmts;

    BlockStmt(std::vector<Stmt *> stmts) : stmts(stmts) {}
};
