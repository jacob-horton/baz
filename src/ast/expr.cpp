#include "expr.h"
#include "expr_visitor.h"

#include <memory>

VarExpr::VarExpr(Token name) : name(name) {}
void VarExpr::accept(ExprVisitor &visitor) {
    visitor.visit_var_expr(this);
}

StructInitExpr::StructInitExpr(Token name, std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties) : name(name), properties(std::move(properties)) {}
void StructInitExpr::accept(ExprVisitor &visitor) {
    visitor.visit_struct_init_expr(this);
}

EnumInitExpr::EnumInitExpr(Token variant, std::unique_ptr<VarExpr> enum_namespace, std::optional<std::unique_ptr<Expr>> payload) : variant(variant), enum_namespace(std::move(enum_namespace)), payload(std::move(payload)) {}
void EnumInitExpr::accept(ExprVisitor &visitor) {
    visitor.visit_enum_init_expr(this);
}

BinaryExpr::BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}
void BinaryExpr::accept(ExprVisitor &visitor) {
    visitor.visit_binary_expr(this);
}

LogicalBinaryExpr::LogicalBinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}
void LogicalBinaryExpr::accept(ExprVisitor &visitor) {
    visitor.visit_logical_binary_expr(this);
}

UnaryExpr::UnaryExpr(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
void UnaryExpr::accept(ExprVisitor &visitor) {
    visitor.visit_unary_expr(this);
}

GetExpr::GetExpr(std::unique_ptr<Expr> value, Token name, bool optional) : object(std::move(value)), name(name), optional(optional) {}
void GetExpr::accept(ExprVisitor &visitor) {
    visitor.visit_get_expr(this);
}

CallExpr::CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args, Token bracket)
    : callee(std::move(callee)), args(std::move(args)), bracket(bracket) {}
void CallExpr::accept(ExprVisitor &visitor) {
    visitor.visit_call_expr(this);
}

GroupingExpr::GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
void GroupingExpr::accept(ExprVisitor &visitor) {
    visitor.visit_grouping_expr(this);
}

LiteralExpr::LiteralExpr(Token literal) : literal(literal) {}
void LiteralExpr::accept(ExprVisitor &visitor) {
    visitor.visit_literal_expr(this);
}
