#include "cpp_generator.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <variant>

inline const std::string BAZ_NAMESPACE = "Baz";

std::string baz_to_cpp_type(Token type, bool optional) {
    if (type.t == TokenType::TYPE && type.lexeme == "str") {
        return optional ? "std::optional<std::string>" : "std::string";
    }

    auto pointer = type.t == TokenType::IDENTIFIER ? "*" : "";
    auto full_non_optional_type = type.lexeme + pointer;
    return optional ? "std::optional<" + full_non_optional_type + ">" : full_non_optional_type;
}

std::string enum_variant_name(std::string enum_name, std::string variant_name, bool namespaced = true) {
    return (namespaced ? BAZ_NAMESPACE + "::" : "") + enum_name + "_" + variant_name;
}

std::string enum_method_name(std::string enum_name, std::string method_name, bool namespaced = true) {
    return (namespaced ? BAZ_NAMESPACE + "::" : "") + enum_name + "_" + method_name;
}

CppGenerator::CppGenerator(std::ostream &output, std::map<std::string, std::shared_ptr<Type>> type_env) : output(output), this_keyword("this"), type_env(type_env) {}

void CppGenerator::generate(std::vector<std::unique_ptr<Stmt>> &stmts) {
    this->output << "#include <iostream>" << std::endl
                 << "#include <variant>" << std::endl
                 << "#include <string>" << std::endl
                 << "#include <optional>" << std::endl
                 << "#include <sstream>" << std::endl
                 << std::endl;

    // To-string function
    this->output << "namespace " << BAZ_NAMESPACE << " {" << std::endl;
    this->output << R"END(// to_string code from https://medium.com/@ryan_forrester_/how-to-convert-c-boolean-to-string-b4b2b3c36d68
    template<typename T>
    std::string to_string(const T& value) {
        // Normal converting element to string
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    // Specialization for bool
    template<>
    std::string to_string<bool>(const bool& value) {
        // Bool -> "true" or "false" instead of "1" and "0"
        return value ? "true" : "false";
    }
})END" << std::endl
                 << std::endl;

    // Declare all struct names (including enum variants)
    for (auto type : this->type_env) {
        if (auto t = std::dynamic_pointer_cast<StructType>(std::get<1>(type))) {
            this->output << "struct " << std::get<0>(type) << ";" << std::endl;
        } else if (auto t = std::dynamic_pointer_cast<EnumType>(std::get<1>(type))) {
            this->output << "namespace " << BAZ_NAMESPACE << " {" << std::endl;
            for (auto variant : t->variants) {
                this->output << "struct " << enum_variant_name(std::get<0>(type), variant.name.lexeme, false) << ";" << std::endl;
            }
            this->output << "}" << std::endl;
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

    if (auto t = std::dynamic_pointer_cast<StructType>(expr->get_type_info().type)) {
        if (expr->properties.size() != t->props.size()) {
            std::cerr << "[BUG] Incorrect number of properties. Should be checked by type checker." << std::endl;
            exit(3);
        }

        // Loop through in order of declared type props
        for (auto &prop : t->props) {
            auto name = prop.name.lexeme;

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

    if (expr->op.t == TokenType::QUESTION_QUESTION) {
        // If `??` operator, use `a.value_or(b)`
        expr->left->accept(*this);
        this->output << ".value_or(";
        expr->right->accept(*this);
        this->output << ")";
    } else {
        // Otherwise use `a <op> b`
        expr->left->accept(*this);
        this->output << " " << expr->op.lexeme << " ";
        expr->right->accept(*this);
    }

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

    if (expr->optional) {
        this->output << "{ auto temp = ";
        expr->object->accept(*this);
        this->output << "; temp.has_value() ? std::optional{temp.value()->" << expr->name.lexeme;
        this->output << "} : std::nullopt; }";
    } else {
        expr->object->accept(*this);
        this->output << "->" << expr->name.lexeme;
    }

    this->output << ")";
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
    // TODO: so much duplication - reduce (also reduce with get_expr)

    // If we are doing x.y()
    if (auto *get_expr = dynamic_cast<GetExpr *>(expr->callee.get())) {
        // And x is an enum type
        if (auto t = std::dynamic_pointer_cast<EnumType>(get_expr->object->get_type_info().type)) {
            this->output << "(";

            if (get_expr->optional) {
                // If calling a function that returns void, handle differently - no return value
                if (auto fun_t = std::dynamic_pointer_cast<FunctionType>(get_expr->get_type_info().type)) {
                    if (fun_t->return_type.lexeme == "void") {
                        this->output << "{ auto temp = ";
                        get_expr->object->accept(*this);
                        this->output << "; if (temp.has_value()) { ";

                        // Use namespaced enum method
                        this->output << enum_method_name(t->name.lexeme, get_expr->name.lexeme) << "(temp.value()";

                        for (auto &arg : expr->args) {
                            this->output << ", ";
                            arg->accept(*this);
                        }

                        this->output << "); }})";
                        return;
                    }
                }

                this->output << "{ auto temp = ";
                get_expr->object->accept(*this);
                this->output << "; temp.has_value() ? std::optional{";

                // Use namespaced enum method
                this->output << enum_method_name(t->name.lexeme, get_expr->name.lexeme) << "(temp.value()";

                for (auto &arg : expr->args) {
                    this->output << ", ";
                    arg->accept(*this);
                }

                this->output << ")} : std::nullopt; })";
                return;
            }

            // Use namespaced enum method
            this->output << enum_method_name(t->name.lexeme, get_expr->name.lexeme) << "(";
            get_expr->object->accept(*this);

            for (auto &arg : expr->args) {
                this->output << ", ";
                arg->accept(*this);
            }

            this->output << "))";
            return;
        } else if (auto t = std::dynamic_pointer_cast<StructType>(get_expr->object->get_type_info().type) && get_expr->get_type_info().optional) {
            this->output << "(";
            if (auto fun_t = std::dynamic_pointer_cast<FunctionType>(get_expr->get_type_info().type)) {
                if (fun_t->return_type.lexeme == "void") {
                    this->output << "{ auto temp = ";
                    get_expr->object->accept(*this);
                    this->output << "; if (temp.has_value()) { temp.value()->" << get_expr->name.lexeme << "(";

                    bool first = true;
                    for (auto &arg : expr->args) {
                        if (!first)
                            this->output << ", ";

                        arg->accept(*this);
                    }

                    this->output << "); }})";
                    return;
                }
            }

            this->output << "{ auto temp = ";
            get_expr->object->accept(*this);
            this->output << "; temp.has_value() ? std::optional{temp.value()->" << get_expr->name.lexeme << "(";

            bool first = true;
            for (auto &arg : expr->args) {
                if (!first)
                    this->output << ", ";

                arg->accept(*this);
                first = false;
            }

            this->output << ")} : std::nullopt; })";
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

    if (expr->literal.t == TokenType::NULL_VAL) {
        this->output << "std::nullopt";
        return;
    }

    this->output << expr->literal.lexeme;
}

// Statements
void CppGenerator::visit_fun_decl_stmt(FunDeclStmt *stmt) {
    std::string return_type = baz_to_cpp_type(stmt->return_type, stmt->return_type_optional);

    // Convert main to use "int" instead of "void" for main
    if (stmt->fun_type == FunType::FUNCTION && stmt->name.lexeme == "main")
        return_type = "int";

    this->output << return_type << " " << stmt->name.lexeme << "(";

    bool first = true;
    for (auto &param : stmt->params) {
        if (!first)
            this->output << ", ";

        this->output << baz_to_cpp_type(param.type, param.is_optional) << " " << param.name.lexeme;
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
    this->output << "namespace " << BAZ_NAMESPACE << " {" << std::endl;
    this->output << baz_to_cpp_type(stmt->return_type, stmt->return_type_optional) << " " << enum_method_name(enum_stmt->enum_name.lexeme, stmt->name.lexeme, false) << "(" << enum_stmt->enum_name.lexeme << " *baz_this";

    for (auto &param : stmt->params) {
        this->output << ", " << baz_to_cpp_type(param.type, param.is_optional) << " " << param.name.lexeme;
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
    this->output << "}" << std::endl;
}

void CppGenerator::visit_struct_decl_stmt(StructDeclStmt *stmt) {
    this->output << "struct " << stmt->name.lexeme << " {" << std::endl;

    this->output << "public:" << std::endl;
    for (auto &prop : stmt->properties) {
        this->output << baz_to_cpp_type(prop.type, prop.is_optional) << " " << prop.name.lexeme << ";" << std::endl;
    }

    for (auto &method : stmt->methods) {
        this->output << std::endl;
        method->accept(*this);
    }

    this->output << "};" << std::endl;
}

void CppGenerator::visit_enum_decl_stmt(EnumDeclStmt *stmt) {
    for (auto &variant : stmt->variants) {
        this->output << "namespace " << BAZ_NAMESPACE << " {" << std::endl;
        this->output << "struct " << enum_variant_name(stmt->name.lexeme, variant.name.lexeme, false) << "{ ";
        if (variant.payload_type.has_value())
            this->output << baz_to_cpp_type(variant.payload_type.value(), variant.is_optional) << " value; ";

        this->output << "};" << std::endl;
        this->output << "}" << std::endl;
    }

    for (auto &method : stmt->methods) {
        this->output << std::endl;
        method->accept(*this);
    }
}

void CppGenerator::visit_variable_decl_stmt(VariableDeclStmt *stmt) {
    // Pointer if user defined type
    this->output << baz_to_cpp_type(stmt->name.type, stmt->name.is_optional) << " " << stmt->name.name.lexeme << " = ";
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
    this->output << "auto " << target_var << " = ";
    stmt->target->accept(*this);
    this->output << ";" << std::endl;

    bool first = true;
    auto optional_getter = "";

    // Do null branch first
    if (stmt->target->get_type_info().optional) {
        first = false;
        this->output << "if (!" << target_var << ".has_value()) {" << std::endl;
        optional_getter = ".value()";

        auto branch = std::find_if(stmt->branches.begin(), stmt->branches.end(), [](const auto &b) {
            return std::holds_alternative<NullPattern>(b.pattern);
        });

        if (branch == stmt->branches.end()) {
            std::cerr << "[BUG] Optional target does not have null pattern branch." << std::endl;
            exit(3);
        }

        for (auto &stmt : branch->body) {
            stmt->accept(*this);
        }

        this->output << "}";
    }

    for (auto &branch : stmt->branches) {
        auto if_keyword = first ? "if" : "else if";
        if (std::holds_alternative<EnumPattern>(branch.pattern)) {
            auto &enum_pattern = std::get<EnumPattern>(branch.pattern);
            auto pattern_variant = enum_variant_name(enum_pattern.enum_type.lexeme, enum_pattern.enum_variant.lexeme);
            this->output << if_keyword << "(std::holds_alternative<" << pattern_variant << ">(*" << target_var << optional_getter << ")) {" << std::endl;

            if (enum_pattern.bound_variable.has_value()) {
                this->output << "auto " << enum_pattern.bound_variable.value()->name.lexeme << " = std::get<" << pattern_variant << ">(*" << target_var << optional_getter << ").value;" << std::endl;
            }
        } else if (std::holds_alternative<NullPattern>(branch.pattern)) {
            // Already handled
            continue;
        } else if (std::holds_alternative<CatchAllPattern>(branch.pattern)) {
            // Handled at end
            continue;
        }

        for (auto &stmt : branch.body) {
            stmt->accept(*this);
        }

        this->output << "}" << std::endl;
        first = false;
    }

    auto catch_all_branch = std::find_if(stmt->branches.begin(), stmt->branches.end(), [](const auto &b) {
        return std::holds_alternative<CatchAllPattern>(b.pattern);
    });
    if (catch_all_branch != stmt->branches.end()) {
        auto &catch_all_pattern = std::get<CatchAllPattern>(catch_all_branch->pattern);
        this->output << "else {" << std::endl;
        this->output << "auto " << catch_all_pattern.bound_variable->name.lexeme << " = " << target_var << ".value();" << std::endl;
        for (auto &stmt : catch_all_branch->body) {
            stmt->accept(*this);
        }

        this->output << "}" << std::endl;
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
    stmt->increment->accept(*this);

    this->output << ") {" << std::endl;

    for (auto &line : stmt->stmts) {
        line->accept(*this);
    }

    this->output << "}" << std::endl;
}

void CppGenerator::visit_print_stmt(PrintStmt *stmt) {
    this->output << "std::cout";

    if (stmt->expr.has_value()) {
        this->output << " << ";

        if (stmt->expr.value()->get_type_info().optional) {
            this->output << "({ auto temp = ";
            stmt->expr.value()->accept(*this);
            this->output << "; temp.has_value() ? " << BAZ_NAMESPACE << "::to_string(temp.value()) : \"null\"; })";
        } else {
            this->output << BAZ_NAMESPACE << "::to_string(";
            stmt->expr.value()->accept(*this);
            this->output << ")";
        }
    }

    if (stmt->newline)
        this->output << " << std::endl";

    this->output << ";" << std::endl;
}

void CppGenerator::visit_panic_stmt(PanicStmt *stmt) {
    this->output << "std::cerr";

    if (stmt->expr.has_value()) {
        this->output << " << ";

        if (stmt->expr.value()->get_type_info().optional) {
            this->output << "({ auto temp = ";
            stmt->expr.value()->accept(*this);
            this->output << "; temp.has_value() ? " << BAZ_NAMESPACE << "::to_string(temp.value()) : \"null\"; })";
        } else {
            this->output << BAZ_NAMESPACE << "::to_string(";
            stmt->expr.value()->accept(*this);
            this->output << ")";
        }
    }

    this->output << " << std::endl;" << std::endl;
    this->output << "exit(1);" << std::endl;
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

void CppGenerator::visit_set_stmt(SetStmt *stmt) {
    // TODO: handle optional set stmt

    stmt->object->accept(*this);
    this->output << "->" << stmt->name.lexeme << " = ";

    stmt->value->accept(*this);
    this->output << ";" << std::endl;
}
