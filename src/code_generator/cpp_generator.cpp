#include "cpp_generator.h"

#include <iostream>
#include <ostream>

CppGenerator::CppGenerator(std::ostream &output) : output(output) {}

void CppGenerator::generate(std::vector<std::unique_ptr<Stmt>> &stmts) {
    output << "#include <iostream>"
           << std::endl
           << std::endl;

    for (auto &stmt : stmts) {
        stmt->accept(*this);
        this->output << std::endl;
    }
}

// Expressions
void CppGenerator::visit_var_expr(VarExpr *expr) {
    this->output << expr->name.lexeme;
}

void CppGenerator::visit_struct_init_expr(StructInitExpr *expr) {
    // TODO: finish this
    this->output << "struct init" << std::endl;
}

void CppGenerator::visit_binary_expr(BinaryExpr *expr) {
    this->output << "(";
    expr->left->accept(*this);
    this->output << " " << expr->op.lexeme << " ";
    expr->right->accept(*this);
    this->output << ")";
}

// TODO: is logical binary needed if we're just generating C++?
void CppGenerator::visit_logical_binary_expr(LogicalBinaryExpr *expr) {
    this->output << "(";
    expr->left->accept(*this);
    this->output << " " << expr->op.lexeme << " ";
    expr->right->accept(*this);
    this->output << ")";
}

void CppGenerator::visit_unary_expr(UnaryExpr *expr) {
    this->output << "(";
    this->output << expr->op.lexeme;
    expr->right->accept(*this);
    this->output << ")";
}

void CppGenerator::visit_get_expr(GetExpr *expr) {
    // TODO: enums
    this->output << "(";
    expr->value->accept(*this);
    this->output << "." << expr->name.lexeme << ")";
}

void CppGenerator::visit_call_expr(CallExpr *expr) {
    // TODO: constructor
    this->output << "(";
    expr->callee->accept(*this);
    this->output << "(";

    bool first = true;
    for (auto &arg : expr->args) {
        if (!first)
            this->output << ", ";

        arg->accept(*this);
        first = false;
    }

    this->output << "))";
}

void CppGenerator::visit_grouping_expr(GroupingExpr *expr) {
    this->output << "(";
    expr->expr->accept(*this);
    this->output << ")";
}

void CppGenerator::visit_literal_expr(LiteralExpr *expr) {
    this->output << expr->literal.lexeme;
}

// Statements
void CppGenerator::visit_fun_decl_stmt(FunDeclStmt *stmt) {
    std::string return_type = stmt->return_type.lexeme;

    // Convert main to use "int" instead of "void" for main
    if (stmt->fun_type == FunType::FUNCTION && stmt->name.lexeme == "main")
        return_type = "int";

    this->output << return_type << " " << stmt->name.lexeme << "(";

    bool first = true;
    for (auto &param : stmt->params) {
        if (!first)
            this->output << ", ";

        this->output << param.type.lexeme << " " << param.name.lexeme;
        first = false;
    }

    this->output << ") {" << std::endl;

    for (auto &line : stmt->body) {
        line->accept(*this);
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    this->output << "struct " << stmt->name.lexeme << " {" << std::endl;

    this->output << "public:" << std::endl;
    for (auto &prop : stmt->properties) {
        this->output << prop.type.lexeme << " " << prop.name.lexeme << ";" << std::endl;
    }

    for (auto &method : stmt->methods) {
        this->output << std::endl;
        method->accept(*this);
    }

    this->output << "};" << std::endl;
}

// TODO: enums
void CppGenerator::visit_enum_decl_stmt(EnumDeclStmt *stmt) {}

void CppGenerator::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    this->output << stmt->name.type.lexeme << " " << stmt->name.name.lexeme << " = ";
    stmt->initialiser->accept(*this);
    this->output << ";" << std::endl;
}

void CppGenerator::visit_expr_stmt(ExprStmt *stmt) {
    stmt->expr->accept(*this);
    this->output << ";" << std::endl;
}

void CppGenerator::visit_block_stmt(BlockStmt *stmt) {
    this->output << "{" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_if_stmt(IfStmt *stmt) {
    this->output << "if (";
    stmt->condition->accept(*this);
    this->output << ") {" << std::endl;

    for (auto &line : stmt->true_block) {
        line->accept(*this);
    }

    this->output << "}";

    if (stmt->false_block.has_value()) {
        this->output << " else {" << std::endl;
        for (auto &line : stmt->false_block.value()) {
            line->accept(*this);
        }

        this->output << "}" << std::endl;
    }
}

// TODO: match statements
void CppGenerator::visit_match_stmt(MatchStmt *stmt) {}

void CppGenerator::visit_while_stmt(WhileStmt *stmt) {
    this->output << "while (";
    stmt->condition->accept(*this);
    this->output << ") {" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_for_stmt(ForStmt *stmt) {
    this->output << "for (";

    stmt->var->accept(*this);
    stmt->condition->accept(*this);
    // TODO: don't include semicolon at end
    stmt->increment->accept(*this);

    this->output << ") {" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_print_stmt(PrintStmt *stmt) {
    this->output << "std::cout << ";

    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);

    if (stmt->newline)
        this->output << " << std::endl";

    this->output << ";" << std::endl;
}

void CppGenerator::visit_return_stmt(ReturnStmt *stmt) {
    this->output << "return ";

    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);

    this->output << ";" << std::endl;
}

void CppGenerator::visit_assign_stmt(AssignStmt *stmt) {
    this->output << stmt->name.lexeme << " = ";
    stmt->value->accept(*this);

    if (stmt->semicolon)
        this->output << ";" << std::endl;
}
