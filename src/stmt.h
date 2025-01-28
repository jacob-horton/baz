#pragma once

#include "enum_variant.h"
#include "expr.h"
#include "token.h"
#include "typed_var.h"

#include <optional>
#include <ostream>
#include <vector>

// Forward declaration - actual implementation will import the visitor
class StmtVisitor;

struct Stmt {
    virtual void accept(StmtVisitor &visitor) = 0;

    virtual ~Stmt() = default;
};

struct FunDeclStmt : public Stmt {
    Token name;
    Token return_type;
    std::vector<TypedVar> params;
    std::vector<std::unique_ptr<Stmt>> body;

    FunDeclStmt(Token name, std::vector<TypedVar> params, Token return_type, std::vector<std::unique_ptr<Stmt>> body);

    void accept(StmtVisitor &visitor) override;
};

struct StructDeclStmt : public Stmt {
    Token name;
    std::vector<TypedVar> properties;
    std::vector<std::unique_ptr<FunDeclStmt>> methods;

    StructDeclStmt(Token name, std::vector<TypedVar> properties, std::vector<std::unique_ptr<FunDeclStmt>> methods);

    void accept(StmtVisitor &visitor) override;
};

struct EnumDeclStmt : public Stmt {
    Token name;
    std::vector<EnumVariant> variants;
    std::vector<std::unique_ptr<FunDeclStmt>> methods;

    EnumDeclStmt(Token name, std::vector<EnumVariant> variants, std::vector<std::unique_ptr<FunDeclStmt>> methods);

    void accept(StmtVisitor &visitor) override;
};

struct VariableDeclStmt : public Stmt {
    TypedVar name;
    std::unique_ptr<Expr> value;

    VariableDeclStmt(TypedVar name, std::unique_ptr<Expr> value);

    void accept(StmtVisitor &visitor) override;
};

struct ExprStmt : public Stmt {
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr);

    void accept(StmtVisitor &visitor) override;
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts);

    void accept(StmtVisitor &visitor) override;
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> true_block;
    std::optional<std::vector<std::unique_ptr<Stmt>>> false_block;

    IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> true_block, std::optional<std::vector<std::unique_ptr<Stmt>>> false_block);

    void accept(StmtVisitor &visitor) override;
};

struct MatchBranch {
    std::unique_ptr<Expr> pattern;
    std::vector<std::unique_ptr<Stmt>> body;

    MatchBranch(std::unique_ptr<Expr> pattern, std::vector<std::unique_ptr<Stmt>> body);
};

struct MatchStmt : public Stmt {
    std::unique_ptr<Expr> target;
    std::vector<MatchBranch> branches;

    MatchStmt(std::unique_ptr<Expr> target, std::vector<MatchBranch> branches);

    void accept(StmtVisitor &visitor) override;
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> stmts;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts);

    void accept(StmtVisitor &visitor) override;
};

struct ForStmt : public Stmt {
    std::unique_ptr<Stmt> var;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> increment;
    std::vector<std::unique_ptr<Stmt>> stmts;

    ForStmt(std::unique_ptr<Stmt> var, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> increment, std::vector<std::unique_ptr<Stmt>> stmts);

    void accept(StmtVisitor &visitor) override;
};

struct PrintStmt : public Stmt {
    std::optional<std::unique_ptr<Expr>> expr;
    bool newline;

    PrintStmt(std::optional<std::unique_ptr<Expr>> expr, bool newline);

    void accept(StmtVisitor &visitor) override;
};

struct ReturnStmt : public Stmt {
    std::optional<std::unique_ptr<Expr>> expr;

    ReturnStmt(std::optional<std::unique_ptr<Expr>> expr);

    void accept(StmtVisitor &visitor) override;
};

struct AssignStmt : public Stmt {
    Token name;
    std::unique_ptr<Expr> value;

    AssignStmt(Token name, std::unique_ptr<Expr> value);

    void accept(StmtVisitor &visitor) override;
};
