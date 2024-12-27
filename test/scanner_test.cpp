#include "../src/scanner.h"

#include <gtest/gtest.h>

TEST(ScannerTest, Keyword) {
  std::string source = "if";
  Scanner scan = Scanner(source.c_str());

  std::optional<Token> next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::IF);

  next = scan.scan_token();
  EXPECT_FALSE(next.has_value());
}

TEST(ScannerTest, Identifier) {
  std::string source = "some_identifier";
  Scanner scan = Scanner(source.c_str());

  std::optional<Token> next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::IDENTIFIER);
}

TEST(ScannerTest, MultipleLines) {
  std::string source = "a\nb c\nd";
  Scanner scan = Scanner(source.c_str());

  std::optional<Token> next = scan.scan_token();
  EXPECT_EQ(next->line, 1);

  next = scan.scan_token();
  EXPECT_EQ(next->line, 2);

  next = scan.scan_token();
  EXPECT_EQ(next->line, 2);

  next = scan.scan_token();
  EXPECT_EQ(next->line, 3);
}

TEST(ScannerTest, EqualTokens) {
  std::string source = "<= >= == != < > = !";
  Scanner scan = Scanner(source.c_str());

  std::optional<Token> next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::LESS_EQUAL);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::GREATER_EQUAL);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::EQUAL_EQUAL);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::BANG_EQUAL);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::LESS);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::GREATER);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::EQUAL);

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::BANG);
}

TEST(ScannerTest, SkipWhitespace) {
  std::string source = "token1      token2\t\n token3    token4 \n token5";
  Scanner scan = Scanner(source.c_str());

  int token_count = 0;
  std::optional<Token> next = scan.scan_token();
  while (next.has_value()) {
    token_count++;
    next = scan.scan_token();
  }

  EXPECT_EQ(token_count, 5);
}

TEST(ScannerTest, Numbers) {
  std::string source = "1 2.345 0.6 789";
  Scanner scan = Scanner(source.c_str());

  int token_count = 0;
  std::optional<Token> next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::INT_VAL);
  EXPECT_EQ(next->get_raw_token(), "1");

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::FLOAT_VAL);
  EXPECT_EQ(next->get_raw_token(), "2.345");

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::FLOAT_VAL);
  EXPECT_EQ(next->get_raw_token(), "0.6");

  next = scan.scan_token();
  EXPECT_EQ(next->t, TokenType::INT_VAL);
  EXPECT_EQ(next->get_raw_token(), "789");
}

TEST(ScannerTest, RawTokenContents) {
  // Test that char* and length of string is correct for each token
  std::string source = "if\t identifier  1234 >   <=";
  Scanner scan = Scanner(source.c_str());

  int token_count = 0;
  std::optional<Token> next = scan.scan_token();
  EXPECT_EQ(next->get_raw_token(), "if");
  EXPECT_EQ(next->start, source.c_str());
  EXPECT_EQ(next->length, strlen("if"));

  next = scan.scan_token();
  EXPECT_EQ(next->get_raw_token(), "identifier");
  EXPECT_EQ(next->start, source.c_str() + source.find("identifier"));
  EXPECT_EQ(next->length, strlen("identifier"));

  next = scan.scan_token();
  EXPECT_EQ(next->get_raw_token(), "1234");
  EXPECT_EQ(next->start, source.c_str() + source.find("1234"));
  EXPECT_EQ(next->length, strlen("1234"));

  next = scan.scan_token();
  EXPECT_EQ(next->get_raw_token(), ">");
  EXPECT_EQ(next->start, source.c_str() + source.find(">"));
  EXPECT_EQ(next->length, strlen(">"));

  next = scan.scan_token();
  EXPECT_EQ(next->get_raw_token(), "<=");
  EXPECT_EQ(next->start, source.c_str() + source.find("<="));
  EXPECT_EQ(next->length, strlen("<="));
}
