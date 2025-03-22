#include "resolver.h"
#include "type.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <tuple>

Resolver::Resolver(std::map<std::string, std::shared_ptr<Type>> type_env) : type_env(type_env) {
    // Global scope
    this->scopes.push_back({});
}

void Resolver::error(Token error_token, std::string message) {
    std::string where = "end";
    if (error_token.t != TokenType::EOF_)
        where = "'" + error_token.lexeme + "'";

    std::cerr << "[line " << error_token.line << "] Resolving error at " << where << ": " << message << std::endl;

    // TODO: handle this
    exit(3);
}

void Resolver::begin_scope() {
    this->scopes.push_back({});
}

void Resolver::end_scope() {
    this->scopes.pop_back();
}

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

std::optional<ResolvedVariable> Resolver::resolve_local(Token name) {
    for (int i = this->scopes.size() - 1; i >= 0; i--) {
        auto scope = this->scopes[i];
        auto resolved = scope.find(name.lexeme);
        if (resolved != scope.end()) {
            return resolved->second;
        }
    }

    return {};
}

void Resolver::resolve_function(FunDeclStmt *fun) {
    this->begin_scope();
    for (auto param : fun->params) {
        this->declare(param.name.lexeme, this->type_env[param.type.lexeme]);
        this->define(param.name.lexeme);
    }

    this->resolve(fun->body);
    this->end_scope();
}

void Resolver::resolve_struct(StructDeclStmt *s) {
    this->begin_scope();

    std::string this_keyword("this");
    this->declare(this_keyword, this->type_env[s->name.lexeme]);
    this->define(this_keyword);

    for (auto &prop : s->properties) {
        this->declare(prop.name.lexeme, this->type_env[prop.type.lexeme]);
        this->define(prop.name.lexeme);
    }

    for (auto &method : s->methods) {
        method->accept(*this);
    }

    this->end_scope();
}

void Resolver::resolve_enum(EnumDeclStmt *e) {
    this->begin_scope();

    std::string this_keyword("this");
    this->declare(this_keyword, this->type_env[e->name.lexeme]);
    this->define(this_keyword);

    for (auto &method : e->methods) {
        method->accept(*this);
    }

    this->end_scope();
}

void Resolver::define(std::string &name) {
    auto &scope = this->scopes.back();
    auto val = scope.find(name);
    if (val == scope.end()) {
        std::cerr << "[BUG] Defining a variable that doesn't exist." << std::endl;
        exit(3);
    }

    scope[name].defined = true;
}

void Resolver::declare(std::string &name, std::shared_ptr<Type> type) {
    auto &scope = this->scopes.back();

    scope[name] = ResolvedVariable{
        name,
        false,
        type,
    };
}

// Expressions
void Resolver::visit_var_expr(VarExpr *expr) {
    auto resolved = this->resolve_local(expr->name);
    if (!resolved.has_value()) {
        this->error(expr->name, "Unknown variable - not declared at this point.");
    }

    if (!resolved.value().defined)
        this->error(expr->name, "Cannot read local variable in its own initialiser.");

    expr->type = resolved.value().type;
}

void Resolver::visit_struct_init_expr(StructInitExpr *expr) {
    for (auto &prop : expr->properties) {
        std::get<1>(prop)->accept(*this);
    }

    if (auto t = std::dynamic_pointer_cast<StructType>(this->type_env[expr->name.lexeme])) {
        expr->type = t;
        return;
    }

    this->error(expr->name, "Cannot use struct initialiser on non-struct.");
}

void Resolver::visit_binary_expr(BinaryExpr *expr) {
    this->resolve(expr->left.get());
    this->resolve(expr->right.get());
}

void Resolver::visit_logical_binary_expr(LogicalBinaryExpr *expr) {
    this->resolve(expr->left.get());
    this->resolve(expr->right.get());
}

void Resolver::visit_unary_expr(UnaryExpr *expr) {
    this->resolve(expr->right.get());
}

void Resolver::visit_get_expr(GetExpr *expr) {
    this->resolve(expr->object.get());

    if (auto t = std::dynamic_pointer_cast<StructType>(expr->object->type)) {
        auto method_type = t->get_method_type(expr->name.lexeme);
        if (method_type.has_value()) {
            expr->type = method_type.value();
            return;
        }

        auto prop_type = t->get_prop_type(expr->name.lexeme);
        if (prop_type.has_value()) {
            expr->type = this->type_env[prop_type.value().lexeme];
            return;
        }

        this->error(expr->name, "Could not find member on struct.");
    } else if (auto t = std::dynamic_pointer_cast<EnumType>(expr->object->type)) {
        auto type = t->get_method_type(expr->name.lexeme);
        if (type.has_value()) {
            expr->type = type.value();
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
        expr->type = t;
        return;
    }

    this->error(enum_name, "Cannot use struct initialiser on non-struct.");
}

void Resolver::visit_call_expr(CallExpr *expr) {
    this->resolve(expr->callee.get());

    if (auto t = std::dynamic_pointer_cast<FunctionType>(expr->callee->type)) {
        expr->type = this->type_env[t->return_type.lexeme];
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
    switch (expr->literal.t) {
        case TokenType::INT_VAL:   expr->type = this->type_env["int"]; break;
        case TokenType::FLOAT_VAL: expr->type = this->type_env["float"]; break;
        case TokenType::BOOL_VAL:  expr->type = this->type_env["bool"]; break;
        case TokenType::NULL_VAL:  expr->type = this->type_env["null"]; break;
        case TokenType::STR_VAL:   expr->type = this->type_env["str"]; break;
        case TokenType::TRUE:
        case TokenType::FALSE:
            expr->type = this->type_env["bool"];
            break;
        default:
            std::cout << "[BUG] Literal type unknown" << std::endl;
            exit(3);
    };
}

// Statements
void Resolver::visit_fun_decl_stmt(FunDeclStmt *stmt) {
    // TODO: move this to FunDeclStmt? To match with StructDeclStmt
    std::vector<std::tuple<Token, Token>> params;
    std::transform(stmt->params.begin(), stmt->params.end(), std::back_inserter(params), [this](TypedVar param) {
        return std::make_tuple(param.name, param.type);
    });
    auto func_type = std::make_shared<FunctionType>(
        stmt->name,
        params,
        stmt->return_type);
    this->declare(stmt->name.lexeme, func_type);
    this->define(stmt->name.lexeme);

    this->resolve_function(stmt);
}

void Resolver::visit_enum_method_decl_stmt(EnumMethodDeclStmt *enum_stmt) {
    auto &stmt = enum_stmt->fun_definition;

    // TODO: move this to FunDeclStmt? To match with StructDeclStmt
    std::vector<std::tuple<Token, Token>> params;
    std::transform(stmt->params.begin(), stmt->params.end(), std::back_inserter(params), [this](TypedVar param) {
        return std::make_tuple(param.name, param.type);
    });
    auto func_type = std::make_shared<FunctionType>(
        stmt->name,
        params,
        stmt->return_type);
    this->declare(stmt->name.lexeme, func_type);
    this->define(stmt->name.lexeme);

    // TODO: put this in method
    this->begin_scope();
    for (auto param : stmt->params) {
        this->declare(param.name.lexeme, this->type_env[param.type.lexeme]);
        this->define(param.name.lexeme);
    }

    this->resolve(stmt->body);
    this->end_scope();
}

void Resolver::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    this->declare(stmt->name.lexeme, this->type_env[stmt->name.lexeme]);
    this->define(stmt->name.lexeme);

    this->resolve_struct(stmt);
}

void Resolver::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    this->declare(stmt->name.lexeme, this->type_env[stmt->name.lexeme]);
    this->define(stmt->name.lexeme);

    this->resolve_enum(stmt);
}

void Resolver::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    this->declare(stmt->name.name.lexeme, this->type_env[stmt->name.type.lexeme]);
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

    auto enum_type = std::dynamic_pointer_cast<EnumType>(stmt->target->type);
    if (!enum_type) {
        // TODO: get token from parser
        this->error(Token{TokenType::L_BRACKET, "PLACEHOLDER", 1}, "Trying to pattern match on non-enum.");
    }

    for (auto &branch : stmt->branches) {
        if (branch.pattern.bound_variable.has_value()) {
            auto &var = branch.pattern.bound_variable.value();
            auto name = branch.pattern.enum_variant.lexeme;

            // TODO: use hashmap or lookup function
            auto variant = std::find_if(enum_type->variants.begin(), enum_type->variants.end(), [name](const auto &t) {
                return t.name.lexeme == name;
            });

            if (variant == enum_type->variants.end()) {
                // TODO: get token from parser
                this->error(Token{TokenType::L_BRACKET, "PLACEHOLDER", 1}, "Could not find variant on enum.");
            }

            if (!variant->payload_type.has_value()) {
                // TODO: get token from parser
                this->error(Token{TokenType::L_BRACKET, "PLACEHOLDER", 1}, "Enum variant has no payload - cannot bind a variable.");
            }

            this->declare(var->name.lexeme, this->type_env[variant->payload_type.value().lexeme]);
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

void Resolver::visit_return_stmt(ReturnStmt *stmt) {
    if (stmt->expr.has_value())
        this->resolve(stmt->expr.value().get());
}

void Resolver::visit_assign_stmt(AssignStmt *stmt) {
    this->resolve(stmt->value.get());
    auto var = this->resolve_local(stmt->name);

    if (!var.has_value())
        this->error(stmt->name, "Cannot assign to a variable that hasn't been declared.");

    stmt->target_type = var.value().type;
}
