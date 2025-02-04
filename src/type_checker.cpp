#include "type_checker.h"
#include "stmt.h"
#include "token.h"
#include "type.h"
#include <iostream>
#include <memory>

TypeChecker::TypeChecker() {}

void TypeChecker::check(std::vector<std::unique_ptr<Stmt>> &stmts) {
    for (auto &stmt : stmts) {
        stmt->accept(*this);
    }
}

void TypeChecker::error(Token error_token, std::string message) {
    std::string where = "end";
    if (error_token.t != TokenType::EOF_)
        where = "'" + error_token.lexeme + "'";

    std::cerr << "[line " << error_token.line << "] Type error at " << where << ": " << message << std::endl;

    throw TypeCheckerError::OP_ON_INCOMPATIBLE_TYPES;
}

// Expressions
void TypeChecker::visitVarExpr(VarExpr *expr) {
    this->result = expr->type;
}

void TypeChecker::visitStructInitExpr(StructInitExpr *expr) {}

bool is_numeric(Type *t) {
    return t->can_coerce_to(TypeClass::INT) || t->can_coerce_to(TypeClass::FLOAT);
}

void TypeChecker::visitBinaryExpr(BinaryExpr *expr) {
    switch (expr->op.t) {
        // TODO: concatenating strings for PLUS
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH: {
            // Check left and right are the same type
            expr->left->accept(*this);
            auto left_t = this->result;

            // If not numeric, we can't operate
            if (!is_numeric(left_t.get()))
                this->error(expr->op, "Operator can only be used on numeric types.");

            expr->right->accept(*this);
            auto right_t = this->result;

            if (*left_t != *right_t)
                this->error(expr->op, "Operands must be the same type, or coercible to the same type.");

            return;
        }
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL: {
            // Check left and right are the same type, and numeric
            expr->left->accept(*this);
            auto left_t = this->result;

            // If not numeric, we can't compare
            if (!is_numeric(left_t.get()))
                this->error(expr->op, "Operator can only be used on numeric types.");

            expr->right->accept(*this);
            auto right_t = this->result;

            // TODO: For now only supporting comparing same type, but need to extend to support float < int for example
            if (*left_t != *right_t)
                this->error(expr->op, "Operands must be the same type, or coercible to the same type.");

            this->result = left_t;
            return;
        }
        default:
            std::cerr << "[BUG] Binary operator type '" << get_token_type_str(expr->op.t) << "' not handled." << std::endl;
            exit(3);
    }
}

void TypeChecker::visitLogicalBinaryExpr(LogicalBinaryExpr *expr) {
    expr->left->accept(*this);
    auto left_t = this->result;

    if (!left_t->can_coerce_to(TypeClass::BOOL))
        this->error(expr->op, "Operator can only be used on boolean types.");

    expr->right->accept(*this);
    auto right_t = this->result;

    if (*left_t != *right_t)
        this->error(expr->op, "Operands must be the same type, or coercible to the same type.");
}

void TypeChecker::visitUnaryExpr(UnaryExpr *expr) {
    expr->right->accept(*this);

    switch (expr->op.t) {
        case TokenType::BANG:
            // If not boolean, we can't invert
            if (!this->result->can_coerce_to(TypeClass::BOOL))
                this->error(expr->op, "Operator can only be used on a boolean type.");
            break;
        case TokenType::MINUS:
            // If not numeric, we can't invert
            if (!is_numeric(this->result.get()))
                this->error(expr->op, "Operator can only be used on numeric types.");
            break;
        default:
            std::cerr << "[BUG] Unary operator type '" << get_token_type_str(expr->op.t) << "' not handled." << std::endl;
            exit(3);
    }
}

void TypeChecker::visitGetExpr(GetExpr *expr) {}

void TypeChecker::visitCallExpr(CallExpr *expr) {}

void TypeChecker::visitGroupingExpr(GroupingExpr *expr) {
    expr->expr->accept(*this);
}

void TypeChecker::visitLiteralExpr(LiteralExpr *expr) {
    this->result = expr->get_type();
}

// Statements
void TypeChecker::visitFunDeclStmt(FunDeclStmt *fun) {
    for (auto &stmt : fun->body) {
        stmt->accept(*this);
    }
}

void TypeChecker::visitStructDeclStmt(StructDeclStmt *stmt) {}

void TypeChecker::visitEnumDeclStmt(EnumDeclStmt *stmt) {}

void TypeChecker::visitVariableDeclStmt(VariableDeclStmt *stmt) {
    stmt->initialiser->accept(*this);
    if (*stmt->name.get_type() != *this->result) {
        this->error(stmt->name.name, "Cannot assign a type '" + this->result->to_string() + "' to variable of type '" + stmt->name.get_type()->to_string() + "'.");
    }
}

void TypeChecker::visitExprStmt(ExprStmt *stmt) {
    stmt->expr->accept(*this);
}

void TypeChecker::visitBlockStmt(BlockStmt *stmt) {}

void TypeChecker::visitIfStmt(IfStmt *stmt) {
    stmt->condition->accept(*this);
    if (!this->result->can_coerce_to(TypeClass::BOOL)) {
        // TODO: store if token for error reporting
        this->error(Token{}, "If condition must be a boolean.");
    }

    for (auto &line : stmt->true_block) {
        line->accept(*this);
    }

    if (stmt->false_block.has_value()) {
        for (auto &line : stmt->false_block.value()) {
            line->accept(*this);
        }
    }
}

void TypeChecker::visitMatchStmt(MatchStmt *stmt) {}

void TypeChecker::visitWhileStmt(WhileStmt *stmt) {}

void TypeChecker::visitForStmt(ForStmt *stmt) {}

void TypeChecker::visitPrintStmt(PrintStmt *stmt) {}

void TypeChecker::visitReturnStmt(ReturnStmt *stmt) {}

void TypeChecker::visitAssignStmt(AssignStmt *stmt) {
    stmt->value->accept(*this);

    if (*this->result != *stmt->target_type) {
        // TODO: use equal token?
        this->error(stmt->name, "Cannot assign a different type to this variable.");
    }
}
