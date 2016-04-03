#ifndef EXPR_AST_H
#define EXPR_AST_H

#include "llvm/IR/Value.h"
#include <deque>
#include <vector>

using namespace llvm;
using namespace std;

// ExprAst - Base class for all expression nodes.
class ExprAst {
public:
    virtual ~ExprAst() {}
    virtual Value* generateCode() = 0;
};

// ExternExprAst - Expression for an extern (function).
class ExternExprAst : public ExprAst {
    Type* type;
    char* id;
    vector<Type*>* paramTypes;
public:
    ExternExprAst(Type* returnType, char* identifier, vector<Type*>* parameterTypes);
    virtual Value* generateCode();
};

// FieldVarDeclExprAst - Expression for a declaring a field variable for both scalars and arrays.
class FieldVarDeclExprAst : public ExprAst {
    Type* type;
    char* id;
    int size;
public:
    FieldVarDeclExprAst(Type* dataType, char* identifier, int quantity);
    virtual Value* generateCode();
};

// FieldDefExprAst - Expression for a declaring and defining a scalar field variable.
class FieldVarDefExprAst : public ExprAst {
    Type* type;
    char* id;
    Constant* value;
public:
    FieldVarDefExprAst(Type* dataType, char* identifier, ExprAst* initialValue);
    virtual Value* generateCode();
};

// FunctionExprAst - Expresion for a function.
class FunctionExprAst : public ExprAst {
    Type* type;
    char* id;
    vector<pair<Type*,char*>*>* paramList;
    deque<ExprAst*>* stmtList;
    Function* function;
    BasicBlock* block;
public:
    FunctionExprAst(Type* returnType, char* identifier, vector<pair<Type*,char*>*>* parameterList, deque<ExprAst*>* statementList);
    virtual Value* generateCode();
    void generateDeferedCode();
};

// FunctionParamExprAst - Expression for a function parameter.
class FunctionParamExprAst : public ExprAst {
    Type* type;
    char* id;
    Argument* param;
public:
    FunctionParamExprAst(Type* dataType, char* identifier, Argument* parameter);
    virtual Value* generateCode();
};

// BlockExprAst - Expression for a block of statements.
class BlockExprAst : public ExprAst {
    deque<ExprAst*>* stmtList;
public:
    BlockExprAst(deque<ExprAst*>* statementList);
    virtual Value* generateCode();
};

// ArrayAssignExprAst - Expression for assigning a value to an index of an array.
class ArrayAssignExprAst : public ExprAst {
    char* id;
    ExprAst* indexExpr;
    ExprAst* assignExpr;
public:
    ArrayAssignExprAst(char* identifier, ExprAst* indexExpression,  ExprAst* assignExpression);
    virtual Value* generateCode();
};

// VarDeclExprAst - Expression for variable declarations.
class VarDeclExprAst : public ExprAst {
    Type* type;
    char* id;
public:
    VarDeclExprAst(Type* dataType, char* identifier);
    virtual Value* generateCode();
};

// VarAssignExprAst - Expression for variable assignments.
class VarAssignExprAst : public ExprAst {
    char* id;
    ExprAst* expr;
public:
    VarAssignExprAst(char* identifier, ExprAst* expression);
    virtual Value* generateCode();
};

// BinaryExprAst - Expression for binary expressions.
class BinaryExprAst : public ExprAst {
    char* op;
    ExprAst* lExpr;
    ExprAst* rExpr;
public:
    BinaryExprAst(char* operation, ExprAst* leftExpression, ExprAst* rightExpression);
    virtual Value* generateCode();
};

// UnaryExprAst - Expression for unary expressions.
class UnaryExprAst : public ExprAst {
    char* op;
    ExprAst* expr;
public:
    UnaryExprAst(char* operation, ExprAst* expression);
    virtual Value* generateCode();
};

// FunctionCallExprAst - Expression for calling a function.
class FunctionCallExprAst : public ExprAst {
    char* id;
    deque<ExprAst*>* args;
public:
    FunctionCallExprAst(char* identifier, deque<ExprAst*>* arguments);
    virtual Value* generateCode();
};

// VarExprAst - Expression for accesing variables.
class VarExprAst : public ExprAst {
    char* id;
public:
    VarExprAst(char* identifier);
    virtual Value* generateCode();
};

// ArrayExprAst - Expression for accessing an index of an array.
class ArrayExprAst : public ExprAst {
    char* id;
    ExprAst* indexExpr;
public:
    ArrayExprAst(char* identifier, ExprAst* indexExpression);
    virtual Value* generateCode();
};

// IntConstExprAst - Expression for integer constants.
class IntConstExprAst : public ExprAst {
    int val;
public:
    IntConstExprAst(int value);
    virtual Value* generateCode();
};

// BoolConstExprAst - Expression for boolean constants.
class BoolConstExprAst : public ExprAst {
    bool val;
public:
    BoolConstExprAst(bool value);
    virtual Value* generateCode();
};

// StringConstExprAst - Expression for string constants.
class StringConstExprAst : public ExprAst {
    string* val;
public:
    StringConstExprAst(char* value);
    virtual Value* generateCode();
};


#endif
