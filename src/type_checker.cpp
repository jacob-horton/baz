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
void TypeChecker::visit_var_expr(VarExpr *expr) {
    this->result = expr->type;
}

void TypeChecker::visit_struct_init_expr(StructInitExpr *expr) {}

bool is_numeric(Type *t) {
    return t->can_coerce_to(TypeClass::INT) || t->can_coerce_to(TypeClass::FLOAT);
}

void TypeChecker::visit_binary_expr(BinaryExpr *expr) {
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

void TypeChecker::visit_logical_binary_expr(LogicalBinaryExpr *expr) {
    expr->left->accept(*this);
    auto left_t = this->result;

    if (!left_t->can_coerce_to(TypeClass::BOOL))
        this->error(expr->op, "Operator can only be used on boolean types.");

    expr->right->accept(*this);
    auto right_t = this->result;

    if (*left_t != *right_t)
        this->error(expr->op, "Operands must be the same type, or coercible to the same type.");
}

void TypeChecker::visit_unary_expr(UnaryExpr *expr) {
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

void TypeChecker::visit_get_expr(GetExpr *expr) {}

void TypeChecker::visit_call_expr(CallExpr *expr) {}

void TypeChecker::visit_grouping_expr(GroupingExpr *expr) {
    expr->expr->accept(*this);
}

void TypeChecker::visit_literal_expr(LiteralExpr *expr) {
    this->result = expr->get_type();
}

// Statements
void TypeChecker::visit_fun_decl_stmt(FunDeclStmt *fun) {
    for (auto &stmt : fun->body) {
        stmt->accept(*this);
    }
}

void TypeChecker::visit_struct_decl_stmt(StructDeclStmt *stmt) {}

void TypeChecker::visit_enum_decl_stmt(EnumDeclStmt *stmt) {}

void TypeChecker::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    stmt->initialiser->accept(*this);
    if (*stmt->name.get_type() != *this->result) {
        this->error(stmt->name.name, "Cannot assign a type '" + this->result->to_string() + "' to variable of type '" + stmt->name.get_type()->to_string() + "'.");
    }
}

void TypeChecker::visit_expr_stmt(ExprStmt *stmt) {
    stmt->expr->accept(*this);
}

void TypeChecker::visit_block_stmt(BlockStmt *stmt) {}

void TypeChecker::visit_if_stmt(IfStmt *stmt) {
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

void TypeChecker::visit_match_stmt(MatchStmt *stmt) {}

void TypeChecker::visit_while_stmt(WhileStmt *stmt) {}

void TypeChecker::visit_for_stmt(ForStmt *stmt) {}

void TypeChecker::visit_print_stmt(PrintStmt *stmt) {}

void TypeChecker::visit_return_stmt(ReturnStmt *stmt) {}

void TypeChecker::visit_assign_stmt(AssignStmt *stmt) {
    stmt->value->accept(*this);

    if (*this->result != *stmt->target_type) {
        // TODO: use equal token?
        this->error(stmt->name, "Cannot assign a different type to this variable.");
    }
}
