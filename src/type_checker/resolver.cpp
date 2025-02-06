#include "resolver.h"

#include <iostream>
#include <memory>

Resolver::Resolver() {}

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
    std::cerr << "[BUG] could not find variable." << std::endl;
    exit(3);
}

void Resolver::resolve_function(FunDeclStmt *fun) {
    this->begin_scope();
    for (auto param : fun->params) {
        this->declare(param);
        this->define(param);
    }

    this->resolve(fun->body);
    this->end_scope();
}

void Resolver::define(TypedVar var) {
    // TODO: how to handle global variables
    if (this->scopes.size() == 0)
        return;

    // TODO: handle case where not declared - should be bug
    this->scopes.back()[var.name.lexeme].defined = true;
}

void Resolver::declare(TypedVar var) {
    // TODO: is this correct - are top-level functions global variables?
    if (scopes.size() == 0) {
        std::cerr << "[BUG] Global variables should not be able to be defined." << std::endl;
        exit(3);
    }

    auto &scope = this->scopes.back();

    scope[var.name.lexeme] = BoundVariable{
        var.name.lexeme,
        false,
        // TODO: actually work this out from var.type
        var.get_type(),
    };
}

// Expressions
void Resolver::visit_var_expr(VarExpr *expr) {
    if (this->scopes.size() != 0 && !scopes.back()[expr->name.lexeme].defined)
        this->error(expr->name, "Cannot read local variable in its own initialiser.");

    expr->type = this->resolve_local(expr->name).type;
}

// TODO:
void Resolver::visit_struct_init_expr(StructInitExpr *expr) {}

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

// TODO:
void Resolver::visit_struct_decl_stmt(StructDeclStmt *stmt) {}

// TODO:
void Resolver::visit_enum_decl_stmt(EnumDeclStmt *stmt) {}

void Resolver::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    this->declare(stmt->name);
    this->resolve(stmt->initialiser.get());
    this->define(stmt->name);
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
