#include "scanner.h"
#include "token.h"

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

Token Scanner::make_token(TokenType t) {
    return Token{
        t,
        this->token_start,
        this->current - this->token_start,
        line,
    };
}

Token Scanner::number() {
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

    return this->make_token(type);
}

// Returns TokenType of keyword if the current token is one
std::optional<TokenType> Scanner::get_keyword_type() {
    char word[this->current - this->token_start + 1];
    strncpy(word, this->token_start, this->current - this->token_start);
    word[this->current - this->token_start] = 0;

    auto token_type = KEYWORDS.find(word);
    if (token_type == KEYWORDS.end()) {
        return {};
    }

    return token_type->second;
}

// If token is a keyword, return the keyword type,
// otherwise it is an identifier
Token Scanner::identifier_or_keyword() {
    // Consume alphanumeric - we've already consumed alpha
    while (is_alphanum(this->peek()))
        this->advance();

    std::optional<TokenType> keyword_type = this->get_keyword_type();
    if (keyword_type.has_value())
        return this->make_token(keyword_type.value());

    return this->make_token(TokenType::IDENTIFIER);
}

Token Scanner::symbol(char start) {
    // TODO: strings in quotes
    TokenType type;

    auto equal_token_type = EQUAL_SYMBOLS.find(start);
    if (equal_token_type != EQUAL_SYMBOLS.end()) {
        if (this->match('='))
            return this->make_token(equal_token_type->second.with_equal);
        else
            return this->make_token(equal_token_type->second.single);
    }

    // TODO: handle double symbols better
    if (start == '|' && this->match('|'))
        return this->make_token(TokenType::OR);

    if (start == '&' && this->match('&'))
        return this->make_token(TokenType::AND);

    if (start == '?' && this->match('?'))
        return this->make_token(TokenType::QUESTION_QUESTION);

    auto token_type = SYMBOLS.find(start);
    if (token_type == SYMBOLS.end()) {
        // TODO: handle error properly
        std::cerr << "Unrecognised symbol: '" << start << "'" << std::endl;
        exit(1);
    }

    return this->make_token(token_type->second);
}

void Scanner::skip_whitespace() {
    // TODO: comments
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
            default:
                return;
        }
    }
}

bool Scanner::match(char c) {
    if (*this->current == c) {
        this->current++;
        return true;
    }

    return false;
}

char Scanner::advance() {
    char current = *this->current;
    this->current++;

    return current;
}

char Scanner::peek() {
    return *this->current;
}

Scanner::Scanner(const char *source) {
    this->current = source;
    this->end = source + strlen(source);
    this->line = 1;
}

std::optional<Token> Scanner::scan_token() {
    if (this->current >= this->end) {
        return {};
    }

    this->skip_whitespace();
    this->token_start = this->current;

    char c = this->advance();
    if (is_digit(c)) {
        Token number = this->number();

        // Check that next token isn't alpha (i.e. we don't want "1234a")
        if (is_alpha(this->peek())) {
            std::cerr << "Unexpected character in number: '"
                      << number.get_raw_token() << this->peek() << "'" << std::endl;
            exit(1);
        }

        return number;
    }

    if (is_alpha(c))
        return this->identifier_or_keyword();

    return this->symbol(c);
}
