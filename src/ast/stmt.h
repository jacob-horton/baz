#pragma once

#include "../scanner/token.h"
#include "enum_variant.h"
#include "expr.h"
#include "typed_var.h"

#include <optional>
#include <ostream>
#include <variant>
#include <vector>

// Forward declaration - actual implementation will import the visitor
class StmtVisitor;

struct Stmt {
    virtual void accept(StmtVisitor &visitor) = 0;

    virtual ~Stmt() = default;
};

enum FunType {
    FUNCTION,
    METHOD,
};

// TODO: all decl stmts to have type set by resolver
struct FunDeclStmt : public Stmt {
    Token name;
    Token return_type;
    bool return_type_optional;
    std::vector<TypedVar> params;
    std::vector<std::unique_ptr<Stmt>> body;
    FunType fun_type;

    FunDeclStmt(Token name, std::vector<TypedVar> params, Token return_type, bool return_type_optional, std::vector<std::unique_ptr<Stmt>> body, FunType fun_type);

    void accept(StmtVisitor &visitor) override;
};

// TODO: all decl stmts to have type set by resolver
struct EnumMethodDeclStmt : public Stmt {
    Token enum_name;
    std::unique_ptr<FunDeclStmt> fun_definition;

    EnumMethodDeclStmt(std::unique_ptr<FunDeclStmt> fun_definition, Token enum_name);

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
    std::vector<std::unique_ptr<EnumMethodDeclStmt>> methods;

    EnumDeclStmt(Token name, std::vector<EnumVariant> variants, std::vector<std::unique_ptr<EnumMethodDeclStmt>> methods);

    void accept(StmtVisitor &visitor) override;
};

struct VariableDeclStmt : public Stmt {
    TypedVar name;
    std::unique_ptr<Expr> initialiser;

    VariableDeclStmt(TypedVar name, std::unique_ptr<Expr> initialiser);

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
    Token keyword;
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> true_block;
    std::optional<std::vector<std::unique_ptr<Stmt>>> false_block;

    IfStmt(Token keyword, std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> true_block, std::optional<std::vector<std::unique_ptr<Stmt>>> false_block);

    void accept(StmtVisitor &visitor) override;
};

struct EnumPattern {
    Token enum_type;
    Token enum_variant;

    // NOTE: if the enum variant has a payload, this is required
    std::optional<std::unique_ptr<VarExpr>> bound_variable;

    EnumPattern(Token enum_type, Token enum_variant, std::optional<std::unique_ptr<VarExpr>> bound_variable);
};

struct NullPattern {};

using MatchPattern = std::variant<EnumPattern, NullPattern>;

struct MatchBranch {
    MatchPattern pattern;
    std::vector<std::unique_ptr<Stmt>> body;

    MatchBranch(MatchPattern pattern, std::vector<std::unique_ptr<Stmt>> body);
};

struct MatchStmt : public Stmt {
    std::unique_ptr<Expr> target;
    std::vector<MatchBranch> branches;
    Token bracket;

    MatchStmt(std::unique_ptr<Expr> target, std::vector<MatchBranch> branches, Token bracket);

    void accept(StmtVisitor &visitor) override;
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> stmts;

    Token keyword;

    WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts, Token keyword);

    void accept(StmtVisitor &visitor) override;
};

struct AssignStmt : public Stmt {
    Token name;
    std::unique_ptr<Expr> value;
    bool semicolon;

    // NOTE: this gets set when resolving
    std::shared_ptr<Type> target_type;

    AssignStmt(Token name, std::unique_ptr<Expr> value);

    void accept(StmtVisitor &visitor) override;
};

struct ForStmt : public Stmt {
    std::unique_ptr<VariableDeclStmt> var;
    std::unique_ptr<ExprStmt> condition;
    std::unique_ptr<AssignStmt> increment;
    std::vector<std::unique_ptr<Stmt>> stmts;

    ForStmt(std::unique_ptr<VariableDeclStmt> var, std::unique_ptr<ExprStmt> condition, std::unique_ptr<AssignStmt> increment, std::vector<std::unique_ptr<Stmt>> stmts);

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
