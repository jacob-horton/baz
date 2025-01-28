#pragma once

#include "expr.h"

class ExprVisitor {
  public:
    virtual void visitVarExpr(VarExpr *expr) = 0;
    virtual void visitStructInitExpr(StructInitExpr *expr) = 0;
    virtual void visitBinaryExpr(BinaryExpr *expr) = 0;
    virtual void visitLogicalBinaryExpr(LogicalBinaryExpr *expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr *expr) = 0;
    virtual void visitGetExpr(GetExpr *expr) = 0;
    virtual void visitCallExpr(CallExpr *expr) = 0;
    virtual void visitGroupingExpr(GroupingExpr *expr) = 0;
    virtual void visitLiteralExpr(LiteralExpr *expr) = 0;
};
