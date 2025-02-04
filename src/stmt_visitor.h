#pragma once

#include "stmt.h"

class StmtVisitor {
  public:
    virtual void visit_fun_decl_stmt(FunDeclStmt *stmt) = 0;
    virtual void visit_struct_decl_stmt(StructDeclStmt *stmt) = 0;
    virtual void visit_enum_decl_stmt(EnumDeclStmt *stmt) = 0;
    virtual void visit_variable_decl_stmt(VariableDeclStmt *stmt) = 0;
    virtual void visit_expr_stmt(ExprStmt *stmt) = 0;
    virtual void visit_block_stmt(BlockStmt *stmt) = 0;
    virtual void visit_if_stmt(IfStmt *stmt) = 0;
    virtual void visit_match_stmt(MatchStmt *stmt) = 0;
    virtual void visit_while_stmt(WhileStmt *stmt) = 0;
    virtual void visit_for_stmt(ForStmt *stmt) = 0;
    virtual void visit_print_stmt(PrintStmt *stmt) = 0;
    virtual void visit_return_stmt(ReturnStmt *stmt) = 0;
    virtual void visit_assign_stmt(AssignStmt *stmt) = 0;
};
