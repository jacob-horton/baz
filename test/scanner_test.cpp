#include "../src/scanner/scanner.h"

#include <gtest/gtest.h>

TEST(ScannerTest, Keyword) {
    std::string source = "if";
    StringScanner scan = StringScanner(source);

    std::optional<Token> next = scan.scan_token();
    EXPECT_EQ(next->t, TokenType::IF);

    next = scan.scan_token();
    EXPECT_EQ(next->t, TokenType::EOF_);
}

TEST(ScannerTest, Identifier) {
    std::string source = "some_identifier1";
    StringScanner scan = StringScanner(source);

    std::optional<Token> next = scan.scan_token();
    EXPECT_EQ(next->t, TokenType::IDENTIFIER);
    EXPECT_EQ(next->lexeme, "some_identifier1");
}

TEST(ScannerTest, MultipleLines) {
    std::string source = "a\nb c\n d\n \"multi\nline\nstring\" e";
    StringScanner scan = StringScanner(source);

    auto expected = {1, 2, 2, 3, 4, 6};

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->line, ex);
    }
}

TEST(ScannerTest, EqualTokens) {
    std::string source = "<= >= == != < > = !";
    StringScanner scan = StringScanner(source);

    auto expected = {
        TokenType::LESS_EQUAL,
        TokenType::GREATER_EQUAL,
        TokenType::EQUAL_EQUAL,
        TokenType::BANG_EQUAL,
        TokenType::LESS,
        TokenType::GREATER,
        TokenType::EQUAL,
        TokenType::BANG,
    };

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->t, ex);
    }
}

TEST(ScannerTest, SkipWhitespace) {
    std::string source = "token1      token2\t\n token3    token4 \n token5";
    StringScanner scan = StringScanner(source);

    int token_count = 0;
    std::optional<Token> next = scan.scan_token();
    while (next.has_value() && next->t != TokenType::EOF_) {
        token_count++;
        next = scan.scan_token();
    }

    EXPECT_EQ(token_count, 5);
}

TEST(ScannerTest, Numbers) {
    std::string source = "1 2.345 0.6 789";
    StringScanner scan = StringScanner(source);

    auto expected = {
        std::make_tuple(TokenType::INT_VAL, "1"),
        std::make_tuple(TokenType::FLOAT_VAL, "2.345"),
        std::make_tuple(TokenType::FLOAT_VAL, "0.6"),
        std::make_tuple(TokenType::INT_VAL, "789"),
    };

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->t, std::get<0>(ex));
        EXPECT_EQ(next->lexeme, std::get<1>(ex));
    }
}

TEST(ScannerTest, RawTokenContents) {
    std::string source = "if\t identifier  1234 >   <=";
    StringScanner scan = StringScanner(source);

    auto expected = {"if", "identifier", "1234", ">", "<="};

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->lexeme, ex);
    }
}

TEST(ScannerTest, NoWhitespace) {
    std::string source = "token.other_token<5.1>3";
    StringScanner scan = StringScanner(source);

    auto expected = {
        TokenType::IDENTIFIER,
        TokenType::DOT,
        TokenType::IDENTIFIER,
        TokenType::LESS,
        TokenType::FLOAT_VAL,
    };

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->t, ex);
    }
}

TEST(ScannerTest, InvalidNumber) {
    std::string source = "1token";
    StringScanner scan = StringScanner(source);

    EXPECT_DEATH({ scan.scan_token(); }, "Unexpected character in number: '1t'");
}

TEST(ScannerTest, InvalidSymbol) {
    std::string source = "^";
    StringScanner scan = StringScanner(source);

    EXPECT_DEATH({ scan.scan_token(); }, "Unrecognised symbol: '\\^'");
}

TEST(ScannerTest, DoubleTokens) {
    std::string source = "&& || ?? ? & |";
    StringScanner scan = StringScanner(source);

    auto expected = {
        TokenType::AND,
        TokenType::OR,
        TokenType::QUESTION_QUESTION,
        TokenType::QUESTION,
    };

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->t, ex);
    }

    EXPECT_DEATH({ scan.scan_token(); }, "Unrecognised symbol: '&'");
    EXPECT_DEATH({ scan.scan_token(); }, "Unrecognised symbol: '|'");
}

TEST(ScannerTest, Types) {
    std::string source = "int str float void bool string boolean";
    StringScanner scan = StringScanner(source);

    auto expected = {
        std::make_tuple(TokenType::TYPE, "int"),
        std::make_tuple(TokenType::TYPE, "str"),
        std::make_tuple(TokenType::TYPE, "float"),
        std::make_tuple(TokenType::TYPE, "void"),
        std::make_tuple(TokenType::TYPE, "bool"),
        std::make_tuple(TokenType::IDENTIFIER, "string"),
        std::make_tuple(TokenType::IDENTIFIER, "boolean"),
    };

    for (auto ex : expected) {
        std::optional<Token> next = scan.scan_token();
        EXPECT_EQ(next->t, std::get<0>(ex));
        EXPECT_EQ(next->lexeme, std::get<1>(ex));
    }
}
