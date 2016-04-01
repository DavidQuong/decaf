#include "expr-asts.h"
#include "llvm-util.h"
#include "symbol-table.h"
#include <cstdio>

ExternExprAst::ExternExprAst(Type* returnType, char* identifier, vector<Type*>* parameterTypes) {
    type = returnType;
    id = identifier;
    paramTypes = parameterTypes;
}
Value* ExternExprAst::generateCode() {
    return createExternFunction(type, id, paramTypes);
}

FunctionExprAst::FunctionExprAst(Type* returnType, char* identifier, vector<pair<Type*,char*>*>* parameterList, deque<ExprAst*>* statementList) {
    type = returnType;
    id = identifier;
    paramList = parameterList;
    stmtList = statementList;
    function = NULL;
    block = NULL;
}
Value* FunctionExprAst::generateCode() {
    function = createFunctionHeader(type, id);
    block = BasicBlock::Create(getGlobalContext(), BRANCH_ENTRY, function);

    // Add parameters (Arguments) to function header, no code generated yet.
    for (vector<pair<Type*,char*>*>::iterator it = paramList->begin(); it != paramList->end(); it++) {
        pair<Type*,char*>* param = *it;
        Type* paramType = param->first;
        char* paramId = param->second;

        Argument* createdParam = new Argument(paramType, paramId, function);
        FunctionParamExprAst* paramExpr = new FunctionParamExprAst(paramType, paramId, createdParam);
        stmtList->push_front(paramExpr);
    }
    
    return function;
}
void FunctionExprAst::generateDeferedCode() {
    getBuilder()->SetInsertPoint(block);
    pushSymbolTable();

    /* // Add parameters (Arguments) to function header. */
    /* for (vector<pair<Type*,char*>*>::iterator it = paramList->begin(); it != paramList->end(); it++) { */
    /*     pair<Type*,char*>* param = *it; */
    /*     Type* paramType = param->first; */
    /*     char* paramId = param->second; */
    /*     Argument* argument = new Argument(paramType, paramId, function); */
    /*     storeParameter(paramType, paramId, argument); */
    /* } */
    
    for (deque<ExprAst*>::iterator it = stmtList->begin(); it != stmtList->end(); it++) {
        ExprAst* expr = *it;
        expr->generateCode();
    }

    createReturn(type);
    popSymbolTable();
}

FunctionParamExprAst::FunctionParamExprAst(Type* dataType, char* identifier, Argument* parameter) {
    type = dataType;
    id = identifier;
    param = parameter;
}
Value* FunctionParamExprAst::generateCode() {
    storeParameter(type, id, param);
}

BlockExprAst::BlockExprAst(deque<ExprAst*>* statementList) {
    stmtList = statementList;
}
Value* BlockExprAst::generateCode() {
    pushSymbolTable();

    for (deque<ExprAst*>::iterator it = stmtList->begin(); it != stmtList->end(); it++) {
        ExprAst* expr = *it;
        expr->generateCode();
    }

    popSymbolTable();
}

VarDeclExprAst::VarDeclExprAst(Type* dataType, char* identifier) {
    type = dataType;
    id = identifier;
}
Value* VarDeclExprAst::generateCode() {
    return declareVariable(type, id);
}

VarAssignExprAst::VarAssignExprAst(char* identifier, ExprAst* expression) {
    id = identifier;
    expr = expression;
}
Value* VarAssignExprAst::generateCode() {
    Value* value = expr->generateCode();
    return assignVariable(id, value);
}

BinaryExprAst::BinaryExprAst(char* operation, ExprAst* leftExpression, ExprAst* rightExpression) {
    op = operation;
    lExpr = leftExpression;
    rExpr = rightExpression;
}
Value* BinaryExprAst::generateCode() {
    Value* lValue = lExpr->generateCode();
    Value* rValue = rExpr->generateCode();

    return computeBinaryExpression(op, lValue, rValue);
}

UnaryExprAst::UnaryExprAst(char* operation, ExprAst* expression) {
    op = operation;
    expr = expression;
}
Value* UnaryExprAst::generateCode() {
    Value* value = expr->generateCode();

    return computeUnaryExpression(op, value);
}

FunctionCallExprAst::FunctionCallExprAst(char* identifier, deque<ExprAst*>* arguments) {
    id = identifier;
    args = arguments;
}
Value* FunctionCallExprAst::generateCode() {
    vector<Value*>* argValues = new vector<Value*>;

    // Evaluate argument expressions
    for (deque<ExprAst*>::iterator it = args->begin(); it != args->end(); it++) {
        ExprAst* expr = *it;
        Value* value = expr->generateCode();
        argValues->push_back(value);
    }

    return callFunction(id, argValues);
}

VarExprAst::VarExprAst(char* identifier) {
    id = identifier;
}
Value* VarExprAst::generateCode() {
    return accessVariable(id);
}

IntConstExprAst::IntConstExprAst(int value) {
    val = value;
}
Value* IntConstExprAst::generateCode() {
    return getIntConstant(val);
}

BoolConstExprAst::BoolConstExprAst(bool value) {
    val = value;
}
Value* BoolConstExprAst::generateCode() {
    return getBoolConstant(val);
}

StringConstExprAst::StringConstExprAst(char* value) {
    val = new string(value);
    val->erase(0, 1);
    val->erase(val->size() - 1);
}
Value* StringConstExprAst::generateCode() {
    return createString(val->c_str());
}
