#include "scanner.h"

#include <cstring>
#include <iostream>
#include <ostream>

bool is_alpha(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
        return true;

    return false;
}

bool is_digit(char c) {
    if (c >= '0' && c <= '9')
        return true;

    return false;
}

bool is_alphanum(char c) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
        (c >= '0' && c <= '9'))
        return true;

    return false;
}

Token StringScanner::make_token(TokenType t) {
    return this->make_token(t, std::string(this->token_start, this->current - this->token_start));
}

Token StringScanner::make_token(TokenType t, std::string lexeme) {
    return this->make_token(t, lexeme, this->line);
}

Token StringScanner::make_token(TokenType t, std::string lexeme, long line) {
    return Token{t, lexeme, line};
}

Token StringScanner::number() {
    TokenType type = TokenType::INT_VAL;

    // Consume digits - we already know we've got an initial one
    while (is_digit(this->peek()))
        this->advance();

    // If reach a `.`, include it and continue matching digits
    // We know it is a float at this point
    if (this->match('.')) {
        type = TokenType::FLOAT_VAL;
        while (is_digit(this->peek()))
            this->advance();
    }

    Token token = this->make_token(type);
    // Check that next token isn't alpha (i.e. we don't want "1234a")
    if (is_alpha(this->peek())) {
        std::cerr << "Unexpected character in number: '" << token.lexeme << this->peek() << "'" << std::endl;
        exit(1);
    }

    return token;
}

// Returns TokenType of keyword if the current token is one
std::optional<TokenType> StringScanner::get_keyword_type() {
    std::string word(this->token_start, this->current - this->token_start);

    auto token_type = KEYWORDS.find(word);
    if (token_type == KEYWORDS.end())
        return {};

    return token_type->second;
}

// If token is a keyword, return the keyword type,
// otherwise it is an identifier
Token StringScanner::identifier_or_keyword() {
    // Consume alphanumeric - we've already consumed alpha
    while (is_alphanum(this->peek()))
        this->advance();

    std::optional<TokenType> keyword_type = this->get_keyword_type();
    if (keyword_type.has_value())
        return this->make_token(keyword_type.value());

    return this->make_token(TokenType::IDENTIFIER);
}

Token StringScanner::symbol(char start) {
    TokenType type;

    auto equal_token_type = EQUAL_SYMBOLS.find(start);
    if (equal_token_type != EQUAL_SYMBOLS.end()) {
        if (this->match('='))
            return this->make_token(equal_token_type->second.with_equal);
        else
            return this->make_token(equal_token_type->second.single);
    }

    auto double_token_type = DOUBLE_SYMBOLS.find(start);
    if (double_token_type != DOUBLE_SYMBOLS.end() && this->match(start)) {
        return this->make_token(double_token_type->second);
    }

    auto token_type = SYMBOLS.find(start);
    if (token_type == SYMBOLS.end()) {
        std::cerr << "Unrecognised symbol: '" << start << "'" << std::endl;
        exit(1);
    }

    return this->make_token(token_type->second);
}

void StringScanner::skip_whitespace() {
    while (true) {
        switch (this->peek()) {
            case '\n':
                this->line++;
                this->advance();
                break;
            case ' ':
            case '\t':
            case '\r':
                this->advance();
                break;
            case '/':
                // Skip single line comments
                if (this->peek_next() == '/') {
                    while (this->peek() != '\n' && !this->is_at_end())
                        this->advance();
                } else if (this->peek_next() == '*') {
                    while ((this->peek() != '*' || this->peek_next() != '/') && !this->is_at_end()) {
                        this->advance();
                    }

                    if (this->is_at_end()) {
                        std::cerr << "Unterminated multiline comment." << std::endl;
                        exit(1);
                    }

                    this->advance();

                    if (this->is_at_end()) {
                        std::cerr << "Unterminated multiline comment." << std::endl;
                        exit(1);
                    }

                    this->advance();
                } else {
                    return;
                }

                break;
            default:
                return;
        }
    }
}

bool StringScanner::match(char c) {
    if (*this->current == c) {
        this->current++;
        return true;
    }

    return false;
}

char StringScanner::advance() {
    char current = *this->current;
    this->current++;

    return current;
}

char StringScanner::peek() {
    return *this->current;
}

char StringScanner::peek_next() {
    return this->current[1];
}

Token StringScanner::string() {
    long start_line = this->line;

    while (this->peek() != '"' && this->current < this->end) {
        if (this->peek() == '\n')
            this->line++;

        this->advance();
    }

    if (this->current >= this->end) {
        std::cerr << "Unterminated string" << std::endl;
        exit(1);
    }

    this->advance();

    return this->make_token(TokenType::STR_VAL, std::string(this->token_start, this->current - this->token_start), start_line);
}

StringScanner::StringScanner(std::string &source) {
    this->current = source.c_str();
    this->end = source.c_str() + source.length();
    this->line = 1;
}

bool StringScanner::is_at_end() {
    return this->current >= this->end;
}

Token StringScanner::scan_token() {
    this->skip_whitespace();

    if (this->is_at_end())
        return this->make_token(TokenType::EOF_, "");

    this->token_start = this->current;

    char c = this->advance();
    if (is_digit(c))
        return this->number();
    if (is_alpha(c))
        return this->identifier_or_keyword();
    if (c == '"')
        return this->string();

    return this->symbol(c);
}
