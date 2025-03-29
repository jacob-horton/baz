#include "stmt.h"
#include "stmt_visitor.h"
#include <memory>

FunDeclStmt::FunDeclStmt(Token name, std::vector<TypedVar> params, Token return_type, bool return_type_optional, std::vector<std::unique_ptr<Stmt>> body, FunType fun_type)
    : name(name), params(params), return_type(return_type), return_type_optional(return_type_optional), body(std::move(body)), fun_type(fun_type) {}
void FunDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_fun_decl_stmt(this);
}

EnumMethodDeclStmt::EnumMethodDeclStmt(std::unique_ptr<FunDeclStmt> fun_definition, Token enum_name)
    : fun_definition(std::move(fun_definition)), enum_name(enum_name) {}
void EnumMethodDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_enum_method_decl_stmt(this);
}

StructDeclStmt::StructDeclStmt(Token name, std::vector<TypedVar> properties, std::vector<std::unique_ptr<FunDeclStmt>> methods)
    : name(name), properties(properties), methods(std::move(methods)) {}
void StructDeclStmt::accept(StmtVisitor &visitor) {
    visitor.visit_struct_decl_stmt(this);
}

EnumDeclStmt::EnumDeclStmt(Token name, std::vector<EnumVariant> variants, std::vector<std::unique_ptr<EnumMethodDeclStmt>> methods)
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

EnumPattern::EnumPattern(Token enum_type, Token enum_variant, std::optional<std::unique_ptr<VarExpr>> bound_variable) : enum_type(enum_type), enum_variant(enum_variant), bound_variable(std::move(bound_variable)) {}

MatchBranch::MatchBranch(MatchPattern pattern, std::vector<std::unique_ptr<Stmt>> body) : pattern(std::move(pattern)), body(std::move(body)) {}

MatchStmt::MatchStmt(std::unique_ptr<Expr> target, std::vector<MatchBranch> branches, Token keyword)
    : target(std::move(target)), branches(std::move(branches)), keyword(keyword) {}
void MatchStmt::accept(StmtVisitor &visitor) {
    visitor.visit_match_stmt(this);
}

WhileStmt::WhileStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Stmt>> stmts, Token keyword) : condition(std::move(condition)), stmts(std::move(stmts)), keyword(keyword) {}
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

ReturnStmt::ReturnStmt(std::optional<std::unique_ptr<Expr>> expr, Token keyword) : expr(std::move(expr)), keyword(keyword) {}
void ReturnStmt::accept(StmtVisitor &visitor) {
    visitor.visit_return_stmt(this);
}

AssignStmt::AssignStmt(Token name, std::unique_ptr<Expr> value) : name(name), value(std::move(value)), semicolon(true) {}
void AssignStmt::accept(StmtVisitor &visitor) {
    visitor.visit_assign_stmt(this);
}

SetStmt::SetStmt(std::unique_ptr<Expr> object, Token name, std::unique_ptr<Expr> value) : object(std::move(object)), name(name), value(std::move(value)) {}
void SetStmt::accept(StmtVisitor &visitor) {
    visitor.visit_set_stmt(this);
}
