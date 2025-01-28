#include "parser.h"
#include "enum_variant.h"
#include "expr.h"
#include "stmt.h"
#include "token.h"
#include "typed_var.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <vector>

Parser::Parser(std::unique_ptr<Scanner> scanner) : scanner(std::move(scanner)) {
    this->advance();
}

std::optional<std::unique_ptr<Stmt>> Parser::parse_stmt() {
    return this->top_level_decl();
}

std::optional<std::unique_ptr<Stmt>> Parser::top_level_decl() {
    if (this->peek().t == TokenType::EOF_)
        return {};

    if (this->match(TokenType::STRUCT))
        return this->struct_decl();

    if (this->match(TokenType::ENUM))
        return this->enum_decl();

    if (this->match(TokenType::FN))
        return this->function_decl();

    this->error(this->peek(), "Unexpected statement at top level.");
    exit(2);
}

std::unique_ptr<Stmt> Parser::nested_decl() {
    if (this->match(TokenType::LET))
        return this->variable_decl();

    return this->statement();
}

std::unique_ptr<Stmt> Parser::statement() {
    if (this->match(TokenType::IF))
        return this->if_statement();
    if (this->match(TokenType::MATCH))
        return this->match_statement();
    if (this->match(TokenType::FOR))
        return this->for_statement();
    if (this->match(TokenType::WHILE))
        return this->while_statement();
    if (this->match(TokenType::PRINT))
        return this->print_statement();
    if (this->match(TokenType::RETURN))
        return this->return_statement();
    if (this->match(TokenType::L_CURLY_BRACKET))
        return std::make_unique<BlockStmt>(std::move(this->block()));

    std::unique_ptr<Expr> expr = this->expression();
    if (this->match(TokenType::EQUAL)) {
        std::unique_ptr<Stmt> assign_stmt = this->assignment(*expr);
        this->consume(TokenType::SEMI_COLON, "Expected ';' after assignment.");
        return assign_stmt;
    }

    // Expression statement
    this->consume(TokenType::SEMI_COLON, "Expected ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::if_statement() {
    this->consume(TokenType::L_BRACKET, "Expected '(' after 'if'.");
    std::unique_ptr<Expr> condition = this->expression();
    this->consume(TokenType::R_BRACKET, "Expected closing ')' after if condition.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before if branch.");

    auto true_block = this->block();

    // TODO: else if
    std::optional<std::vector<std::unique_ptr<Stmt>>> false_block = {};
    if (this->match(TokenType::ELSE)) {
        this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before else branch.");
        false_block = this->block();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(true_block), std::move(false_block));
}

std::unique_ptr<Stmt> Parser::match_statement() {
    this->consume(TokenType::L_BRACKET, "Expected '(' after 'match'.");
    std::unique_ptr<Expr> target = this->expression();
    this->consume(TokenType::R_BRACKET, "Expected closing ')' after match target.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before match body.");

    std::vector<MatchBranch> branches;
    do {
        auto pattern = this->call();
        this->consume(TokenType::COLON, "Expected ':' after match pattern.");

        this->consume(TokenType::L_CURLY_BRACKET, "Expected block after match pattern.");
        auto body = this->block();

        branches.push_back(MatchBranch(std::move(pattern), std::move(body)));
    } while (this->match(TokenType::COMMA) && !this->check(TokenType::R_CURLY_BRACKET));

    this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after match branches");
    return std::make_unique<MatchStmt>(std::move(target), std::move(branches));
}

std::unique_ptr<Stmt> Parser::for_statement() {
    this->consume(TokenType::L_BRACKET, "Expected '(' after 'for'.");
    this->consume(TokenType::LET, "Expected variable declaration.");
    std::unique_ptr<Stmt> var = this->variable_decl();

    std::unique_ptr<Expr> condition = this->expression();
    this->consume(TokenType::SEMI_COLON, "Expected ';' after expression.");
    auto condition_stmt = std::make_unique<ExprStmt>(std::move(condition));

    std::unique_ptr<Expr> expr = this->expression();
    this->consume(TokenType::EQUAL, "Expected '=' after assignment target.");
    std::unique_ptr<Stmt> increment = this->assignment(*expr);

    this->consume(TokenType::R_BRACKET, "Expected closing ')' after for condition.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before loop body.");
    auto body = this->block();
    return std::make_unique<ForStmt>(std::move(var), std::move(condition_stmt), std::move(increment), std::move(body));
}

std::unique_ptr<Stmt> Parser::while_statement() {
    this->consume(TokenType::L_BRACKET, "Expected '(' after 'while'.");
    std::unique_ptr<Expr> condition = this->expression();
    this->consume(TokenType::R_BRACKET, "Expected closing ')' after while condition.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before loop body.");

    auto body = this->block();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::print_statement() {
    Token print = this->previous();

    this->consume(TokenType::L_BRACKET, "Expected '(' after 'print'.");
    if (this->match(TokenType::R_BRACKET)) {
        this->consume(TokenType::SEMI_COLON, "Expected ';' after print statement.");
        return std::make_unique<PrintStmt>(
            std::optional<std::unique_ptr<Expr>>{},
            print.lexeme == "println");
    }

    std::unique_ptr<Expr> value = this->expression();
    this->consume(TokenType::R_BRACKET, "Expected closing ')' after print value.");
    this->consume(TokenType::SEMI_COLON, "Expected ';' after print statement.");

    return std::make_unique<PrintStmt>(std::move(value), print.lexeme == "println");
}

std::unique_ptr<Stmt> Parser::return_statement() {
    if (this->match(TokenType::SEMI_COLON)) {
        return std::make_unique<ReturnStmt>(std::optional<std::unique_ptr<Expr>>{});
    }

    std::unique_ptr<Expr> value = this->expression();
    this->consume(TokenType::SEMI_COLON, "Expected ';' after return statement.");

    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::assignment(Expr &lhs) {
    Token equals_token = this->previous();
    std::unique_ptr<Expr> value = this->expression();

    // TODO: is there a better way to do this
    if (VarExpr *var = dynamic_cast<VarExpr *>(&lhs)) {
        Token name = var->name;
        return std::make_unique<AssignStmt>(name, std::move(value));
    }

    this->error(equals_token, "Invalid assignment target.");
    exit(2);
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!this->check(TokenType::R_CURLY_BRACKET)) {
        stmts.push_back(this->nested_decl());
    }

    this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after block.");

    return stmts;
}

TypedVar Parser::typed_identifier() {
    Token name = this->consume(TokenType::IDENTIFIER, "Expected identifier.");

    this->consume(TokenType::COLON, "Expected type for identifier.");

    Token type = this->type();
    bool is_optional = this->match(TokenType::QUESTION);

    return TypedVar(name, type, is_optional);
}

Token Parser::type() {
    if (this->match(TokenType::TYPE))
        return this->previous();

    return this->consume(TokenType::IDENTIFIER, "Expected type.");
}

std::unique_ptr<StructDeclStmt> Parser::struct_decl() {
    Token name = this->consume(TokenType::IDENTIFIER, "Expected struct name.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before function body.");

    std::vector<TypedVar> properties;
    std::vector<std::unique_ptr<FunDeclStmt>> methods;

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

std::unique_ptr<EnumDeclStmt> Parser::enum_decl() {
    Token name = this->consume(TokenType::IDENTIFIER, "Expected enum name.");
    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before function body.");

    std::vector<EnumVariant> variants;
    std::vector<std::unique_ptr<FunDeclStmt>> methods;

    while (!this->check(TokenType::R_CURLY_BRACKET)) {
        if (this->match(TokenType::FN)) {
            methods.push_back(this->function_decl());
        } else {
            variants.push_back(this->enum_variant());
            this->consume(TokenType::SEMI_COLON, "Expected ';' after property.");
        }
    };

    this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after struct body.");

    return std::make_unique<EnumDeclStmt>(name, variants, std::move(methods));
}

EnumVariant Parser::enum_variant() {
    Token name = this->consume(TokenType::IDENTIFIER, "Expected name for enum variant.");

    std::optional<Token> type = {};
    bool is_optional = false;
    if (this->match(TokenType::L_BRACKET)) {
        type = this->type();
        is_optional = this->match(TokenType::QUESTION);

        this->consume(TokenType::R_BRACKET, "Expected ')' after variant payload.");
    }

    return EnumVariant(name, type, is_optional);
}

std::unique_ptr<FunDeclStmt> Parser::function_decl() {
    // TODO: method or function
    Token name = this->consume(TokenType::IDENTIFIER, "Expected function name.");
    std::vector<TypedVar> params;

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
    Token return_type = this->type();

    this->consume(TokenType::L_CURLY_BRACKET, "Expected '{' before function body.");
    std::vector<std::unique_ptr<Stmt>> body = this->block();

    return std::make_unique<FunDeclStmt>(name, params, return_type, std::move(body));
}

std::unique_ptr<VariableDeclStmt> Parser::variable_decl() {
    TypedVar name = this->typed_identifier();
    this->consume(TokenType::EQUAL, "Expected '=' after variable declaration.");
    std::unique_ptr<Expr> expr = this->expression();

    this->consume(TokenType::SEMI_COLON, "Expected ';' after variable declaration.");

    return std::make_unique<VariableDeclStmt>(name, std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return this->logical_or();
}

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
        Token prev = this->previous();
        return std::make_unique<UnaryExpr>(prev, this->unary());
    }

    return this->call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = this->primary();

    while (true) {
        if (this->match(TokenType::L_BRACKET)) {
            expr = this->finish_call(std::move(expr));
        } else if (this->match(TokenType::DOT)) {
            Token name = this->consume(TokenType::IDENTIFIER, "Expected property name after '.'.");
            expr = std::make_unique<GetExpr>(std::move(expr), name);
            // TODO: do we need to handle this better - if we have `call()?;` what will happen
            // TODO: remove duplication
        } else if (this->match(TokenType::QUESTION) && this->match(TokenType::DOT)) {
            Token name = this->consume(TokenType::IDENTIFIER, "Expected property name after '.'.");
            expr = std::make_unique<GetExpr>(std::move(expr), name);
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> args;

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
    if (this->match(TokenType::IDENTIFIER) || this->match(TokenType::THIS)) {
        if (this->match(TokenType::L_CURLY_BRACKET))
            return this->finish_struct_init(this->previous());

        return std::make_unique<VarExpr>(this->previous());
    }

    if (this->match(TokenType::TRUE) || this->match(TokenType::FALSE) ||
        this->match(TokenType::NULL_VAL) || this->match(TokenType::INT_VAL) ||
        this->match(TokenType::FLOAT_VAL) || this->match(TokenType::STR_VAL)) {
        return std::make_unique<LiteralExpr>(this->previous());
    }

    if (this->match(TokenType::L_BRACKET)) {
        std::unique_ptr<Expr> expr = this->expression();
        this->consume(TokenType::R_BRACKET, "Expected closing ')'.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    this->error(this->peek(), "Expected expression.");
    exit(2);
}

std::unique_ptr<Expr> Parser::finish_struct_init(Token name) {
    std::vector<std::tuple<Token, std::unique_ptr<Expr>>> properties;

    // Loop while there are commas
    // If there is a comma followed by a right bracket, it is a trailing comma
    do {
        Token prop_name = this->consume(TokenType::IDENTIFIER, "Expected property name.");
        this->consume(TokenType::COLON, "Expected property value.");

        std::unique_ptr<Expr> value = this->expression();
        properties.push_back(std::make_tuple(prop_name, std::move(value)));
    } while (this->match(TokenType::COMMA) && !this->check(TokenType::R_CURLY_BRACKET));

    this->consume(TokenType::R_CURLY_BRACKET, "Expected '}' after struct initialisation.");

    return std::make_unique<StructInitExpr>(name, std::move(properties));
}

Token Parser::advance() {
    this->prev = this->current;
    this->current = scanner->scan_token();

    return this->previous();
}

Token Parser::peek() {
    return this->current;
}

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
    Token curr = this->peek();
    if (curr.t == TokenType::EOF_)
        return false;

    return curr.t == t;
}

Token Parser::consume(TokenType t, std::string error_message) {
    if (this->check(t))
        return this->advance();

    this->error(this->peek(), error_message);
    exit(2);
}

void Parser::error(Token error_token, std::string message) {
    std::string where = "end";
    if (error_token.t != TokenType::EOF_)
        where = "'" + error_token.lexeme + "'";

    std::cerr << "[line " << error_token.line << "] Error at " << where << ": " << message << std::endl;
}
