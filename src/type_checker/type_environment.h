#pragma once

#include "../ast/expr_visitor.h"
#include "../ast/stmt_visitor.h"
#include "../type_checker/type.h"

#include <fstream>
#include <map>
#include <memory>
#include <string>

// Only implements `StmtVisitor` as it does not need to check expressions
class TypeEnvironment : public StmtVisitor {
  public:
    std::map<std::string, std::shared_ptr<Type>> type_env;

    TypeEnvironment();

    void generate_type_env(std::vector<std::unique_ptr<Stmt>> &stmts);

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
    void visit_panic_stmt(PanicStmt *stmt);
    void visit_return_stmt(ReturnStmt *stmt);
    void visit_assign_stmt(AssignStmt *stmt);
    void visit_set_stmt(SetStmt *stmt);
};
