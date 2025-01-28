#pragma once

#include "expr.h"

class ExprVisitor {
  public:
    virtual void visitAssignExpr(AssignExpr *expr);
    virtual void visitVarExpr(VarExpr *expr);
    virtual void visitStructInitExpr(StructInitExpr *expr);
    virtual void visitBinaryExpr(BinaryExpr *expr);
    virtual void visitLogicalBinaryExpr(LogicalBinaryExpr *expr);
    virtual void visitUnaryExpr(UnaryExpr *expr);
    virtual void visitGetExpr(GetExpr *expr);
    virtual void visitCallExpr(CallExpr *expr);
    virtual void visitGroupingExpr(GroupingExpr *expr);
    virtual void visitLiteralExpr(LiteralExpr *expr);
};
