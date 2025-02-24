#include "resolver.h"
#include "type.h"

#include <iostream>
#include <memory>

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
        this->declare(param.name.lexeme, param.get_type());
        this->define(param.name.lexeme);
    }

    this->resolve(fun->body);
    this->end_scope();
}

void Resolver::resolve_struct(StructDeclStmt *s) {
    this->begin_scope();
    for (auto &prop : s->properties) {
        this->declare(prop.name.lexeme, prop.get_type());
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
    if (!scopes.back()[expr->name.lexeme].defined)
        this->error(expr->name, "Cannot read local variable in its own initialiser.");

    expr->type = this->resolve_local(expr->name).type;
}

void Resolver::visit_struct_init_expr(StructInitExpr *expr) {
    auto type = this->resolve_local(expr->name).type;
    if (auto t = std::dynamic_pointer_cast<UserDefinedType>(type)) {
        expr->type = t;
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
}

void Resolver::visit_call_expr(CallExpr *expr) {
    this->resolve(expr->callee.get());

    for (auto &arg : expr->args) {
        this->resolve(arg.get());
    }
}

void Resolver::visit_grouping_expr(GroupingExpr *expr) {
    this->resolve(expr->expr.get());
}

// NOTE: do nothing here - no variables to resolve
void Resolver::visit_literal_expr(LiteralExpr *expr) {}

// Statements
void Resolver::visit_fun_decl_stmt(FunDeclStmt *stmt) {
    // TODO: Type for this
    // this->declare(stmt->name);
    // this->define(stmt->name);
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
    this->declare(stmt->name.name.lexeme, stmt->name.get_type());
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
