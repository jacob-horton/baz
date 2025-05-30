#include "resolver.h"
#include "type.h"

#include <algorithm>
#include <iostream>
#include <memory>

Resolver::Resolver(std::map<std::string, std::shared_ptr<Type>> type_env) : type_env(type_env) {
    // Global scope
    this->scopes.push_back({});
}

// Print error with line number and exit
void Resolver::error(Token error_token, std::string message) {
    std::string where = "end";
    if (error_token.t != TokenType::EOF_)
        where = "'" + error_token.lexeme + "'";

    std::cerr << "[line " << error_token.line << "] Resolving error at " << where << ": " << message << std::endl;
    exit(5);
}

void Resolver::begin_scope() {
    this->scopes.push_back({});
}

void Resolver::end_scope() {
    this->scopes.pop_back();
}

// Resolve a list of statements
void Resolver::resolve(std::vector<std::unique_ptr<Stmt>> &stmts) {
    for (auto &stmt : stmts) {
        stmt->accept(*this);
    }
}

void Resolver::resolve(Stmt *stmt) {
    stmt->accept(*this);
}

void Resolver::resolve(Expr *expr) {
    expr->accept(*this);
}

// Try to find the variable name in an accessible scope
std::optional<ResolvedVariable> Resolver::resolve_local(Token name) {
    // Loop from current scope, up to top - find the first one to match the variable name
    for (int i = this->scopes.size() - 1; i >= 0; i--) {
        auto scope = this->scopes[i];
        auto resolved = scope.find(name.lexeme);
        if (resolved != scope.end()) {
            return resolved->second;
        }
    }

    return std::nullopt;
}

void Resolver::resolve_function(FunDeclStmt *fun) {
    this->begin_scope();

    // Declare and define all parameters
    for (auto param : fun->params) {
        this->declare(param.name.lexeme, this->type_env[param.type.lexeme], param.is_optional);
        this->define(param.name.lexeme);
    }

    // Resolve the body
    this->resolve(fun->body);
    this->end_scope();
}

void Resolver::resolve_struct(StructDeclStmt *s) {
    this->begin_scope();

    // Always have access to "this" within a struct
    std::string this_keyword("this");
    this->declare(this_keyword, this->type_env[s->name.lexeme], false);
    this->define(this_keyword);

    // Declare and define all properties
    for (auto &prop : s->properties) {
        auto type = this->type_env.find(prop.type.lexeme);
        if (type == this->type_env.end())
            this->error(prop.type, "Unknown type for struct prop.");

        this->declare(prop.name.lexeme, this->type_env[prop.type.lexeme], prop.is_optional);
        this->define(prop.name.lexeme);
    }

    // Resolve all methods
    for (auto &method : s->methods) {
        method->accept(*this);
    }

    this->end_scope();
}

void Resolver::resolve_enum(EnumDeclStmt *e) {
    this->begin_scope();

    // Always have access to "this" within an enum
    std::string this_keyword("this");
    this->declare(this_keyword, this->type_env[e->name.lexeme], false);
    this->define(this_keyword);

    // Resolve all methods
    for (auto &method : e->methods) {
        method->accept(*this);
    }

    this->end_scope();
}

// Define a variable after it has been declared
void Resolver::define(std::string &name) {
    auto &scope = this->scopes.back();
    auto val = scope.find(name);
    if (val == scope.end()) {
        std::cerr << "[BUG] Defining a variable that doesn't exist." << std::endl;
        exit(3);
    }

    scope[name].defined = true;
}

// Declare a variable and its type in the current scope
void Resolver::declare(std::string &name, std::shared_ptr<Type> type, bool optional) {
    auto &scope = this->scopes.back();

    scope[name] = ResolvedVariable{
        name,
        false,
        type,
        optional};
}

//// Expressions

void Resolver::visit_var_expr(VarExpr *expr) {
    auto resolved = this->resolve_local(expr->name);
    if (!resolved.has_value()) {
        this->error(expr->name, "Unknown variable - not declared at this point.");
    }

    if (!resolved.value().defined)
        this->error(expr->name, "Cannot read local variable in its own initialiser.");

    expr->set_type_info(TypeInfo(resolved.value().type, resolved.value().optional));
}

void Resolver::visit_struct_init_expr(StructInitExpr *expr) {
    for (auto &prop : expr->properties) {
        std::get<1>(prop)->accept(*this);
    }

    if (auto t = std::dynamic_pointer_cast<StructType>(this->type_env[expr->name.lexeme])) {
        // Since we are initialising it, it must be non-null
        expr->set_type_info(TypeInfo(t, false));
        return;
    }

    this->error(expr->name, "Cannot use struct initialiser on non-struct.");
}

void Resolver::visit_binary_expr(BinaryExpr *expr) {
    this->resolve(expr->left.get());
    this->resolve(expr->right.get());
}

void Resolver::visit_unary_expr(UnaryExpr *expr) {
    this->resolve(expr->right.get());
}

void Resolver::visit_get_expr(GetExpr *expr) {
    this->resolve(expr->object.get());

    if (auto t = std::dynamic_pointer_cast<StructType>(expr->object->get_type_info().type)) {
        auto method_type = t->get_method_type(expr->name.lexeme);
        if (method_type.has_value()) {
            // If the struct is optional, we are using optional chaining - the result is optional
            expr->set_type_info(TypeInfo(method_type.value(), expr->object->get_type_info().optional));
            return;
        }

        auto prop_type = t->get_prop_type(expr->name.lexeme);
        if (prop_type.has_value()) {
            // If the struct is optional, we are using optional chaining - the result is optional
            // Otherwise, check if the property is optional
            expr->set_type_info(TypeInfo(this->type_env[prop_type.value().type.lexeme], expr->object->get_type_info().optional || prop_type.value().optional));
            return;
        }

        this->error(expr->name, "Could not find member on struct.");
    } else if (auto t = std::dynamic_pointer_cast<EnumType>(expr->object->get_type_info().type)) {
        auto type = t->get_method_type(expr->name.lexeme);
        if (type.has_value()) {
            // If the enum is optional, we are using optional chaining - the result is optional
            expr->set_type_info(TypeInfo(type.value(), expr->object->get_type_info().optional));
            return;
        }

        this->error(expr->name, "Could not find property on enum.");
    } else {
        this->error(expr->name, "Cannot get property on a variable that is neither a struct nor enum.");
    }
}

void Resolver::visit_enum_init_expr(EnumInitExpr *expr) {
    if (expr->payload.has_value()) {
        expr->payload.value()->accept(*this);
    }

    auto enum_name = expr->enum_namespace->name;
    if (auto t = std::dynamic_pointer_cast<EnumType>(this->type_env[enum_name.lexeme])) {
        // Since we are initialising it, it must be non-null
        expr->set_type_info(TypeInfo(t, false));
        return;
    }

    this->error(enum_name, "Cannot use struct initialiser on non-struct.");
}

void Resolver::visit_call_expr(CallExpr *expr) {
    this->resolve(expr->callee.get());

    if (auto t = std::dynamic_pointer_cast<FunctionType>(expr->callee->get_type_info().type)) {
        expr->set_type_info(TypeInfo(this->type_env[t->return_type.lexeme], t->return_type_optional));
    } else {
        this->error(expr->bracket, "Cannot call non-function.");
    }

    for (auto &arg : expr->args) {
        this->resolve(arg.get());
    }
}

void Resolver::visit_grouping_expr(GroupingExpr *expr) {
    this->resolve(expr->expr.get());
}

// NOTE: no variables to resolve here
void Resolver::visit_literal_expr(LiteralExpr *expr) {
    // Set type info if literal
    switch (expr->literal.t) {
        case TokenType::INT_VAL:   expr->set_type_info(TypeInfo(this->type_env["int"], false)); break;
        case TokenType::FLOAT_VAL: expr->set_type_info(TypeInfo(this->type_env["float"], false)); break;
        case TokenType::BOOL_VAL:  expr->set_type_info(TypeInfo(this->type_env["bool"], false)); break;
        case TokenType::NULL_VAL:  expr->set_type_info(TypeInfo(this->type_env["null"], true)); break;
        case TokenType::STR_VAL:   expr->set_type_info(TypeInfo(this->type_env["str"], false)); break;
        case TokenType::TRUE:
        case TokenType::FALSE:
            expr->set_type_info(TypeInfo(this->type_env["bool"], false));
            break;
        default:
            std::cout << "[BUG] Literal type unknown" << std::endl;
            exit(3);
    };
}

//// Statements

void Resolver::visit_fun_decl_stmt(FunDeclStmt *stmt) {
    auto func_type = std::make_shared<FunctionType>(
        stmt->name,
        stmt->params,
        stmt->return_type,
        stmt->return_type_optional);

    // Functions can never be optional
    this->declare(stmt->name.lexeme, func_type, false);
    this->define(stmt->name.lexeme);

    this->resolve_function(stmt);
}

void Resolver::visit_enum_method_decl_stmt(EnumMethodDeclStmt *enum_stmt) {
    auto &stmt = enum_stmt->fun_definition;

    auto func_type = std::make_shared<FunctionType>(
        stmt->name,
        stmt->params,
        stmt->return_type,
        stmt->return_type_optional);

    this->declare(stmt->name.lexeme, func_type, false);
    this->define(stmt->name.lexeme);

    this->begin_scope();
    for (auto param : stmt->params) {
        this->declare(param.name.lexeme, this->type_env[param.type.lexeme], false);
        this->define(param.name.lexeme);
    }

    this->resolve(stmt->body);
    this->end_scope();
}

void Resolver::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    this->declare(stmt->name.lexeme, this->type_env[stmt->name.lexeme], false);
    this->define(stmt->name.lexeme);

    this->resolve_struct(stmt);
}

void Resolver::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    this->declare(stmt->name.lexeme, this->type_env[stmt->name.lexeme], false);
    this->define(stmt->name.lexeme);

    this->resolve_enum(stmt);
}

void Resolver::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    this->declare(stmt->name.name.lexeme, this->type_env[stmt->name.type.lexeme], stmt->name.is_optional);
    this->resolve(stmt->initialiser.get());
    this->define(stmt->name.name.lexeme);
}

void Resolver::visit_expr_stmt(ExprStmt *stmt) {
    this->resolve(stmt->expr.get());
}

void Resolver::visit_block_stmt(BlockStmt *stmt) {
    this->begin_scope();
    this->resolve(stmt->stmts);
    this->end_scope();
}

void Resolver::visit_if_stmt(IfStmt *stmt) {
    this->resolve(stmt->condition.get());
    this->resolve(stmt->true_block);

    if (stmt->false_block.has_value())
        this->resolve(stmt->false_block.value());
}

void Resolver::visit_match_stmt(MatchStmt *stmt) {
    this->resolve(stmt->target.get());

    auto enum_type = std::dynamic_pointer_cast<EnumType>(stmt->target->get_type_info().type);
    if (!(enum_type || stmt->target->get_type_info().optional)) {
        this->error(stmt->keyword, "Can only match on enum or optional.");
    }

    for (auto &branch : stmt->branches) {
        if (std::holds_alternative<EnumPattern>(branch.pattern)) {
            auto &enum_pattern = std::get<EnumPattern>(branch.pattern);

            // If we have a bound value - check its type and declare/define it
            if (enum_pattern.bound_variable.has_value()) {
                auto &var = enum_pattern.bound_variable.value();
                auto name = enum_pattern.enum_variant.lexeme;

                auto pattern_type = std::dynamic_pointer_cast<EnumType>(this->type_env[enum_pattern.enum_type.lexeme]);
                if (!pattern_type) {
                    this->error(enum_pattern.enum_type, "Pattern must be an enum variant.");
                }

                auto variant = std::find_if(pattern_type->variants.begin(), pattern_type->variants.end(), [name](const auto &t) {
                    return t.name.lexeme == name;
                });

                if (variant == pattern_type->variants.end()) {
                    this->error(enum_pattern.enum_variant, "Could not find variant on enum.");
                }

                if (!variant->payload_type.has_value()) {
                    this->error(enum_pattern.bound_variable.value()->name, "Enum variant has no payload - cannot bind a variable.");
                }

                this->declare(var->name.lexeme, this->type_env[variant->payload_type.value().lexeme], variant->is_optional);
                this->define(var->name.lexeme);
            }
        } else if (std::holds_alternative<CatchAllPattern>(branch.pattern)) {
            auto &catch_all_pattern = std::get<CatchAllPattern>(branch.pattern);
            auto &var = catch_all_pattern.bound_variable;

            this->declare(var->name.lexeme, stmt->target->get_type_info().type, false);
            this->define(var->name.lexeme);
        }

        this->resolve(branch.body);
    }
}

void Resolver::visit_while_stmt(WhileStmt *stmt) {
    this->resolve(stmt->condition.get());
    this->resolve(stmt->stmts);
}

void Resolver::visit_for_stmt(ForStmt *stmt) {
    this->resolve(stmt->var.get());
    this->resolve(stmt->condition.get());
    this->resolve(stmt->increment.get());
    this->resolve(stmt->stmts);
}

void Resolver::visit_print_stmt(PrintStmt *stmt) {
    if (stmt->expr.has_value())
        this->resolve(stmt->expr.value().get());
}

void Resolver::visit_panic_stmt(PanicStmt *stmt) {
    if (stmt->expr.has_value())
        this->resolve(stmt->expr.value().get());
}

void Resolver::visit_return_stmt(ReturnStmt *stmt) {
    if (stmt->expr.has_value())
        this->resolve(stmt->expr.value().get());
}

void Resolver::visit_assign_stmt(AssignStmt *stmt) {
    this->resolve(stmt->value.get());
    auto var = this->resolve_local(stmt->name);

    if (!var.has_value())
        this->error(stmt->name, "Cannot assign to a variable that hasn't been declared.");

    stmt->set_target_type_info(TypeInfo(var.value().type, var.value().optional));
}

void Resolver::visit_set_stmt(SetStmt *stmt) {
    this->resolve(stmt->value.get());
    this->resolve(stmt->object.get());

    if (stmt->object->get_type_info().optional) {
        this->error(stmt->name, "Assigning to a property of an optional struct is not currently supported.");
    }

    if (auto t = std::dynamic_pointer_cast<StructType>(stmt->object->get_type_info().type)) {
        auto prop_type = t->get_prop_type(stmt->name.lexeme);
        if (!prop_type.has_value()) {
            this->error(stmt->name, "Could not find property on struct.");
        }

        stmt->set_target_type_info(TypeInfo(this->type_env[prop_type.value().type.lexeme], prop_type.value().optional));
    } else {
        this->error(stmt->name, "Cannot get property on a variable that is not a struct.");
    }
}
