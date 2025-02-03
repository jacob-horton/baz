#pragma once

#include "expr_visitor.h"
#include "stmt_visitor.h"
#include <fstream>
#include <string>

enum TypeCheckerError {
    OP_ON_INCOMPATIBLE_TYPES,
};

enum TypeClass {
    INT,
    FLOAT,
    BOOL,
    NULL_,
    STR,
};

struct Type {
    TypeClass type_class;

    Type(TypeClass tc) : type_class(tc) {}

    virtual bool can_coerce_to(Type type) {
        return false;
    };

    virtual ~Type() = default;
};

struct IntType : public Type {
    IntType() : Type(TypeClass::INT) {}
};

struct FloatType : public Type {
    FloatType() : Type(TypeClass::FLOAT) {}
};

struct BoolType : public Type {
    BoolType() : Type(TypeClass::BOOL) {}
};

struct NullType : public Type {
    NullType() : Type(TypeClass::NULL_) {}
};

struct StrType : public Type {
    StrType() : Type(TypeClass::STR) {}
};

class TypeChecker : public ExprVisitor,
                    public StmtVisitor {
  public:
    // TODO: should this be private?
    std::unique_ptr<Type> result;
    TypeChecker();

    void error(Token t, std::string message);

    void visitVarExpr(VarExpr *expr);
    void visitStructInitExpr(StructInitExpr *expr);
    void visitBinaryExpr(BinaryExpr *expr);
    void visitLogicalBinaryExpr(LogicalBinaryExpr *expr);
    void visitUnaryExpr(UnaryExpr *expr);
    void visitGetExpr(GetExpr *expr);
    void visitCallExpr(CallExpr *expr);
    void visitGroupingExpr(GroupingExpr *expr);
    void visitLiteralExpr(LiteralExpr *expr);

    void visitFunDeclStmt(FunDeclStmt *stmt);
    void visitStructDeclStmt(StructDeclStmt *stmt);
    void visitEnumDeclStmt(EnumDeclStmt *stmt);
    void visitVariableDeclStmt(VariableDeclStmt *stmt);
    void visitExprStmt(ExprStmt *stmt);
    void visitBlockStmt(BlockStmt *stmt);
    void visitIfStmt(IfStmt *stmt);
    void visitMatchStmt(MatchStmt *stmt);
    void visitWhileStmt(WhileStmt *stmt);
    void visitForStmt(ForStmt *stmt);
    void visitPrintStmt(PrintStmt *stmt);
    void visitReturnStmt(ReturnStmt *stmt);
    void visitAssignStmt(AssignStmt *stmt);
};
