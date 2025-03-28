#include "type_checker.h"
#include "type.h"

#include <algorithm>
#include <iostream>
#include <memory>

bool can_coerce_to(std::shared_ptr<Type> from, bool from_optional, std::shared_ptr<Type> to, bool to_optional) {
    if (from_optional && !to_optional) {
        return false;
    }

    if (to_optional && from->type_class == TypeClass::NULL_)
        return true;

    return from->can_coerce_to(to);
}

TypeChecker::TypeChecker(std::map<std::string, std::shared_ptr<Type>> type_env) : type_env(type_env), result(TypeInfo(nullptr, false)) {}

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
    this->result = expr->type_info;
}

void TypeChecker::visit_struct_init_expr(StructInitExpr *expr) {
    if (auto t = std::dynamic_pointer_cast<StructType>(expr->type_info.type)) {
        if (expr->properties.size() != t->props.size())
            this->error(expr->name, "Expected " + std::to_string(t->props.size()) + " arguments, received " + std::to_string(expr->properties.size()) + ".");

        // Loop through in order of declared type props
        for (auto &prop : t->props) {
            auto name = prop.name.lexeme;

            // Find corresponding property
            // TODO: use hashmap or lookup function
            auto p = std::find_if(expr->properties.begin(), expr->properties.end(), [name](const auto &p) {
                return std::get<0>(p).lexeme == name;
            });

            if (p == expr->properties.end())
                this->error(expr->name, "Missing property " + name + " from constructor.");

            auto from = std::get<1>(*p)->type_info;
            auto to = this->type_env[prop.type.lexeme];
            if (!can_coerce_to(from.type, from.optional, to, prop.is_optional)) {
                this->error(std::get<0>(*p), "Cannot assign a type '" + from.type->to_string() + (from.optional && from.type->type_class != TypeClass::NULL_ ? "?" : "") + "' to variable of type '" + to->to_string() + (prop.is_optional ? "?" : "") + "'.");
            }
        }
    } else {
        this->error(expr->name, "Cannot initialise non-struct.");
    }

    this->result = expr->type_info;
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
            if (left_t.optional || !(is_numeric(left_t.type.get()) || left_t.type.get()->type_class == TypeClass::STR))
                this->error(expr->op, "Operator can only be used on numeric or string types.");

            expr->right->accept(*this);
            auto right_t = this->result;

            if (!can_coerce_to(right_t.type, right_t.optional, left_t.type, left_t.optional))
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
            if (left_t.optional || !is_numeric(left_t.type.get()))
                this->error(expr->op, "Operator can only be used on numeric types.");

            expr->right->accept(*this);
            auto right_t = this->result;

            if (!can_coerce_to(right_t.type, right_t.optional, left_t.type, left_t.optional))
                this->error(expr->op, "Operands must be the same type, or coercible to the same type.");

            this->result = TypeInfo(this->type_env["bool"], false);
            return;
        }
        case TokenType::QUESTION_QUESTION: {
            // Check left and right are the same type
            expr->left->accept(*this);
            auto left_t = this->result;

            expr->right->accept(*this);
            auto right_t = this->result;

            if (!left_t.optional)
                this->error(expr->op, "Cannot use '\?\?' operator on a non-nullable type.");

            // Has to coerce to non-nullable version of left type
            if (!can_coerce_to(right_t.type, right_t.optional, left_t.type, false))
                this->error(expr->op, "Operands must be the same type, or coercible to the same type.");

            // Always go to non-optional form of left type
            this->result = left_t;
            this->result.optional = false;
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

    auto from = this->result;
    auto to = this->type_env["bool"];
    if (!can_coerce_to(from.type, from.optional, to, false))
        this->error(expr->op, "Operator can only be used on boolean types.");

    expr->right->accept(*this);
    auto right_t = this->result;

    if (left_t.type != right_t.type)
        this->error(expr->op, "Operands must be the same type, or coercible to the same type.");
}

void TypeChecker::visit_unary_expr(UnaryExpr *expr) {
    expr->right->accept(*this);

    switch (expr->op.t) {
        case TokenType::BANG: {
            // If not boolean, we can't invert
            auto from = this->result;
            auto to = this->type_env["bool"];
            if (!can_coerce_to(from.type, from.optional, to, false))
                this->error(expr->op, "Operator can only be used on a boolean type.");

            break;
        }
        case TokenType::MINUS:
            // If not numeric, we can't invert
            if (this->result.optional || !is_numeric(this->result.type.get()))
                this->error(expr->op, "Operator can only be used on numeric types.");
            break;
        default:
            std::cerr << "[BUG] Unary operator type '" << get_token_type_str(expr->op.t) << "' not handled." << std::endl;
            exit(3);
    }
}

void TypeChecker::visit_get_expr(GetExpr *expr) {
    expr->object->accept(*this);
    if (expr->optional && !this->result.optional) {
        this->error(expr->name, "Cannot use optional chaining on non-optional type.");
    }

    if (!expr->optional && this->result.optional) {
        this->error(expr->name, "Must use optional chaining for optional type.");
    }

    this->result = expr->type_info;
}

void TypeChecker::visit_enum_init_expr(EnumInitExpr *expr) {
    // Check payload if there is one
    if (expr->payload.has_value()) {
        expr->payload.value()->accept(*this);

        // Cast to enum type (should always be enum type)
        if (auto t = std::dynamic_pointer_cast<EnumType>(expr->type_info.type)) {
            // Check type of payload for this variant
            auto payload_type_info = t->get_variant_payload_type(expr->variant.lexeme);
            auto payload_type = this->type_env[payload_type_info->type.lexeme];

            if (!can_coerce_to(this->result.type, this->result.optional, payload_type, payload_type_info->optional)) {
                this->error(expr->variant, "Payload of enum cannot be coerced to a valid type.");
            }

            if (this->result.optional && !payload_type_info->optional) {
                this->error(expr->variant, "Expected non-optional type. Received optional type.");
            }
        } else {
            std::cerr << "[BUG] Enum doesn't have enum type" << std::endl;
            exit(3);
        }
    }

    this->result = expr->type_info;
}

void TypeChecker::visit_call_expr(CallExpr *expr) {
    expr->callee->accept(*this);

    if (auto t = std::dynamic_pointer_cast<FunctionType>(expr->callee->type_info.type)) {
        if (expr->args.size() != t->params.size()) {
            this->error(expr->bracket, "Expected " + std::to_string(t->params.size()) + " arguments, received " + std::to_string(expr->args.size()) + ".");
        }

        for (int i = 0; i < expr->args.size(); i++) {
            if (expr->args[i]->type_info.type != this->type_env[t->params[i].type.lexeme]) {
                this->error(expr->bracket, "Invalid type passed to function.");
            }

            if (expr->args[i]->type_info.optional && !t->params[i].is_optional) {
                this->error(expr->bracket, "Expected non-optional type. Received optional type.");
            }
        }
    } else {
        this->error(expr->bracket, "Cannot call non-function.");
    }

    this->result = expr->type_info;

    // If we are optional chaining, carry forwards optional type
    this->result.optional |= expr->callee->type_info.optional;
}

void TypeChecker::visit_grouping_expr(GroupingExpr *expr) {
    expr->expr->accept(*this);
}

void TypeChecker::visit_literal_expr(LiteralExpr *expr) {
    this->result = expr->type_info;
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

    auto from = this->result;
    auto to = this->type_env[stmt->name.type.lexeme];
    if (!can_coerce_to(from.type, from.optional, to, stmt->name.is_optional)) {
        this->error(stmt->name.name, "Cannot assign a type '" + from.type->to_string() + (from.optional && from.type->type_class != TypeClass::NULL_ ? "?" : "") + "' to variable of type '" + to->to_string() + (stmt->name.is_optional ? "?" : "") + "'.");
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
    auto from = this->result;
    auto to = this->type_env["bool"];
    if (!can_coerce_to(from.type, from.optional, to, false)) {
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
    // TODO: handle optionals
    auto t = std::dynamic_pointer_cast<EnumType>(stmt->target->type_info.type);
    if (!t) {
        this->error(stmt->bracket, "Cannot match on non-enum.");
    }

    // TODO: when supporting concrete value matching, we may have more patterns than variants. This will need rethinking
    if (stmt->branches.size() != (t->variants.size() + (stmt->target->type_info.optional ? 1 : 0))) {
        this->error(stmt->bracket, "Not all variants covered in pattern matching.");
    }

    bool has_null_branch = false;
    for (auto &branch : stmt->branches) {
        if (std::holds_alternative<EnumPattern>(branch.pattern)) {
            auto &enum_pattern = std::get<EnumPattern>(branch.pattern);
            if (enum_pattern.enum_type.lexeme != t->name.lexeme) {
                this->error(enum_pattern.enum_type, "Match pattern must be for the same enum type.");
            }

            auto variant_name = enum_pattern.enum_variant.lexeme;
            auto v = std::find_if(t->variants.begin(), t->variants.end(), [variant_name](const auto &v) {
                return v.name.lexeme == variant_name;
            });

            if (v == t->variants.end()) {
                this->error(enum_pattern.enum_type, "Could not find variant on enum type.");
            }

            if (v->payload_type.has_value()) {
                if (!enum_pattern.bound_variable.has_value()) {
                    this->error(enum_pattern.enum_variant, "Enum variant expects payload.");
                }

                // TODO: when supporting concrete values, check the type is correct - make sure resolver sets the type
                //
                // auto payload_type = this->type_env[v->payload_type.value().lexeme];
                // auto &provided_type_info = enum_pattern.bound_variable.value()->type_info;
                // if (!can_coerce_to(provided_type_info.type, provided_type_info.optional, payload_type, /* TODO: payload optional */)) {
                //     this->error(enum_pattern.enum_type, "Payload type must be equal to or coercible to the type in the enum definition.");
                // }
            } else {
                if (enum_pattern.bound_variable.has_value()) {
                    this->error(enum_pattern.enum_type, "No payload available for this enum variant.");
                }
            }
        } else if (std::holds_alternative<NullPattern>(branch.pattern)) {
            has_null_branch = true;
        }

        for (auto &line : branch.body) {
            line->accept(*this);
        }
    }

    if (stmt->target->type_info.optional && !has_null_branch) {
        this->error(stmt->bracket, "Null variant not provided.");
    }
}

void TypeChecker::visit_while_stmt(WhileStmt *stmt) {
    stmt->condition->accept(*this);

    auto from = this->result;
    auto to = this->type_env["bool"];
    if (!can_coerce_to(from.type, from.optional, to, false)) {
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

    // TODO: Check if existing variable is nullable
    auto from = this->result;
    auto to = stmt->target_type;
    if (!can_coerce_to(from.type, from.optional, to, false)) {
        this->error(stmt->name, "Cannot assign a different type to this variable.");
    }
}
