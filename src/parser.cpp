#include "parser.h"
#include "expr.h"
#include "stmt.h"
#include "token.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

Parser::Parser(Scanner scanner) : scanner(scanner) { this->advance(); }

std::unique_ptr<Stmt> Parser::parse_stmt() { return this->declaration(); }

std::unique_ptr<Stmt> Parser::declaration() {
    if (this->match(TokenType::STRUCT))
        return this->struct_decl();

    if (this->match(TokenType::FN))
        return this->function_decl();

    // TODO: only allow statements inside functions or structs or enums
    return this->statement();
}

std::unique_ptr<Stmt> Parser::statement() {
    std::unique_ptr<Expr> expr = this->expression();

    // Assignment
    if (this->match(TokenType::EQUAL)) {
        std::unique_ptr<Expr> value = this->expression();
        this->consume(TokenType::SEMI_COLON, "Expected ';' after assignment.");

        // TODO: is there a better way to do this
        if (VarExpr *var = dynamic_cast<VarExpr *>((Expr *)&expr)) {
            Token name = var->name;
            return std::make_unique<ExprStmt>(
                std::make_unique<AssignExpr>(name, std::move(value)));
        }

        std::cerr << "Invalid assignment target." << std::endl;
        exit(2);
    }

    // Expression statement
    this->consume(TokenType::SEMI_COLON, "Expected ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmt> > Parser::block() {
    std::vector<std::unique_ptr<Stmt> > stmts;
    while (!this->check(TokenType::R_CURLY_BRACKET)) {
        stmts.push_back(this->statement());
    }

    this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after block.");

    return stmts;
}

Token Parser::typed_identifier() {
    Token id = this->consume(TokenType::IDENTIFIER, "Expected identifier.");

    consume(TokenType::COLON, "Expected type for identifier.");
    Token type = this->consume(TokenType::TYPE, "Expected type after ':'.");

    // TODO: do something with type
    return id;
}

std::unique_ptr<StructDeclStmt> Parser::struct_decl() {
    Token name = this->consume(TokenType::IDENTIFIER, "Expected struct name.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before function body.");

    std::vector<Token> properties;
    std::vector<std::unique_ptr<FunDeclStmt> > methods;

    while (!this->check(TokenType::R_CURLY_BRACKET)) {
        if (this->match(TokenType::FN)) {
            methods.push_back(this->function_decl());
        } else {
            properties.push_back(this->typed_identifier());
            this->consume(TokenType::SEMI_COLON, "Expected ';' after property.");
        }
    };

    this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after struct body.");

    return std::make_unique<StructDeclStmt>(name, properties, std::move(methods));
}

std::unique_ptr<FunDeclStmt> Parser::function_decl() {
    // TODO: method or function
    Token name = this->consume(TokenType::IDENTIFIER, "Expected function name.");
    std::vector<Token> params;

    this->consume(TokenType::L_BRACKET, "Expected '(' after function name.");
    if (!this->check(TokenType::R_BRACKET)) {
        // Loop while there are commas
        // If there is a comma followed by a right bracket, it is a trailing comma
        do {
            params.push_back(this->typed_identifier());
        } while (this->match(TokenType::COMMA) && !this->check(TokenType::R_BRACKET));
    }

    this->consume(TokenType::R_BRACKET, "Expected ')' after parameter list.");

    this->consume(TokenType::COLON, "Expected return type.");
    Token type = this->consume(TokenType::TYPE, "Expected return type after ':'.");

    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before function body.");

    std::vector<std::unique_ptr<Stmt> > body = this->block();

    return std::make_unique<FunDeclStmt>(name, params, std::move(body));
}

std::unique_ptr<Expr> Parser::expression() { return this->logical_or(); }

std::unique_ptr<Expr> Parser::logical_or() {
    std::unique_ptr<Expr> expr = this->logical_and();

    while (this->match(TokenType::OR)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->logical_and();
        expr = std::make_unique<LogicalBinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logical_and() {
    std::unique_ptr<Expr> expr = this->equality();

    while (this->match(TokenType::AND)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->equality();
        expr = std::make_unique<LogicalBinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = this->comparison();

    if (this->match(TokenType::EQUAL_EQUAL) ||
        this->match(TokenType::BANG_EQUAL)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = this->term();

    if (this->match(TokenType::LESS) || this->match(TokenType::LESS_EQUAL) ||
        this->match(TokenType::GREATER) ||
        this->match(TokenType::GREATER_EQUAL)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = this->factor();

    while (this->match(TokenType::PLUS) || this->match(TokenType::MINUS)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = this->fallback();

    while (this->match(TokenType::STAR) || this->match(TokenType::SLASH)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->fallback();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::fallback() {
    std::unique_ptr<Expr> expr = this->unary();

    if (this->match(TokenType::QUESTION_QUESTION)) {
        Token op = this->previous();
        std::unique_ptr<Expr> rhs = this->unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(rhs));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (this->match(TokenType::BANG) || this->match(TokenType::MINUS)) {
        return std::make_unique<UnaryExpr>(this->previous(), this->unary());
    }

    return this->call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = this->primary();

    while (true) {
        if (this->match(TokenType::L_BRACKET)) {
            expr = this->finish_call(std::move(expr));
        } else if (this->match(TokenType::DOT)) {
            Token name = this->consume(TokenType::IDENTIFIER,
                                       "Expect property name after '.'.");
            expr = std::make_unique<GetExpr>(std::move(expr), name);
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr> > args;

    if (!this->check(TokenType::R_BRACKET)) {
        // Loop while there are commas
        // If there is a comma followed by a right bracket, it is a trailing comma
        do {
            args.push_back(this->expression());
        } while (this->match(TokenType::COMMA) && !this->check(TokenType::R_BRACKET));
    }

    this->consume(TokenType::R_BRACKET, "Expected ')' after arguments.");

    return std::make_unique<CallExpr>(std::move(callee), std::move(args));
}

std::unique_ptr<Expr> Parser::primary() {
    // TODO: strings
    if (this->match(TokenType::TRUE) || this->match(TokenType::FALSE) ||
        this->match(TokenType::NULL_VAL) || this->match(TokenType::INT_VAL) ||
        this->match(TokenType::FLOAT_VAL) || this->match(TokenType::IDENTIFIER)) {
        return std::make_unique<PrimaryExpr>(this->previous());
    }

    if (this->match(TokenType::L_BRACKET)) {
        std::unique_ptr<Expr> expr = this->expression();
        this->consume(TokenType::R_BRACKET, "Expected closing ')'.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    std::cerr << "Expected expression." << std::endl;
    exit(2);
}

std::optional<Token> Parser::advance() {
    this->prev = this->current;
    this->current = scanner.scan_token();

    return this->prev;
}

std::optional<Token> Parser::peek() { return this->current; }
Token Parser::previous() {
    if (!this->prev.has_value()) {
        std::cerr << "[BUG] Expected previous token to exist." << std::endl;
        exit(3);
    }

    return this->prev.value();
}

bool Parser::match(TokenType t) {
    if (this->check(t)) {
        this->advance();
        return true;
    }

    return false;
}

bool Parser::check(TokenType t) {
    std::optional<Token> curr = this->peek();
    if (!curr.has_value())
        return false;

    return curr->t == t;
}

Token Parser::consume(TokenType t, std::string error_message) {
    if (this->check(t)) {
        auto curr = this->advance();
        if (curr.has_value())
            return curr.value();
    }

    std::cerr << error_message << std::endl;
    exit(2);
}
