#pragma once

#include "expr.h"

// Visitor pattern interface
class ExprVisitor {
  public:
    virtual void visit_var_expr(VarExpr *expr) = 0;
    virtual void visit_struct_init_expr(StructInitExpr *expr) = 0;
    virtual void visit_binary_expr(BinaryExpr *expr) = 0;
    virtual void visit_unary_expr(UnaryExpr *expr) = 0;
    virtual void visit_get_expr(GetExpr *expr) = 0;
    virtual void visit_enum_init_expr(EnumInitExpr *expr) = 0;
    virtual void visit_call_expr(CallExpr *expr) = 0;
    virtual void visit_grouping_expr(GroupingExpr *expr) = 0;
    virtual void visit_literal_expr(LiteralExpr *expr) = 0;
};
