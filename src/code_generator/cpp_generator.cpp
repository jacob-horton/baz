#include "cpp_generator.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>

std::string baz_to_cpp_type(Token type) {
    if (type.t == TokenType::TYPE && type.lexeme == "str") {
        return "std::string";
    }

    auto pointer = type.t == TokenType::IDENTIFIER ? "*" : "";
    return type.lexeme + pointer;
}

std::string enum_variant_name(std::string enum_name, std::string variant_name) {
    return "Baz_" + enum_name + variant_name;
}

std::string enum_method_name(std::string enum_name, std::string method_name) {
    return "Baz_" + enum_name + "_" + method_name;
}

CppGenerator::CppGenerator(std::ostream &output, std::map<std::string, std::shared_ptr<Type>> type_env) : output(output), this_keyword("this"), type_env(type_env) {}

void CppGenerator::generate(std::vector<std::unique_ptr<Stmt>> &stmts) {
    this->output << "#include <iostream>" << std::endl
                 << "#include <variant>" << std::endl
                 << std::endl;

    // Declare all struct names (including enum variants)
    for (auto type : this->type_env) {
        if (auto t = std::dynamic_pointer_cast<StructType>(std::get<1>(type))) {
            this->output << "struct " << std::get<0>(type) << ";" << std::endl;
        } else if (auto t = std::dynamic_pointer_cast<EnumType>(std::get<1>(type))) {
            for (auto variant : t->variants) {
                this->output << "struct " << enum_variant_name(std::get<0>(type), variant.name.lexeme) << ";" << std::endl;
            }
        }
    }

    // Declare all enums
    for (auto type : this->type_env) {
        if (auto t = std::dynamic_pointer_cast<EnumType>(std::get<1>(type))) {
            this->output << "using " << t->name.lexeme << " = std::variant<";
            for (int i = 0; i < t->variants.size(); i++) {
                this->output << enum_variant_name(t->name.lexeme, t->variants[i].name.lexeme);
                if (i < t->variants.size() - 1)
                    this->output << ",";
            }
            this->output << ">;" << std::endl;
        }
    }

    this->output << std::endl;

    for (auto &stmt : stmts) {
        stmt->accept(*this);
        this->output << std::endl;
    }
}

// Expressions
void CppGenerator::visit_var_expr(VarExpr *expr) {
    if (expr->name.lexeme == "this") {
        this->output << this->this_keyword;
        return;
    }
    this->output << expr->name.lexeme;
}

void CppGenerator::visit_struct_init_expr(StructInitExpr *expr) {
    this->output << "(new " << expr->name.lexeme << "{";

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

    this->output << "})";
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
    expr->object->accept(*this);
    this->output << "->" << expr->name.lexeme << ")";
}

void CppGenerator::visit_enum_init_expr(EnumInitExpr *expr) {
    if (VarExpr *enum_name = dynamic_cast<VarExpr *>(expr->enum_namespace.get())) {
        this->output << "(new " << enum_name->name.lexeme << "(" << enum_variant_name(enum_name->name.lexeme, expr->variant.lexeme) << "{";

        if (expr->payload.has_value())
            expr->payload.value()->accept(*this);

        this->output << "}))";
    } else {
        std::cerr << "[BUG] trying to initialise variant of non-enum." << std::endl;
        exit(3);
    }
}

void CppGenerator::visit_call_expr(CallExpr *expr) {
    // If we are doing x.y()
    if (auto *enum_method = dynamic_cast<GetExpr *>(expr->callee.get())) {
        // And x is an enum type
        if (auto t = std::dynamic_pointer_cast<EnumType>(enum_method->object->type)) {
            this->output << "(" << enum_method_name(t->name.lexeme, enum_method->name.lexeme) << "(";
            enum_method->object->accept(*this);

            for (auto &arg : expr->args) {
                this->output << ", ";
                arg->accept(*this);
            }

            this->output << "))";
            return;
        }
    }

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
    this->output << baz_to_cpp_type(stmt->return_type) << " " << enum_method_name(enum_stmt->enum_name.lexeme, stmt->name.lexeme) << "(" << enum_stmt->enum_name.lexeme << " *baz_this";

    for (auto &param : stmt->params) {
        this->output << ", " << baz_to_cpp_type(param.type) << " " << param.name.lexeme;
    }

    this->output << ") {" << std::endl;

    for (auto &line : stmt->body) {
        // Use "baz_this" instead of "this" as we are just using normal functions, not methods
        auto prev_this_keyword = this->this_keyword;
        this->this_keyword = "baz_this";

        line->accept(*this);

        this->this_keyword = prev_this_keyword;
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    this->output << "struct " << stmt->name.lexeme << " {" << std::endl;

    this->output << "public:" << std::endl;
    for (auto &prop : stmt->properties) {
        this->output << baz_to_cpp_type(prop.type) << " " << prop.name.lexeme << ";" << std::endl;
    }

    for (auto &method : stmt->methods) {
        this->output << std::endl;
        method->accept(*this);
    }

    this->output << "};" << std::endl;
}

void CppGenerator::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    for (auto &variant : stmt->variants) {
        this->output << "struct " << enum_variant_name(stmt->name.lexeme, variant.name.lexeme) << "{ ";
        if (variant.payload_type.has_value())
            this->output << baz_to_cpp_type(variant.payload_type.value()) << " value; ";

        this->output << "};" << std::endl;
    }

    for (auto &method : stmt->methods) {
        this->output << std::endl;
        method->accept(*this);
    }
}

void CppGenerator::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    // Pointer if user defined type
    this->output << baz_to_cpp_type(stmt->name.type) << " " << stmt->name.name.lexeme << " = ";
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

void CppGenerator::visit_match_stmt(MatchStmt *stmt) {
    // TODO: make sure this is a unique name
    std::string target_var("baz_enum_target");
    this->output << "auto " << target_var << " = *";
    stmt->target->accept(*this);
    this->output << ";" << std::endl;

    bool first = true;
    for (auto &branch : stmt->branches) {
        auto pattern_variant = enum_variant_name(branch.pattern.enum_type.lexeme, branch.pattern.enum_variant.lexeme);

        auto if_keyword = first ? "if" : "else if";
        this->output << if_keyword << "(std::holds_alternative<" << pattern_variant << ">(" << target_var << ")) {" << std::endl;

        if (branch.pattern.bound_variable.has_value()) {
            this->output << "auto " << branch.pattern.bound_variable.value()->name.lexeme << " = std::get<" << pattern_variant << ">(" << target_var << ").value;" << std::endl;
        }

        for (auto &stmt : branch.body) {
            stmt->accept(*this);
        }

        this->output << "}" << std::endl;
        first = false;
    }
}

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
    if (stmt->name.lexeme == "this") {
        // TODO: is this what we want
        // If we're reassigning "this", need to dereference
        this->output << "*" << this->this_keyword << " = *";
    } else {
        this->output << stmt->name.lexeme << " = ";
    }

    stmt->value->accept(*this);

    if (stmt->semicolon)
        this->output << ";" << std::endl;
}
