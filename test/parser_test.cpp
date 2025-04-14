#include "../src/parser/parser.h"
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

struct FunctionWrappedTokenGenerator {
    std::vector<TokenType> token_types;
    int i;

    const std::vector<TokenType> function_tokens = {FN, IDENTIFIER, L_BRACKET, R_BRACKET, COLON, TYPE, L_CURLY_BRACKET};

    FunctionWrappedTokenGenerator(std::vector<TokenType> token_types)
        : token_types(token_types), i(0) {}

    Token type_to_token(TokenType t) {
        return Token{t, "", -1};
    }

    Token generate_token() {
        if (i < this->function_tokens.size())
            return this->type_to_token(this->function_tokens[i++]);

        if (i < this->function_tokens.size() + this->token_types.size())
            return this->type_to_token(this->token_types[i++ - this->function_tokens.size()]);

        return this->type_to_token(R_CURLY_BRACKET);
    }
};

TEST(ParserTest, BinaryOperator) {
    std::unique_ptr<MockScanner> scan = std::make_unique<MockScanner>();

    FunctionWrappedTokenGenerator gen({INT_VAL, PLUS, FLOAT_VAL, SEMI_COLON});
    EXPECT_CALL(*scan, scan_token).WillRepeatedly(testing::Invoke(&gen, &FunctionWrappedTokenGenerator::generate_token));

    Parser p = Parser(std::move(scan));
    auto s = p.parse_stmt();

    auto fn = CHECK_AND_CAST(s->get(), FunDeclStmt *);
    auto expr_stmt = CHECK_AND_CAST(fn->body[0].get(), ExprStmt *);
    auto expr = CHECK_AND_CAST(expr_stmt->expr.get(), BinaryExpr *);
    EXPECT_EQ(expr->op.t, TokenType::PLUS);

    auto left = CHECK_AND_CAST(expr->left.get(), LiteralExpr *);
    EXPECT_EQ(left->literal.t, TokenType::INT_VAL);

    auto right = CHECK_AND_CAST(expr->right.get(), LiteralExpr *);
    EXPECT_EQ(right->literal.t, TokenType::FLOAT_VAL);
}

// TODO: precedence tests
// TODO: struct tests
// TODO: enum tests
// TODO: function tests
// TODO: only struct, enum, function at top level tests
// TODO: if/else-if/else tests
// TODO: invalid syntax tests
// TODO: match tests
// TODO: print vs println tests
