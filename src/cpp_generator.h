#pragma once

#include "expr_visitor.h"
#include "stmt_visitor.h"
#include <fstream>

class CppGenerator : public ExprVisitor, public StmtVisitor {
  private:
    std::ostream &output;

  public:
    CppGenerator(std::ostream &file);

    void generate(std::vector<std::unique_ptr<Stmt>> &stmts);

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
