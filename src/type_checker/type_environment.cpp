#include "type_environment.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <tuple>

TypeEnvironment::TypeEnvironment() {
    // Add primitives
    this->type_env["int"] = std::make_unique<IntType>();
    this->type_env["float"] = std::make_unique<FloatType>();
    this->type_env["bool"] = std::make_unique<BoolType>();
    this->type_env["null"] = std::make_unique<NullType>();
    this->type_env["str"] = std::make_unique<StrType>();
    this->type_env["void"] = std::make_unique<VoidType>();
}

void TypeEnvironment::generate_type_env(std::vector<std::unique_ptr<Stmt>> &stmts) {
    for (auto &stmt : stmts) {
        stmt->accept(*this);
    }
}

// Expressions
void TypeEnvironment::visit_var_expr(VarExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_struct_init_expr(StructInitExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_binary_expr(BinaryExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_logical_binary_expr(LogicalBinaryExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_unary_expr(UnaryExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_get_expr(GetExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_enum_init_expr(EnumInitExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_call_expr(CallExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_grouping_expr(GroupingExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_literal_expr(LiteralExpr *expr) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

// Statements
void TypeEnvironment::visit_fun_decl_stmt(FunDeclStmt *stmt) {}

void TypeEnvironment::visit_enum_method_decl_stmt(EnumMethodDeclStmt *stmt) {}

void TypeEnvironment::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> props;
    for (auto &prop : stmt->properties) {
        props.push_back(std::make_tuple(prop.name, this->type_env[prop.type.lexeme]));
    }

    std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods;
    for (auto &method : stmt->methods) {
        auto return_type = this->type_env[method->return_type.lexeme];

        std::vector<std::tuple<Token, std::shared_ptr<Type>>> params;
        std::transform(method->params.begin(), method->params.end(), std::back_inserter(params), [this](TypedVar param) {
            return std::make_tuple(param.name, this->type_env[param.type.lexeme]);
        });
        std::shared_ptr<Type> func_type = std::make_unique<FunctionType>(
            stmt->name,
            params,
            std::move(return_type));

        methods.push_back(std::make_tuple(method->name, func_type));
    }

    this->type_env[stmt->name.lexeme] = std::make_unique<StructType>(stmt->name, props, methods);
}

void TypeEnvironment::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    std::vector<std::tuple<Token, std::shared_ptr<Type>>> methods;
    for (auto &method : stmt->methods) {
        auto return_type = this->type_env[method->fun_definition->return_type.lexeme];

        std::vector<std::tuple<Token, std::shared_ptr<Type>>> params;
        std::transform(method->fun_definition->params.begin(), method->fun_definition->params.end(), std::back_inserter(params), [this](TypedVar param) {
            return std::make_tuple(param.name, this->type_env[param.type.lexeme]);
        });
        std::shared_ptr<Type> func_type = std::make_unique<FunctionType>(
            stmt->name,
            params,
            std::move(return_type));

        methods.push_back(std::make_tuple(method->fun_definition->name, func_type));
    }

    this->type_env[stmt->name.lexeme] = std::make_unique<EnumType>(stmt->name, stmt->variants, methods);
}

void TypeEnvironment::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_expr_stmt(ExprStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_block_stmt(BlockStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_if_stmt(IfStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_match_stmt(MatchStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_while_stmt(WhileStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_for_stmt(ForStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_print_stmt(PrintStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_return_stmt(ReturnStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}

void TypeEnvironment::visit_assign_stmt(AssignStmt *stmt) {
    std::cerr << "Unimplemented" << std::endl;
    exit(3);
}
