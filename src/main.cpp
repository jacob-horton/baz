#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "code_generator/cpp_generator.h"
#include "parser/parser.h"
#include "scanner/scanner.h"
#include "type_checker/resolver.h"
#include "type_checker/type_checker.h"

std::string read_file(std::string path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

int main(int argc, char *argv[]) {
    auto source = read_file("../examples/type_checking.baz");
    if (argc == 2) {
        auto arg = argv[1];
        if (strcmp(arg, "--help") == 0) {
            std::cout << "Usage:" << std::endl
                      << "- './baz'                compile a default example piece of code" << std::endl
                      << "- './baz <source_code>'  compile the specified source code file" << std::endl;
            exit(0);
        }

        source = read_file(argv[1]);
    }

    auto scan = std::make_unique<StringScanner>(source);
    Parser parser = Parser(std::move(scan));

    std::vector<std::unique_ptr<Stmt>> stmts;
    auto stmt = parser.parse_stmt();
    while (stmt.has_value()) {
        stmts.push_back(std::move(stmt.value()));
        stmt = parser.parse_stmt();
    }

    // Resolve types
    auto resolver = Resolver();
    resolver.resolve(stmts);

    // Check types
    auto type_checker = TypeChecker();
    try {
        type_checker.check(stmts);
    } catch (TypeCheckerError) {
        std::cout << "Failed type check" << std::endl;
        exit(4);
    }

    // Generate C++
    std::ofstream file("output.cpp");
    auto cpp_generator = CppGenerator(file);
    cpp_generator.generate(stmts);

    std::cout << "Successfully outputted to './output.cpp'" << std::endl;

    return 0;
}
