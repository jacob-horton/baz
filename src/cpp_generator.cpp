#include "cpp_generator.h"

#include <iostream>
#include <ostream>

// Expressions
void CppGenerator::visitVarExpr(VarExpr *expr) {
    std::cout << expr->name.lexeme;
}

void CppGenerator::visitStructInitExpr(StructInitExpr *expr) {
    // TODO: finish this
    std::cout << "struct init" << std::endl;
}

void CppGenerator::visitBinaryExpr(BinaryExpr *expr) {
    std::cout << "(";
    expr->left->accept(*this);
    std::cout << " " << expr->op.lexeme << " ";
    expr->right->accept(*this);
    std::cout << ")";
}

// TODO: is logical binary needed if we're just generating C++?
void CppGenerator::visitLogicalBinaryExpr(LogicalBinaryExpr *expr) {
    std::cout << "(";
    expr->left->accept(*this);
    std::cout << " " << expr->op.lexeme << " ";
    expr->right->accept(*this);
    std::cout << ")";
}

void CppGenerator::visitUnaryExpr(UnaryExpr *expr) {
    std::cout << "(";
    std::cout << expr->op.lexeme;
    expr->right->accept(*this);
    std::cout << ")";
}

void CppGenerator::visitGetExpr(GetExpr *expr) {
    // TODO: enums
    std::cout << "(";
    expr->value->accept(*this);
    std::cout << "." << expr->name.lexeme << ")";
}

void CppGenerator::visitCallExpr(CallExpr *expr) {
    // TODO: constructor
    std::cout << "(";
    expr->callee->accept(*this);
    std::cout << "(";

    bool first = true;
    for (auto &arg : expr->args) {
        if (!first)
            std::cout << ", ";

        arg->accept(*this);
        first = false;
    }

    std::cout << "))";
}

void CppGenerator::visitGroupingExpr(GroupingExpr *expr) {
    std::cout << "(";
    expr->expr->accept(*this);
    std::cout << ")";
}

void CppGenerator::visitLiteralExpr(LiteralExpr *expr) {
    std::cout << expr->literal.lexeme;
}

// Statments
void CppGenerator::visitFunDeclStmt(FunDeclStmt *stmt) {
    std::cout << stmt->return_type.lexeme << " " << stmt->name.lexeme << "(";

    bool first = true;
    for (auto &param : stmt->params) {
        if (!first)
            std::cout << ", ";

        std::cout << param.type.lexeme << " " << param.name.lexeme;
        first = false;
    }

    std::cout << ") {" << std::endl;

    for (auto &line : stmt->body) {
        line->accept(*this);
    }

    std::cout << "}" << std::endl;
}

void CppGenerator::visitStructDeclStmt(StructDeclStmt *stmt) {
    std::cout << "struct " << stmt->name.lexeme << " {" << std::endl;

    std::cout << "public:" << std::endl;
    for (auto &prop : stmt->properties) {
        std::cout << prop.type.lexeme << " " << prop.name.lexeme << ";" << std::endl;
    }

    for (auto &method : stmt->methods) {
        std::cout << std::endl;
        method->accept(*this);
    }

    std::cout << "};" << std::endl;
}

// TODO: enums
void CppGenerator::visitEnumDeclStmt(EnumDeclStmt *stmt) {}

void CppGenerator::visitVariableDeclStmt(VariableDeclStmt *stmt) {
    std::cout << stmt->name.type.lexeme << " " << stmt->name.name.lexeme << " = ";
    stmt->value->accept(*this);
    std::cout << ";" << std::endl;
}

void CppGenerator::visitExprStmt(ExprStmt *stmt) {
    stmt->expr->accept(*this);
    std::cout << ";" << std::endl;
}

void CppGenerator::visitBlockStmt(BlockStmt *stmt) {
    std::cout << "{" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    std::cout << "}" << std::endl;
}

void CppGenerator::visitIfStmt(IfStmt *stmt) {
    std::cout << "if (";
    stmt->condition->accept(*this);
    std::cout << ") {" << std::endl;

    for (auto &line : stmt->true_block) {
        line->accept(*this);
    }

    std::cout << "}";

    if (stmt->false_block.has_value()) {
        std::cout << " else {" << std::endl;
        for (auto &line : stmt->false_block.value()) {
            line->accept(*this);
        }

        std::cout << "}" << std::endl;
    }
}

// TODO: match statements
void CppGenerator::visitMatchStmt(MatchStmt *stmt) {}

void CppGenerator::visitWhileStmt(WhileStmt *stmt) {
    std::cout << "while (";
    stmt->condition->accept(*this);
    std::cout << ") {" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    std::cout << "}" << std::endl;
}

void CppGenerator::visitForStmt(ForStmt *stmt) {
    std::cout << "for (";

    stmt->var->accept(*this);
    stmt->condition->accept(*this);
    // TODO: don't include semicolon at end
    stmt->increment->accept(*this);

    std::cout << ") {" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    std::cout << "}" << std::endl;
}

void CppGenerator::visitPrintStmt(PrintStmt *stmt) {
    std::cout << "std::cout << ";

    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);

    if (stmt->newline)
        std::cout << " << std::endl";

    std::cout << ";" << std::endl;
}

void CppGenerator::visitReturnStmt(ReturnStmt *stmt) {
    std::cout << "return ";

    if (stmt->expr.has_value())
        stmt->expr.value()->accept(*this);

    std::cout << ";" << std::endl;
}

void CppGenerator::visitAssignStmt(AssignStmt *stmt) {
    std::cout << stmt->name.lexeme << " = ";
    stmt->value->accept(*this);
    std::cout << ";" << std::endl;
}
