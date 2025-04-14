#include "../src/ast/stmt.h"
#include "../src/parser/parser.h"
#include "../src/scanner/scanner.h"
#include "../src/type_checker/resolver.h"
#include "../src/type_checker/type_checker.h"
#include "../src/type_checker/type_environment.h"

#include <gtest/gtest.h>

std::vector<std::unique_ptr<Stmt>> run_test(std::string &source) {
    auto scan = std::make_unique<StringScanner>(source);
    Parser parser = Parser(std::move(scan));

    std::vector<std::unique_ptr<Stmt>> stmts;
    auto stmt = parser.parse_stmt();
    while (stmt.has_value()) {
        stmts.push_back(std::move(stmt.value()));
        stmt = parser.parse_stmt();
    }

    // Generate type environment
    auto type_env = TypeEnvironment();
    type_env.generate_type_env(stmts);

    // Resolve types
    auto resolver = Resolver(type_env.type_env);
    resolver.resolve(stmts);

    // Check types
    auto type_checker = TypeChecker(type_env.type_env);
    type_checker.check(stmts);

    return std::move(stmts);
}

TEST(TypeCheckerTest, ReturnTypes) {
    auto expected = {
        std::make_tuple("returns_success.baz", true),
        std::make_tuple("for_missing_return.baz", false),
        std::make_tuple("if_missing_return.baz", false),
        std::make_tuple("match_missing_return.baz", false),
        std::make_tuple("missing_return.baz", false),
        std::make_tuple("nested_missing_return.baz", false),
        std::make_tuple("returns_no_val_when_void.baz", false),
        std::make_tuple("returns_val_when_void.baz", false),
        std::make_tuple("returns_wrong_type.baz", false),
        std::make_tuple("while_missing_return.baz", false),
    };

    for (auto ex : expected) {
        std::ifstream t(std::string("../test/test_cases/") + std::get<0>(ex));
        std::string source((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

        if (std::get<1>(ex)) {
            EXPECT_NO_THROW({ run_test(source); });
        } else {
            EXPECT_ANY_THROW({ run_test(source); });
        }
    }
}

TEST(TypeCheckerTest, GeneralTypeChecking) {
    std::ifstream t(std::string("../test/test_cases/type_checking.baz"));
    std::string source((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());

    EXPECT_NO_THROW({ run_test(source); });
}

TEST(TypeCheckerTest, Optionals) {
    auto expected = {
        std::make_tuple("optional_type_checking_succeed.baz", true),
        std::make_tuple("optional_type_checking_fail.baz", false),
    };

    for (auto ex : expected) {
        std::ifstream t(std::string("../test/test_cases/") + std::get<0>(ex));
        std::string source((std::istreambuf_iterator<char>(t)),
                           std::istreambuf_iterator<char>());

        if (std::get<1>(ex)) {
            EXPECT_NO_THROW({ run_test(source); });
        } else {
            EXPECT_ANY_THROW({ run_test(source); });
        }
    }
}
