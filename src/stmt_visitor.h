#pragma once

#include "stmt.h"

class StmtVisitor {
  public:
    virtual void visitFunDeclStmt(FunDeclStmt *stmt) = 0;
    virtual void visitStructDeclStmt(StructDeclStmt *stmt) = 0;
    virtual void visitEnumDeclStmt(EnumDeclStmt *stmt) = 0;
    virtual void visitVariableDeclStmt(VariableDeclStmt *stmt) = 0;
    virtual void visitExprStmt(ExprStmt *stmt) = 0;
    virtual void visitBlockStmt(BlockStmt *stmt) = 0;
    virtual void visitIfStmt(IfStmt *stmt) = 0;
    virtual void visitMatchStmt(MatchStmt *stmt) = 0;
    virtual void visitWhileStmt(WhileStmt *stmt) = 0;
    virtual void visitForStmt(ForStmt *stmt) = 0;
    virtual void visitPrintStmt(PrintStmt *stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt *stmt) = 0;
    virtual void visitAssignStmt(AssignStmt *stmt) = 0;
};
