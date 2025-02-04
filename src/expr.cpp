#include "expr.h"
#include "expr_visitor.h"
#include <iostream>

VarExpr::VarExpr(Token name) : name(name) {}
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

std::unique_ptr<Type> LiteralExpr::get_type() {
    // TODO: "this" and user defined types
    switch (this->literal.t) {
        case TokenType::TRUE:
        case TokenType::FALSE:
            return std::make_unique<BoolType>();
        case TokenType::NULL_VAL:
            return std::make_unique<NullType>();
        case TokenType::INT_VAL:
            return std::make_unique<IntType>();
        case TokenType::FLOAT_VAL:
            return std::make_unique<FloatType>();
        case TokenType::STR_VAL:
            return std::make_unique<StrType>();
        default:
            std::cerr << "[BUG] Token type '" << get_token_type_str(this->literal.t) << "' not handled." << std::endl;
            exit(3);
    }
}
