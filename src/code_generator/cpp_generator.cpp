#include "cpp_generator.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>

std::string baz_to_cpp_type(Token type) {
    if (type.t == TokenType::TYPE && type.lexeme == "str") {
        return "std::string";
    }

    return type.lexeme;
}

CppGenerator::CppGenerator(std::ostream &output) : output(output) {}

void CppGenerator::generate(std::vector<std::unique_ptr<Stmt>> &stmts) {
    output << "#include <iostream>" << std::endl
           << "#include <variant>"
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
    this->output << "new " << expr->name.lexeme << "{";

    if (auto t = std::dynamic_pointer_cast<StructType>(expr->type)) {
        if (expr->properties.size() != t->props.size()) {
            // TODO: handle error properly, and report which are missing/extra
            std::cerr << "[BUG] Incorrect number of properties." << std::endl;
            exit(3);
        }

        // Loop through in order of declared type props
        for (auto &prop : t->props) {
            auto name = std::get<0>(prop).lexeme;

            // Find corresponding property
            // TODO: use hashmap or lookup function
            auto p = std::find_if(expr->properties.begin(), expr->properties.end(), [name](const auto &t) {
                return std::get<0>(t).lexeme == name;
            });

            if (p != expr->properties.end()) {
                std::get<1>(*p)->accept(*this);
                this->output << ", ";
            } else {
                std::cerr << "[BUG] Not all properties initialised. This should be checked in the type checker." << std::endl;
                exit(3);
            }
        }
    } else {
        std::cerr << "[BUG] Trying to initialise non-struct. This should be checked in the type checker" << std::endl;
        exit(3);
    }

    this->output << "}";
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
    this->output << "(";
    expr->value->accept(*this);
    this->output << "->" << expr->name.lexeme << ")";
}

void CppGenerator::visit_enum_init_expr(EnumInitExpr *expr) {
    if (VarExpr *e = dynamic_cast<VarExpr *>(expr->enum_namespace.get())) {
        this->output << "new " << e->name.lexeme << "(Baz_" << e->name.lexeme << expr->name.lexeme << "{";

        if (expr->payload.has_value())
            expr->payload.value()->accept(*this);

        this->output << "})";
    } else {
        std::cerr << "[BUG] trying to initialise variant of non-enum." << std::endl;
        exit(3);
    }
}

void CppGenerator::visit_call_expr(CallExpr *expr) {
    this->output << "(";

    // If we are doing x.y()
    if (auto *enum_variant = dynamic_cast<GetExpr *>(expr->callee.get())) {
        // And x is an enum type
        if (auto t = std::dynamic_pointer_cast<EnumType>(enum_variant->value->type)) {
            // We calling a method on the enum - need to call `Baz_EnumName_methodName(enum_variable, other, args, ...)`
            this->output << "Baz_" << t->name.lexeme << "_" << enum_variant->name.lexeme << "(";
            enum_variant->value->accept(*this);

            for (auto &arg : expr->args) {
                this->output << ", ";
                arg->accept(*this);
            }

            this->output << "))";
            return;
        }
    }

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
    if (expr->literal.t == TokenType::STR_VAL) {
        this->output << "std::string(" << expr->literal.lexeme << ")";
        return;
    }

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

        this->output << baz_to_cpp_type(param.type) << " " << param.name.lexeme;
        first = false;
    }

    this->output << ") {" << std::endl;

    for (auto &line : stmt->body) {
        line->accept(*this);
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_enum_method_decl_stmt(EnumMethodDeclStmt *enum_stmt) {
    auto &stmt = enum_stmt->fun_definition;
    std::string return_type = stmt->return_type.lexeme;

    this->output << return_type << " Baz_" << enum_stmt->enum_name.lexeme << "_" << stmt->name.lexeme << "(" << enum_stmt->enum_name.lexeme << " *baz_this";

    for (auto &param : stmt->params) {
        this->output << ", " << baz_to_cpp_type(param.type) << " " << param.name.lexeme;
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

void CppGenerator::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    for (auto &variant : stmt->variants) {
        this->output << "struct Baz_" << stmt->name.lexeme << variant.name.lexeme << "{ ";
        if (variant.payload_type.has_value())
            this->output << baz_to_cpp_type(variant.payload_type.value()) << " value; ";

        this->output << "};" << std::endl;
    }

    this->output << "using " << stmt->name.lexeme << " = std::variant<";
    for (int i = 0; i < stmt->variants.size(); i++) {
        this->output << "Baz_" << stmt->name.lexeme << stmt->variants[i].name.lexeme;
        if (i < stmt->variants.size() - 1)
            this->output << ",";
    }
    this->output << ">;" << std::endl;

    // TODO:
    for (auto &method : stmt->methods) {
        this->output << std::endl;
        method->accept(*this);
    }
}

void CppGenerator::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    // Pointer if user defined type
    auto pointer = stmt->name.type.t == TokenType::IDENTIFIER ? "*" : "";

    this->output << baz_to_cpp_type(stmt->name.type) << pointer << " " << stmt->name.name.lexeme << " = ";
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
