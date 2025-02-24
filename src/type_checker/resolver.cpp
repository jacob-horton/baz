#include "resolver.h"
#include "type.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <tuple>

Resolver::Resolver() {
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

BoundVariable Resolver::resolve_local(Token name) {
    for (int i = this->scopes.size() - 1; i >= 0; i--) {
        auto scope = this->scopes[i];
        auto resolved = scope.find(name.lexeme);
        if (resolved != scope.end()) {
            return resolved->second;
        }
    }

    // TODO: handle this - I don't think it's a bug in the compiler - it is a user error
    std::cerr << "[BUG] Could not find variable." << std::endl;
    exit(3);
}

void Resolver::resolve_function(FunDeclStmt *fun) {
    this->begin_scope();
    for (auto param : fun->params) {
        this->declare(param.name.lexeme, this->token_to_type(param.type));
        this->define(param.name.lexeme);
    }

    this->resolve(fun->body);
    this->end_scope();
}

void Resolver::resolve_struct(StructDeclStmt *s) {
    this->begin_scope();
    for (auto &prop : s->properties) {
        this->declare(prop.name.lexeme, this->token_to_type(prop.type));
        this->define(prop.name.lexeme);
    }

    for (auto &method : s->methods) {
        method->accept(*this);
    }

    this->end_scope();
}

void Resolver::define(std::string &name) {
    // TODO: handle case where not declared - should be bug
    this->scopes.back()[name].defined = true;
}

void Resolver::declare(std::string &name, std::shared_ptr<Type> type) {
    auto &scope = this->scopes.back();

    scope[name] = BoundVariable{
        name,
        false,
        type,
    };
}

// Expressions
void Resolver::visit_var_expr(VarExpr *expr) {
    // TODO: update error message here ad struct init
    auto resolved = this->resolve_local(expr->name);
    if (!resolved.defined)
        this->error(expr->name, "Cannot read local variable in its own initialiser.");

    expr->type = resolved.type;
}

void Resolver::visit_struct_init_expr(StructInitExpr *expr) {
    auto type = this->resolve_local(expr->name).type;
    if (auto t = std::dynamic_pointer_cast<StructType>(type)) {
        expr->type = t;
    } else {
        this->error(expr->name, "Cannot use struct initialiser on non-struct.");
    }
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
    this->resolve(expr->value.get());

    if (auto t = std::dynamic_pointer_cast<StructType>(expr->value->type)) {
        // TODO: use hashmap or lookup function
        auto name = expr->name.lexeme;
        auto p = std::find_if(t->props.begin(), t->props.end(), [name](const auto &t) {
            return std::get<0>(t).lexeme == name;
        });

        if (p != t->props.end()) {
            expr->type = std::get<1>(*p);
        } else {
            // TODO: handle error properly, and report which is missing
            this->error(expr->name, "Could not find property on struct.");
        }
    } else {
        this->error(expr->name, "Cannot get property on non-struct.");
    }
}

void Resolver::visit_call_expr(CallExpr *expr) {
    this->resolve(expr->callee.get());

    // TODO: check this
    if (auto t = std::dynamic_pointer_cast<FunctionType>(expr->callee->type)) {
        expr->type = t->return_type;
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
    expr->type = this->token_to_type(expr->literal);
}

std::shared_ptr<Type> Resolver::token_to_type(Token type) {
    switch (type.t) {
        case TokenType::TYPE: {
            if (type.lexeme == "str")
                return std::make_unique<StrType>();

            if (type.lexeme == "int")
                return std::make_unique<IntType>();

            if (type.lexeme == "float")
                return std::make_unique<FloatType>();

            if (type.lexeme == "bool")
                return std::make_unique<BoolType>();

            // TODO: only allow for return type
            if (type.lexeme == "void")
                return std::make_unique<VoidType>();
        }
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
        case TokenType::IDENTIFIER: {
            return this->resolve_local(type).type;
        }
        default:
            std::cerr << "[BUG] Unrecognised type of token (" << get_token_type_str(type.t) << ", '" << type.lexeme << "')." << std::endl;
            exit(3);
    }
}

// Statements
void Resolver::visit_fun_decl_stmt(FunDeclStmt *stmt) {
    auto return_type = this->token_to_type(stmt->return_type);

    // TODO: move this to FunDeclStmt? To match with StructDeclStmt
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> params;
    std::transform(stmt->params.begin(), stmt->params.end(), std::back_inserter(params), [this](TypedVar param) {
        return std::make_tuple(param.name, this->token_to_type(param.type));
    });
    auto func_type = std::make_unique<FunctionType>(
        stmt->name,
        params,
        std::move(return_type));
    this->declare(stmt->name.lexeme, std::move(func_type));
    this->define(stmt->name.lexeme);

    this->resolve_function(stmt);
}

void Resolver::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    this->declare(stmt->name.lexeme, stmt->get_type());
    this->define(stmt->name.lexeme);

    this->resolve_struct(stmt);
}

// TODO:
void Resolver::visit_enum_decl_stmt(EnumDeclStmt *stmt) {}

void Resolver::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    this->declare(stmt->name.name.lexeme, this->token_to_type(stmt->name.type));
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

// TODO:
void Resolver::visit_match_stmt(MatchStmt *stmt) {}

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
    stmt->target_type = this->resolve_local(stmt->name).type;
}
