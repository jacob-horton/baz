#include "type_checker.h"
#include "stmt.h"
#include "token.h"
#include <iostream>
#include <memory>

TypeChecker::TypeChecker() {}

// Expressions
void TypeChecker::visitVarExpr(VarExpr *expr) {
    // TODO: lookup type in environment
    this->result = std::make_unique<FloatType>();
}

void TypeChecker::visitStructInitExpr(StructInitExpr *expr) {}

void TypeChecker::visitBinaryExpr(BinaryExpr *expr) {
    switch (expr->op.t) {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH: {
            // TODO: only allow FLOAT, INT, STR

            // Check left and right are the same type
            expr->left->accept(*this);
            if (!this->result.has_value())
                return;

            auto left_t = std::move(this->result);

            expr->right->accept(*this);
            if (!this->result.has_value())
                return;

            auto right_t = std::move(this->result);
            if (left_t.value()->type_class != right_t.value()->type_class) {
                this->result = {};
                return;
            }

            this->result = std::move(left_t);
            return;
        }
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL: {
            // Check left and right are the same type, and numeric
            expr->left->accept(*this);
            if (!this->result.has_value())
                return;

            // If not numeric, we can't compare
            if (!(this->result.value()->type_class == TypeClass::INT || this->result.value()->type_class == TypeClass::FLOAT)) {
                this->result = {};
                return;
            }

            auto left_t = std::move(this->result);

            expr->right->accept(*this);
            if (!this->result.has_value())
                return;

            auto right_t = std::move(this->result);
            // TODO: For now only supporting comparing same type, but need to extend to support float < int for example
            if (left_t.value()->type_class != right_t.value()->type_class) {
                this->result = {};
                return;
            }

            this->result = std::move(left_t);
            return;
        }
        default:
            std::cerr << "[BUG] Operator type '" << get_token_type_str(expr->op.t) << "' not handled." << std::endl;
            exit(3);
    }
}

void TypeChecker::visitLogicalBinaryExpr(LogicalBinaryExpr *expr) {}

void TypeChecker::visitUnaryExpr(UnaryExpr *expr) {}

void TypeChecker::visitGetExpr(GetExpr *expr) {}

void TypeChecker::visitCallExpr(CallExpr *expr) {}

void TypeChecker::visitGroupingExpr(GroupingExpr *expr) {}

void TypeChecker::visitLiteralExpr(LiteralExpr *expr) {
    // TODO: "this" and user defined types
    switch (expr->literal.t) {
        case TokenType::TRUE:
        case TokenType::FALSE:
            this->result = std::make_unique<BoolType>();
            break;
        case TokenType::NULL_VAL:
            this->result = std::make_unique<NullType>();
            break;
        case TokenType::INT_VAL:
            this->result = std::make_unique<IntType>();
            break;
        case TokenType::FLOAT_VAL:
            this->result = std::make_unique<FloatType>();
            break;
        case TokenType::STR_VAL:
            this->result = std::make_unique<StrType>();
            break;
        default:
            std::cerr << "[BUG] Token type '" << get_token_type_str(expr->literal.t) << "' not handled." << std::endl;
            exit(3);
    }

    this->success = true;
}

// Statments
void TypeChecker::visitFunDeclStmt(FunDeclStmt *stmt) {}

void TypeChecker::visitStructDeclStmt(StructDeclStmt *stmt) {}

void TypeChecker::visitEnumDeclStmt(EnumDeclStmt *stmt) {}

void TypeChecker::visitVariableDeclStmt(VariableDeclStmt *stmt) {}

void TypeChecker::visitExprStmt(ExprStmt *stmt) {}

void TypeChecker::visitBlockStmt(BlockStmt *stmt) {}

void TypeChecker::visitIfStmt(IfStmt *stmt) {}

void TypeChecker::visitMatchStmt(MatchStmt *stmt) {}

void TypeChecker::visitWhileStmt(WhileStmt *stmt) {}

void TypeChecker::visitForStmt(ForStmt *stmt) {}

void TypeChecker::visitPrintStmt(PrintStmt *stmt) {}

void TypeChecker::visitReturnStmt(ReturnStmt *stmt) {}

void TypeChecker::visitAssignStmt(AssignStmt *stmt) {}
