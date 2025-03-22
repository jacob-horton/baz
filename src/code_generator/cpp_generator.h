#pragma once

#include "../ast/expr_visitor.h"
#include "../ast/stmt_visitor.h"

#include <fstream>

class CppGenerator : public ExprVisitor, public StmtVisitor {
  private:
    std::ostream &output;
    std::string this_keyword;
    std::map<std::string, std::shared_ptr<Type>> type_env;

  public:
    CppGenerator(std::ostream &file, std::map<std::string, std::shared_ptr<Type>> type_env);

    void generate(std::vector<std::unique_ptr<Stmt>> &stmts);

    void visit_var_expr(VarExpr *expr);
    void visit_struct_init_expr(StructInitExpr *expr);
    void visit_binary_expr(BinaryExpr *expr);
    void visit_logical_binary_expr(LogicalBinaryExpr *expr);
    void visit_unary_expr(UnaryExpr *expr);
    void visit_get_expr(GetExpr *expr);
    void visit_enum_init_expr(EnumInitExpr *expr);
    void visit_call_expr(CallExpr *expr);
    void visit_grouping_expr(GroupingExpr *expr);
    void visit_literal_expr(LiteralExpr *expr);

    void visit_fun_decl_stmt(FunDeclStmt *stmt);
    void visit_enum_method_decl_stmt(EnumMethodDeclStmt *stmt);
    void visit_struct_decl_stmt(StructDeclStmt *stmt);
    void visit_enum_decl_stmt(EnumDeclStmt *stmt);
    void visit_variable_decl_stmt(VariableDeclStmt *stmt);
    void visit_expr_stmt(ExprStmt *stmt);
    void visit_block_stmt(BlockStmt *stmt);
    void visit_if_stmt(IfStmt *stmt);
    void visit_match_stmt(MatchStmt *stmt);
    void visit_while_stmt(WhileStmt *stmt);
    void visit_for_stmt(ForStmt *stmt);
    void visit_print_stmt(PrintStmt *stmt);
    void visit_return_stmt(ReturnStmt *stmt);
    void visit_assign_stmt(AssignStmt *stmt);
};
