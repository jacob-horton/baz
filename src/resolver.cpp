#include "resolver.h"
#include "stmt.h"
#include "token.h"
#include "type.h"

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

void Resolver::beginScope() {
    this->scopes.push_back({});
}

void Resolver::endScope() {
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

BoundVariable Resolver::resolveLocal(Token name) {
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

void Resolver::resolveFunction(FunDeclStmt *fun) {
    this->beginScope();
    for (auto param : fun->params) {
        this->declare(param);
        this->define(param);
    }

    this->resolve(fun->body);
    this->endScope();
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
    std::shared_ptr<Type> type = from_typed_var(var);

    scope[var.name.lexeme] = BoundVariable{
        var.name.lexeme,
        false,
        // TODO: actually work this out from var.type
        type,
    };
}

// Expressions
void Resolver::visitVarExpr(VarExpr *expr) {
    if (this->scopes.size() != 0 && !scopes.back()[expr->name.lexeme].defined)
        this->error(expr->name, "Cannot read local variable in its own initialiser.");

    expr->type = this->resolveLocal(expr->name).type;
}

// TODO:
void Resolver::visitStructInitExpr(StructInitExpr *expr) {}

void Resolver::visitBinaryExpr(BinaryExpr *expr) {
    this->resolve(expr->left.get());
    this->resolve(expr->right.get());
}

void Resolver::visitLogicalBinaryExpr(LogicalBinaryExpr *expr) {
    this->resolve(expr->left.get());
    this->resolve(expr->right.get());
}

void Resolver::visitUnaryExpr(UnaryExpr *expr) {
    this->resolve(expr->right.get());
}

void Resolver::visitGetExpr(GetExpr *expr) {
    this->resolve(expr->value.get());
}

void Resolver::visitCallExpr(CallExpr *expr) {
    this->resolve(expr->callee.get());

    for (auto &arg : expr->args) {
        this->resolve(arg.get());
    }
}

void Resolver::visitGroupingExpr(GroupingExpr *expr) {
    this->resolve(expr->expr.get());
}

// NOTE: do nothing here - no variables to resolve
void Resolver::visitLiteralExpr(LiteralExpr *expr) {}

// Statements
void Resolver::visitFunDeclStmt(FunDeclStmt *stmt) {
    // TODO: Type for this
    // this->declare(stmt->name);
    // this->define(stmt->name);
    this->resolveFunction(stmt);
}

// TODO:
void Resolver::visitStructDeclStmt(StructDeclStmt *stmt) {}

// TODO:
void Resolver::visitEnumDeclStmt(EnumDeclStmt *stmt) {}

void Resolver::visitVariableDeclStmt(VariableDeclStmt *stmt) {
    this->declare(stmt->name);
    this->resolve(stmt->initialiser.get());
    this->define(stmt->name);
}

void Resolver::visitExprStmt(ExprStmt *stmt) {
    this->resolve(stmt->expr.get());
}

void Resolver::visitBlockStmt(BlockStmt *stmt) {
    this->beginScope();
    this->resolve(stmt->stmts);
    this->endScope();
}

void Resolver::visitIfStmt(IfStmt *stmt) {
    this->resolve(stmt->condition.get());
    this->resolve(stmt->true_block);

    if (stmt->false_block.has_value())
        this->resolve(stmt->false_block.value());
}

// TODO:
void Resolver::visitMatchStmt(MatchStmt *stmt) {}

void Resolver::visitWhileStmt(WhileStmt *stmt) {
    this->resolve(stmt->condition.get());
    this->resolve(stmt->stmts);
}

void Resolver::visitForStmt(ForStmt *stmt) {
    this->resolve(stmt->var.get());
    this->resolve(stmt->condition.get());
    this->resolve(stmt->increment.get());
    this->resolve(stmt->stmts);
}

void Resolver::visitPrintStmt(PrintStmt *stmt) {
    if (stmt->expr.has_value())
        this->resolve(stmt->expr.value().get());
}

void Resolver::visitReturnStmt(ReturnStmt *stmt) {
    if (stmt->expr.has_value())
        this->resolve(stmt->expr.value().get());
}

void Resolver::visitAssignStmt(AssignStmt *stmt) {
    this->resolve(stmt->value.get());
    stmt->target_type = this->resolveLocal(stmt->name).type;
}
