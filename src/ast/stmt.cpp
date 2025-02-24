#include "stmt_visitor.h"
#include <memory>

FunDeclStmt::FunDeclStmt(Token name, std::vector<TypedVar> params, Token return_type, std::vector<std::unique_ptr<Stmt>> body, FunType fun_type)
    : name(name), params(params), return_type(return_type), body(std::move(body)), fun_type(fun_type) {}
void FunDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_fun_decl_stmt(this);
}

// TODO: do this in resolver
StructDeclStmt::StructDeclStmt(Token name, std::vector<TypedVar> properties, std::vector<std::unique_ptr<FunDeclStmt>> methods)
    : name(name), properties(properties), methods(std::move(methods)) {
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> props;
    for (auto &prop : properties) {
        props.push_back(std::make_tuple(prop.name, prop.get_type()));
    }
    this->type = std::make_unique<StructType>(name, props);
}
void StructDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_struct_decl_stmt(this);
}
std::shared_ptr<StructType> StructDeclStmt::get_type() {
    return this->type;
}

EnumDeclStmt::EnumDeclStmt(Token name, std::vector<EnumVariant> variants, std::vector<std::unique_ptr<FunDeclStmt>> methods)
    : name(name), variants(variants), methods(std::move(methods)) {}
void EnumDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_enum_decl_stmt(this);
}

VariableDeclStmt::VariableDeclStmt(TypedVar name, std::unique_ptr<Expr> initialiser) : name(name), initialiser(std::move(initialiser)) {}
void VariableDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_variable_decl_stmt(this);
}

ExprStmt::ExprStmt(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
void ExprStmt::accept(StmtVisitor &visitor) {
    visitor.visit_expr_stmt(this);
}

BlockStmt::BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : stmts(std::move(stmts)) {}
void BlockStmt::accept(StmtVisitor &visitor) {
    visitor.visit_block_stmt(this);
}

IfStmt::IfStmt(Token keyword, std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> true_block, std::optional<std::vector<std::unique_ptr<Stmt>>> false_block)
    : keyword(keyword), condition(std::move(condition)), true_block(std::move(true_block)), false_block(std::move(false_block)) {}
void IfStmt::accept(StmtVisitor &visitor) {
    visitor.visit_if_stmt(this);
}

MatchBranch::MatchBranch(std::unique_ptr<Expr> pattern, std::vector<std::unique_ptr<Stmt>> body) : pattern(std::move(pattern)), body(std::move(body)) {}

MatchStmt::MatchStmt(std::unique_ptr<Expr> target, std::vector<MatchBranch> branches)
    : target(std::move(target)), branches(std::move(branches)) {}
void MatchStmt::accept(StmtVisitor &visitor) {
    visitor.visit_match_stmt(this);
}

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts) : condition(std::move(condition)), stmts(std::move(stmts)) {}
void WhileStmt::accept(StmtVisitor &visitor) {
    visitor.visit_while_stmt(this);
}

ForStmt::ForStmt(std::unique_ptr<VariableDeclStmt> var, std::unique_ptr<ExprStmt> condition, std::unique_ptr<AssignStmt> increment, std::vector<std::unique_ptr<Stmt>> stmts)
    : var(std::move(var)), condition(std::move(condition)), increment(std::move(increment)), stmts(std::move(stmts)) {}
void ForStmt::accept(StmtVisitor &visitor) {
    visitor.visit_for_stmt(this);
}

PrintStmt::PrintStmt(std::optional<std::unique_ptr<Expr>> expr, bool newline) : expr(std::move(expr)), newline(newline) {}
void PrintStmt::accept(StmtVisitor &visitor) {
    visitor.visit_print_stmt(this);
}

ReturnStmt::ReturnStmt(std::optional<std::unique_ptr<Expr>> expr) : expr(std::move(expr)) {}
void ReturnStmt::accept(StmtVisitor &visitor) {
    visitor.visit_return_stmt(this);
}

AssignStmt::AssignStmt(Token name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)), semicolon(true) {}
void AssignStmt::accept(StmtVisitor &visitor) {
    visitor.visit_assign_stmt(this);
}
