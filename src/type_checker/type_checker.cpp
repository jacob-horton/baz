#include "type_checker.h"
#include "type.h"

#include <algorithm>
#include <iostream>
#include <memory>

TypeChecker::TypeChecker(std::map<std::string, std::shared_ptr<Type>> type_env) : type_env(type_env) {}

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

void TypeChecker::visit_struct_init_expr(StructInitExpr *expr) {
    if (auto t = std::dynamic_pointer_cast<StructType>(expr->type)) {
        if (expr->properties.size() != t->props.size())
            this->error(expr->name, "Expected " + std::to_string(t->props.size()) + " arguments, received " + std::to_string(expr->properties.size()) + ".");

        // Loop through in order of declared type props
        for (auto &prop : t->props) {
            auto name = std::get<0>(prop).lexeme;

            // Find corresponding property
            // TODO: use hashmap or lookup function
            auto p = std::find_if(expr->properties.begin(), expr->properties.end(), [name](const auto &t) {
                return std::get<0>(t).lexeme == name;
            });

            if (p == expr->properties.end())
                this->error(expr->name, "Missing property " + name + " from constructor.");
        }
    } else {
        this->error(expr->name, "Cannot initialise non-struct.");
    }

    this->result = expr->type;
}

bool TypeChecker::is_numeric(Type *t) {
    return t->can_coerce_to(this->type_env["int"]) || t->can_coerce_to(this->type_env["float"]);
}

void TypeChecker::visit_binary_expr(BinaryExpr *expr) {
    switch (expr->op.t) {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH: {
            // Check left and right are the same type
            expr->left->accept(*this);
            auto left_t = this->result;

            // If not numeric or string, we can't operate
            if (!(is_numeric(left_t.get()) || left_t.get()->type_class == TypeClass::STR))
                this->error(expr->op, "Operator can only be used on numeric or string types.");

            expr->right->accept(*this);
            auto right_t = this->result;

            if (!right_t->can_coerce_to(left_t))
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

            if (!right_t->can_coerce_to(left_t))
                this->error(expr->op, "Operands must be the same type, or coercible to the same type.");

            this->result = this->type_env["bool"];
            return;
        }
        case TokenType::QUESTION_QUESTION: {
            // TODO: handle null types - make sure it narrows to non-nullable type
            // Check left and right are the same type
            expr->left->accept(*this);
            auto left_t = this->result;

            expr->right->accept(*this);
            auto right_t = this->result;

            if (!right_t->can_coerce_to(left_t))
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

    if (!left_t->can_coerce_to(this->type_env["bool"]))
        this->error(expr->op, "Operator can only be used on boolean types.");

    expr->right->accept(*this);
    auto right_t = this->result;

    if (left_t != right_t)
        this->error(expr->op, "Operands must be the same type, or coercible to the same type.");
}

void TypeChecker::visit_unary_expr(UnaryExpr *expr) {
    expr->right->accept(*this);

    switch (expr->op.t) {
        case TokenType::BANG:
            // If not boolean, we can't invert
            if (!this->result->can_coerce_to(this->type_env["bool"]))
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

void TypeChecker::visit_get_expr(GetExpr *expr) {
    expr->object->accept(*this);
}

void TypeChecker::visit_enum_init_expr(EnumInitExpr *expr) {
    // Check payload if there is one
    if (expr->payload.has_value()) {
        expr->payload.value()->accept(*this);

        // Cast to enum type (should always be enum type)
        if (auto t = std::dynamic_pointer_cast<EnumType>(expr->type)) {
            // Check type of payload for this variant
            auto payload_type_symbol = t->get_variant_payload_type(expr->variant.lexeme)->lexeme;
            auto payload_type = this->type_env[payload_type_symbol];

            if (!this->result->can_coerce_to(payload_type)) {
                this->error(expr->variant, "Payload of enum cannot be coerced to a valid type.");
            }
        } else {
            std::cerr << "[BUG] Enum doesn't have enum type" << std::endl;
            exit(3);
        }
    }

    this->result = expr->type;
}

void TypeChecker::visit_call_expr(CallExpr *expr) {
    expr->callee->accept(*this);

    if (auto t = std::dynamic_pointer_cast<FunctionType>(expr->callee->type)) {
        if (expr->args.size() != t->params.size()) {
            this->error(expr->bracket, "Expected " + std::to_string(t->params.size()) + " arguments, received " + std::to_string(expr->args.size()) + ".");
        }

        for (int i = 0; i < expr->args.size(); i++) {
            if (expr->args[i]->type != this->type_env[std::get<1>(t->params[i]).lexeme]) {
                this->error(expr->bracket, "Invalid type passed to function.");
            }
        }
    } else {
        this->error(expr->bracket, "Cannot call non-function.");
    }

    this->result = expr->type;
}

void TypeChecker::visit_grouping_expr(GroupingExpr *expr) {
    expr->expr->accept(*this);
}

void TypeChecker::visit_literal_expr(LiteralExpr *expr) {
    this->result = expr->type;
}

// Statements
void TypeChecker::visit_fun_decl_stmt(FunDeclStmt *fun) {
    this->check(fun->body);
}

void TypeChecker::visit_enum_method_decl_stmt(EnumMethodDeclStmt *fun) {
    fun->fun_definition->accept(*this);
}

void TypeChecker::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    for (auto &method : stmt->methods) {
        method->accept(*this);
    }
}

void TypeChecker::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    for (auto &method : stmt->methods) {
        method->accept(*this);
    }
}

void TypeChecker::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    stmt->initialiser->accept(*this);
    if (!this->result->can_coerce_to(this->type_env[stmt->name.type.lexeme])) {
        this->error(stmt->name.name, "Cannot assign a type '" + this->result->to_string() + "' to variable of type '" + this->type_env[stmt->name.type.lexeme]->to_string() + "'.");
    }
}

void TypeChecker::visit_expr_stmt(ExprStmt *stmt) {
    stmt->expr->accept(*this);
}

void TypeChecker::visit_block_stmt(BlockStmt *stmt) {
    this->check(stmt->stmts);
}

void TypeChecker::visit_if_stmt(IfStmt *stmt) {
    stmt->condition->accept(*this);
    if (!this->result->can_coerce_to(this->type_env["bool"])) {
        this->error(stmt->keyword, "If condition must be a boolean value.");
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

void TypeChecker::visit_match_stmt(MatchStmt *stmt) {
    // std::cerr << "Unimplemented" << std::endl;
    // exit(3);
}

void TypeChecker::visit_while_stmt(WhileStmt *stmt) {
    stmt->condition->accept(*this);
    if (!this->result->can_coerce_to(this->type_env["bool"])) {
        this->error(stmt->keyword, "While condition must be a boolean value.");
    }

    this->check(stmt->stmts);
}

void TypeChecker::visit_for_stmt(ForStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeChecker::visit_print_stmt(PrintStmt *stmt) {
    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);
}

void TypeChecker::visit_return_stmt(ReturnStmt *stmt) {
    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);
}

void TypeChecker::visit_assign_stmt(AssignStmt *stmt) {
    stmt->value->accept(*this);

    if (!this->result->can_coerce_to(stmt->target_type)) {
        this->error(stmt->name, "Cannot assign a different type to this variable.");
    }
}
