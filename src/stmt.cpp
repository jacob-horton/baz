#include "stmt.h"
#include "stmt_visitor.h"

FunDeclStmt::FunDeclStmt(Token name, std::vector<TypedVar> params, Token return_type, std::vector<std::unique_ptr<Stmt>> body)
    : name(name), params(params), return_type(return_type), body(std::move(body)) {}
void FunDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visitFunDeclStmt(this);
}

StructDeclStmt::StructDeclStmt(Token name, std::vector<TypedVar> properties, std::vector<std::unique_ptr<FunDeclStmt>> methods)
    : name(name), properties(properties), methods(std::move(methods)) {}
void StructDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visitStructDeclStmt(this);
}

EnumDeclStmt::EnumDeclStmt(Token name, std::vector<EnumVariant> variants, std::vector<std::unique_ptr<FunDeclStmt>> methods)
    : name(name), variants(variants), methods(std::move(methods)) {}
void EnumDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visitEnumDeclStmt(this);
}

VariableDeclStmt::VariableDeclStmt(TypedVar name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}
void VariableDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visitVariableDeclStmt(this);
}

ExprStmt::ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
void ExprStmt::accept(StmtVisitor &visitor) {
    visitor.visitExprStmt(this);
}

BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}
void BlockStmt::accept(StmtVisitor &visitor) {
    visitor.visitBlockStmt(this);
}

IfStmt::IfStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> true_block, std::optional<std::vector<std::unique_ptr<Stmt>>> false_block)
    : condition(std::move(condition)), true_block(std::move(true_block)), false_block(std::move(false_block)) {}
void IfStmt::accept(StmtVisitor &visitor) {
    visitor.visitIfStmt(this);
}

MatchBranch::MatchBranch(std::unique_ptr<Expr> pattern, std::vector<std::unique_ptr<Stmt>> body) : pattern(std::move(pattern)), body(std::move(body)) {}

MatchStmt::MatchStmt(std::unique_ptr<Expr> target, std::vector<MatchBranch> branches)
    : target(std::move(target)), branches(std::move(branches)) {}
void MatchStmt::accept(StmtVisitor &visitor) {
    visitor.visitMatchStmt(this);
}

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts) : condition(std::move(condition)), stmts(std::move(stmts)) {}
void WhileStmt::accept(StmtVisitor &visitor) {
    visitor.visitWhileStmt(this);
}

ForStmt::ForStmt(std::unique_ptr<Stmt> var, std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> increment, std::vector<std::unique_ptr<Stmt>> stmts)
    : var(std::move(var)), condition(std::move(condition)), increment(std::move(increment)), stmts(std::move(stmts)) {}
void ForStmt::accept(StmtVisitor &visitor) {
    visitor.visitForStmt(this);
}

PrintStmt::PrintStmt(std::optional<std::unique_ptr<Expr>> expr, bool newline) : expr(std::move(expr)), newline(newline) {}
void PrintStmt::accept(StmtVisitor &visitor) {
    visitor.visitPrintStmt(this);
}

ReturnStmt::ReturnStmt(std::optional<std::unique_ptr<Expr>> expr) : expr(std::move(expr)) {}
void ReturnStmt::accept(StmtVisitor &visitor) {
    visitor.visitReturnStmt(this);
}

AssignStmt::AssignStmt(Token name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)) {}
void AssignStmt::accept(StmtVisitor &visitor) {
    visitor.visitAssignStmt(this);
}
