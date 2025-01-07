#include "../src/parser.h"
#include "./scanner_mock.h"

#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <memory>

struct TokenGenerator {
    std::vector<TokenType> token_types;
    int i;

    TokenGenerator(std::vector<TokenType> token_types)
        : token_types(token_types), i(0) {}

    Token generate_token() {
        return Token{this->token_types[i++], "", -1};
    }
};

TEST(ParserTest, BinaryOperator) {
    // TODO: mock scanner to test independently
    std::unique_ptr<MockScanner> scan = std::make_unique<MockScanner>();

    TokenGenerator gen({INT_VAL, TokenType::PLUS, TokenType::FLOAT_VAL, TokenType::SEMI_COLON});
    EXPECT_CALL(*scan, scan_token).WillRepeatedly(testing::Invoke(&gen, &TokenGenerator::generate_token));

    Parser p = Parser(std::move(scan));
    std::unique_ptr<Stmt> s = p.parse_stmt();

    // TODO: assert result
    //
    // EXPECT_NE(dynamic_cast<ExprStmt *>(s.get()), nullptr);
    //
    // std::unique_ptr<Expr> expr = std::move(((ExprStmt *)s.get())->expr);
    // EXPECT_NE(dynamic_cast<BinaryExpr *>(expr.get()), nullptr);
    // BinaryExpr *bin_expr = (BinaryExpr *)expr.get();
    // EXPECT_EQ(bin_expr->left, TokenType::INT_VAL);
}
