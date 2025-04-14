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

bool TypeChecker::is_numeric(Type *t) {
    return t->can_coerce_to(this->type_env["int"]) || t->can_coerce_to(this->type_env["float"]);
}

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
    this->result = expr->get_type_info();
    expr->set_type_info(this->result);
    this->always_returns = false;
}

void TypeChecker::visit_struct_init_expr(StructInitExpr *expr) {
    if (auto t = std::dynamic_pointer_cast<StructType>(expr->get_type_info().type)) {
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

            auto from = std::get<1>(*p)->get_type_info();
            auto to = this->type_env[prop.type.lexeme];
            if (!can_coerce_to(from.type, from.optional, to, prop.is_optional)) {
                this->error(std::get<0>(*p), "Cannot assign a type '" + from.type->to_string() + (from.optional && from.type->type_class != TypeClass::NULL_ ? "?" : "") + "' to variable of type '" + to->to_string() + (prop.is_optional ? "?" : "") + "'.");
            }
        }
    } else {
        this->error(expr->name, "Cannot initialise non-struct.");
    }

    this->result = expr->get_type_info();
    expr->set_type_info(this->result);
    this->always_returns = false;
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

            this->result = left_t;
            expr->set_type_info(this->result);
            this->always_returns = false;
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
            expr->set_type_info(this->result);
            this->always_returns = false;
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
            expr->set_type_info(this->result);

            this->always_returns = false;
            return;
        }
        case TokenType::EQUAL_EQUAL: {
            expr->left->accept(*this);
            auto left_t = this->result;

            expr->right->accept(*this);
            auto right_t = this->result;

            // Left can coerce to right or right to left
            if (!(can_coerce_to(right_t.type, right_t.optional, left_t.type, left_t.optional) || can_coerce_to(left_t.type, left_t.optional, right_t.type, right_t.optional)))
                this->error(expr->op, "Operands must be the same type, or coercible to the same type.");

            this->result = TypeInfo(this->type_env["bool"], false);
            expr->set_type_info(this->result);
            this->always_returns = false;
            return;
        }
        case TokenType::AND:
        case TokenType::OR:  {
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

            this->result = TypeInfo(this->type_env["bool"], false);
            expr->set_type_info(this->result);
            this->always_returns = false;
            return;
        }
        default:
            std::cerr << "[BUG] Binary operator type '" << get_token_type_str(expr->op.t) << "' not handled." << std::endl;
            exit(3);
    }
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

    expr->set_type_info(this->result);
    this->always_returns = false;
}

void TypeChecker::visit_get_expr(GetExpr *expr) {
    expr->object->accept(*this);
    if (expr->optional && !this->result.optional) {
        this->error(expr->name, "Cannot use optional chaining on non-optional type.");
    }

    if (!expr->optional && this->result.optional) {
        this->error(expr->name, "Must use optional chaining for optional type.");
    }

    this->result = expr->get_type_info();
    expr->set_type_info(this->result);
    this->always_returns = false;
}

void TypeChecker::visit_enum_init_expr(EnumInitExpr *expr) {
    // Check payload if there is one
    if (expr->payload.has_value()) {
        expr->payload.value()->accept(*this);

        // Cast to enum type (should always be enum type)
        if (auto t = std::dynamic_pointer_cast<EnumType>(expr->get_type_info().type)) {
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

    this->result = expr->get_type_info();
    expr->set_type_info(this->result);
    this->always_returns = false;
}

void TypeChecker::visit_call_expr(CallExpr *expr) {
    expr->callee->accept(*this);

    if (auto t = std::dynamic_pointer_cast<FunctionType>(expr->callee->get_type_info().type)) {
        if (expr->args.size() != t->params.size()) {
            this->error(expr->bracket, "Expected " + std::to_string(t->params.size()) + " arguments, received " + std::to_string(expr->args.size()) + ".");
        }

        for (int i = 0; i < expr->args.size(); i++) {
            expr->args[i]->accept(*this);
            if (!this->result.type->can_coerce_to(this->type_env[t->params[i].type.lexeme])) {
                this->error(expr->bracket, "Invalid type passed to function.");
            }

            if (this->result.optional && !t->params[i].is_optional) {
                this->error(expr->bracket, "Expected non-optional type. Received optional type.");
            }
        }
    } else {
        this->error(expr->bracket, "Cannot call non-function.");
    }

    this->result = expr->get_type_info();
    expr->set_type_info(this->result);

    // If we are optional chaining, carry forwards optional type
    this->result.optional |= expr->callee->get_type_info().optional;
    this->always_returns = false;
}

void TypeChecker::visit_grouping_expr(GroupingExpr *expr) {
    expr->expr->accept(*this);
    expr->set_type_info(this->result);
    this->always_returns = false;
}

void TypeChecker::visit_literal_expr(LiteralExpr *expr) {
    this->result = expr->get_type_info();
    expr->set_type_info(this->result);
    this->always_returns = false;
}

// Statements
void TypeChecker::visit_fun_decl_stmt(FunDeclStmt *fun) {
    auto prev_fn_ret_type = this->surrounding_fn_return_type;
    this->surrounding_fn_return_type = Temp{
        fun->return_type,
        fun->return_type_optional,
    };

    bool returns = false;
    for (auto &s : fun->body) {
        s->accept(*this);
        returns = returns || this->always_returns;
    }

    this->surrounding_fn_return_type = prev_fn_ret_type;

    if (fun->return_type.lexeme != "void" && !returns) {
        this->error(fun->name, "Not all code paths return.");
    }

    this->always_returns = false;
}

void TypeChecker::visit_enum_method_decl_stmt(EnumMethodDeclStmt *fun) {
    fun->fun_definition->accept(*this);

    this->always_returns = false;
}

void TypeChecker::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    for (auto &method : stmt->methods) {
        method->accept(*this);
    }

    this->always_returns = false;
}

void TypeChecker::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    for (auto &method : stmt->methods) {
        method->accept(*this);
    }

    this->always_returns = false;
}

void TypeChecker::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    stmt->initialiser->accept(*this);

    auto from = this->result;
    auto to = this->type_env[stmt->name.type.lexeme];
    if (!can_coerce_to(from.type, from.optional, to, stmt->name.is_optional)) {
        this->error(stmt->name.name, "Cannot assign a type '" + from.type->to_string() + (from.optional && from.type->type_class != TypeClass::NULL_ ? "?" : "") + "' to variable of type '" + to->to_string() + (stmt->name.is_optional ? "?" : "") + "'.");
    }

    this->always_returns = false;
}

void TypeChecker::visit_expr_stmt(ExprStmt *stmt) {
    stmt->expr->accept(*this);
    this->always_returns = false;
}

void TypeChecker::visit_block_stmt(BlockStmt *stmt) {
    bool returns = false;

    for (auto &s : stmt->stmts) {
        s->accept(*this);

        // If any statement always returns, the block always returns
        returns = returns || this->always_returns;
    }

    this->always_returns = returns;
}

void TypeChecker::visit_if_stmt(IfStmt *stmt) {
    stmt->condition->accept(*this);

    auto from = this->result;
    auto to = this->type_env["bool"];
    if (!can_coerce_to(from.type, from.optional, to, false)) {
        this->error(stmt->keyword, "If condition must be a boolean value.");
    }

    bool true_block_returns = false;
    for (auto &line : stmt->true_block) {
        line->accept(*this);
        true_block_returns = true_block_returns || this->always_returns;
    }

    bool false_block_returns = false;
    if (stmt->false_block.has_value()) {
        for (auto &line : stmt->false_block.value()) {
            line->accept(*this);
            false_block_returns = false_block_returns || this->always_returns;
        }
    }

    this->always_returns = true_block_returns && false_block_returns;
}

void TypeChecker::visit_match_stmt(MatchStmt *stmt) {
    bool all_branches_return = true;

    auto t = std::dynamic_pointer_cast<EnumType>(stmt->target->get_type_info().type);
    if (!(t || stmt->target->get_type_info().optional)) {
        this->error(stmt->keyword, "Can only match on enum or optional.");
    }

    if (t) {
        // Enum
        // TODO: when supporting concrete value matching, we may have more patterns than variants. This will need rethinking
        if (stmt->branches.size() != (t->variants.size() + (stmt->target->get_type_info().optional ? 1 : 0))) {
            this->error(stmt->keyword, "Not all variants covered in pattern matching.");
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
                    // auto &provided_type_info = enum_pattern.bound_variable.value()->get_type_info();
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
            } else {
                this->error(stmt->keyword, "Can only match enum variant or null on enum.");
            }

            bool returns = false;
            for (auto &line : branch.body) {
                line->accept(*this);
                returns = returns || this->always_returns;
            }

            if (!returns)
                all_branches_return = false;
        }

        if (stmt->target->get_type_info().optional && !has_null_branch) {
            this->error(stmt->keyword, "Null variant not provided.");
        }

        this->always_returns = all_branches_return;
    } else {
        // Either null or bound variable
        if (stmt->branches.size() != 2) {
            this->error(stmt->keyword, "Not all variants covered in pattern matching.");
        }

        bool has_null_branch = false;
        bool has_catch_all_branch = false;
        for (auto &branch : stmt->branches) {
            if (std::holds_alternative<CatchAllPattern>(branch.pattern)) {
                has_catch_all_branch = true;
            } else if (std::holds_alternative<NullPattern>(branch.pattern)) {
                has_null_branch = true;
            } else {
                this->error(stmt->keyword, "Can only match existing or null on enum.");
            }

            bool returns = false;
            for (auto &line : branch.body) {
                line->accept(*this);
                returns = returns || this->always_returns;
            }

            if (!returns)
                all_branches_return = false;
        }

        if (!has_null_branch || !has_catch_all_branch) {
            this->error(stmt->keyword, "Not all variants covered in pattern matching.");
        }

        this->always_returns = all_branches_return;
    }
}

void TypeChecker::visit_while_stmt(WhileStmt *stmt) {
    stmt->condition->accept(*this);

    auto from = this->result;
    auto to = this->type_env["bool"];
    if (!can_coerce_to(from.type, from.optional, to, false)) {
        this->error(stmt->keyword, "While condition must be a boolean value.");
    }

    bool returns = false;
    for (auto &s : stmt->stmts) {
        s->accept(*this);
        returns = returns || this->always_returns;
    }

    this->always_returns = returns;
}

void TypeChecker::visit_for_stmt(ForStmt *stmt) {
    stmt->var->accept(*this);
    stmt->condition->accept(*this);
    stmt->increment->accept(*this);

    bool returns = false;
    for (auto &s : stmt->stmts) {
        s->accept(*this);
        returns = returns || this->always_returns;
    }

    this->always_returns = returns;
}

void TypeChecker::visit_print_stmt(PrintStmt *stmt) {
    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);

    this->always_returns = false;
}

void TypeChecker::visit_panic_stmt(PanicStmt *stmt) {
    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);

    this->always_returns = true;
}

void TypeChecker::visit_return_stmt(ReturnStmt *stmt) {
    if (!this->surrounding_fn_return_type.has_value()) {
        this->error(stmt->keyword, "Cannot return from outside of a function.");
    }

    if (stmt->expr.has_value()) {
        stmt->expr.value()->accept(*this);

        if (!can_coerce_to(this->result.type, this->result.optional, this->type_env[this->surrounding_fn_return_type->type.lexeme], this->surrounding_fn_return_type->optional)) {
            this->error(stmt->keyword, "Must return a type that equals or can coerce to the return type of the function.");
        }
    } else {
        if (this->surrounding_fn_return_type->type.lexeme != "void") {
            this->error(stmt->keyword, "Must return a value from a non-void function.");
        }
    }

    this->always_returns = true;
}

void TypeChecker::visit_assign_stmt(AssignStmt *stmt) {
    stmt->value->accept(*this);

    auto from = this->result;
    auto to = stmt->get_target_type_info();
    if (!can_coerce_to(from.type, from.optional, to.type, to.optional)) {
        this->error(stmt->name, "Cannot assign a different type to this variable.");
    }

    this->always_returns = false;
}

void TypeChecker::visit_set_stmt(SetStmt *stmt) {
    stmt->value->accept(*this);

    auto from = this->result;
    auto to = stmt->get_target_type_info();
    if (!can_coerce_to(from.type, from.optional, to.type, to.optional)) {
        this->error(stmt->name, "Cannot assign a different type to this variable.");
    }

    this->always_returns = false;
}
