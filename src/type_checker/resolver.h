#pragma once

#include "../ast/expr_visitor.h"
#include "../ast/stmt_visitor.h"
#include "../type_checker/type.h"

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

    void begin_scope();
    void end_scope();

    void resolve(std::vector<std::unique_ptr<Stmt>> &stmts);
    void resolve(Stmt *stmt);
    void resolve(Expr *expr);
    void resolve_function(FunDeclStmt *fun);
    void resolve_struct(StructDeclStmt *s);

    BoundVariable resolve_local(Token name);

    std::shared_ptr<Type> token_to_type(Token name);

    void declare(std::string &name, std::shared_ptr<Type> type);
    void define(std::string &name);

    void visit_var_expr(VarExpr *expr);
    void visit_struct_init_expr(StructInitExpr *expr);
    void visit_binary_expr(BinaryExpr *expr);
    void visit_logical_binary_expr(LogicalBinaryExpr *expr);
    void visit_unary_expr(UnaryExpr *expr);
    void visit_get_expr(GetExpr *expr);
    void visit_call_expr(CallExpr *expr);
    void visit_grouping_expr(GroupingExpr *expr);
    void visit_literal_expr(LiteralExpr *expr);

    void visit_fun_decl_stmt(FunDeclStmt *stmt);
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
