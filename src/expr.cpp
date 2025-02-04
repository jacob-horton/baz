#include "expr.h"
#include "expr_visitor.h"

// TODO: remove type({})
VarExpr::VarExpr(Token name) : name(name), type({}) {}
void VarExpr::accept(ExprVisitor &visitor) {
    visitor.visitVarExpr(this);
}

StructInitExpr::StructInitExpr(Token name, std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties) : name(name), properties(std::move(properties)) {}
void StructInitExpr::accept(ExprVisitor &visitor) {
    visitor.visitStructInitExpr(this);
}

BinaryExpr::BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}
void BinaryExpr::accept(ExprVisitor &visitor) {
    visitor.visitBinaryExpr(this);
}

LogicalBinaryExpr::LogicalBinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}
void LogicalBinaryExpr::accept(ExprVisitor &visitor) {
    visitor.visitLogicalBinaryExpr(this);
}

UnaryExpr::UnaryExpr(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
void UnaryExpr::accept(ExprVisitor &visitor) {
    visitor.visitUnaryExpr(this);
}

GetExpr::GetExpr(std::unique_ptr<Expr> value, Token name) : value(std::move(value)), name(name) {}
void GetExpr::accept(ExprVisitor &visitor) {
    visitor.visitGetExpr(this);
}

CallExpr::CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args)
    : callee(std::move(callee)), args(std::move(args)) {}
void CallExpr::accept(ExprVisitor &visitor) {
    visitor.visitCallExpr(this);
}

GroupingExpr::GroupingExpr(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}
void GroupingExpr::accept(ExprVisitor &visitor) {
    visitor.visitGroupingExpr(this);
}

LiteralExpr::LiteralExpr(Token literal) : literal(literal) {}
void LiteralExpr::accept(ExprVisitor &visitor) {
    visitor.visitLiteralExpr(this);
}
