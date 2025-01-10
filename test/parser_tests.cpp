#include "../src/parser.h"
#include "./scanner_mock.h"

#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <memory>

#define CHECK_AND_CAST(value, type)                     \
    ({                                                  \
        auto __cast_result = dynamic_cast<type>(value); \
        EXPECT_NE(__cast_result, nullptr);              \
        __cast_result;                                  \
    })

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
    std::unique_ptr<MockScanner> scan = std::make_unique<MockScanner>();

    TokenGenerator gen({INT_VAL, PLUS, FLOAT_VAL, SEMI_COLON});
    EXPECT_CALL(*scan, scan_token).WillRepeatedly(testing::Invoke(&gen, &TokenGenerator::generate_token));

    Parser p = Parser(std::move(scan));
    auto s = p.parse_stmt();

    auto top_level = CHECK_AND_CAST(s->get(), ExprStmt *);
    auto expr = CHECK_AND_CAST(top_level->expr.get(), BinaryExpr *);
    EXPECT_EQ(expr->op.t, TokenType::PLUS);

    auto left = CHECK_AND_CAST(expr->left.get(), LiteralExpr *);
    EXPECT_EQ(left->literal.t, TokenType::INT_VAL);

    auto right = CHECK_AND_CAST(expr->right.get(), LiteralExpr *);
    EXPECT_EQ(right->literal.t, TokenType::FLOAT_VAL);
}
