#pragma once

#include "expr_visitor.h"
#include "stmt_visitor.h"
#include "type.h"

#include <fstream>
#include <map>
#include <string>

// TODO: rename? Maybe ResolvedVariable
struct BoundVariable {
    std::string name;
    bool defined;
    std::shared_ptr<Type> type;
};

class Resolver : public ExprVisitor, public StmtVisitor {
  private:
    std::vector<std::map<std::string, BoundVariable>> scopes;

  public:
    Resolver();

    void error(Token t, std::string message);

    void beginScope();
    void endScope();

    // TODO: can we remove these?
    void resolve(std::vector<std::unique_ptr<Stmt>> &stmts);
    void resolve(Stmt *stmt);
    void resolve(Expr *expr);
    void resolveFunction(FunDeclStmt *fun);

    BoundVariable resolveLocal(Token name);

    void declare(TypedVar var);
    void define(TypedVar var);

    void visitVarExpr(VarExpr *expr);
    void visitStructInitExpr(StructInitExpr *expr);
    void visitBinaryExpr(BinaryExpr *expr);
    void visitLogicalBinaryExpr(LogicalBinaryExpr *expr);
    void visitUnaryExpr(UnaryExpr *expr);
    void visitGetExpr(GetExpr *expr);
    void visitCallExpr(CallExpr *expr);
    void visitGroupingExpr(GroupingExpr *expr);
    void visitLiteralExpr(LiteralExpr *expr);

    void visitFunDeclStmt(FunDeclStmt *stmt);
    void visitStructDeclStmt(StructDeclStmt *stmt);
    void visitEnumDeclStmt(EnumDeclStmt *stmt);
    void visitVariableDeclStmt(VariableDeclStmt *stmt);
    void visitExprStmt(ExprStmt *stmt);
    void visitBlockStmt(BlockStmt *stmt);
    void visitIfStmt(IfStmt *stmt);
    void visitMatchStmt(MatchStmt *stmt);
    void visitWhileStmt(WhileStmt *stmt);
    void visitForStmt(ForStmt *stmt);
    void visitPrintStmt(PrintStmt *stmt);
    void visitReturnStmt(ReturnStmt *stmt);
    void visitAssignStmt(AssignStmt *stmt);
};
