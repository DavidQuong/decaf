#include "expr-asts.h"
#include "llvm-util.h"
#include "symbol-table.h"
#include <cstdio>
#include <vector>

static vector<BasicBlock*>* continueBlockList = new vector<BasicBlock*>;
static vector<BasicBlock*>* breakBlockList = new vector<BasicBlock*>;

ExternExprAst::ExternExprAst(Type* returnType, char* identifier, vector<Type*>* parameterTypes) {
    type = returnType;
    id = identifier;
    paramTypes = parameterTypes;
}
Value* ExternExprAst::generateCode() {
    return createExternFunction(type, id, paramTypes);
}

FieldVarDeclExprAst::FieldVarDeclExprAst(Type* dataType, char* identifier, int quantity) {
    type = dataType;
    id = identifier;
    size = quantity;
}
Value* FieldVarDeclExprAst::generateCode() {
    Value* value;

    if (size == VALUE_SCALAR) { // For scalar variables
        if (type == getLLVMType(VALUE_INTTYPE)) { // For integer type
            Constant* constVal = (Constant*) getIntConstant(0);
            value = createGlobalScalar(type, id, constVal);
        } else { // For boolean type
            Constant* constVal = (Constant*) getBoolConstant(false);
            value = createGlobalScalar(type, id, constVal);
        }
    } else if (0 < size)  { // For array variables
        value = createArray(type, id, size);
    } else { // For arrays of size 0;
        throw runtime_error("Invalid error size, must be at least 1.\n");
    }

    return value;
}

FieldVarDefExprAst::FieldVarDefExprAst(Type* dataType, char* identifier, ExprAst* initialValue) {
    type = dataType;
    id = identifier;
    value = (Constant*) initialValue->generateCode();
}
Value* FieldVarDefExprAst::generateCode() {
   return createGlobalScalar(type, id, value);
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

    for (deque<ExprAst*>::iterator it = stmtList->begin(); it != stmtList->end(); it++) {
        ExprAst* expr = *it;
        expr->generateCode();
    }

    createDefaultReturn(type);
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

ForBlockExprAst::ForBlockExprAst(deque<ExprAst*>* initAssignList, ExprAst* conditionExpression, deque<ExprAst*>* updateAssignList, ExprAst* blockExpression) {
    initList = initAssignList;
    condExpr = conditionExpression;
    updateList = updateAssignList;
    blockExpr = blockExpression;
}
Value* ForBlockExprAst::generateCode() {
    if (initList->empty()) {
        throw runtime_error("For loop variable initialization assignments list is empty.\n");
    } else if (updateList->empty()) {
        throw runtime_error("For loop variable update assignments list is empty.\n");
    }

    Function* currentFunction = getBuilder()->GetInsertBlock()->getParent();
    BasicBlock* loopBlock = BasicBlock::Create(getGlobalContext(), BRANCH_LOOP, currentFunction);
    BasicBlock* bodyBlock = BasicBlock::Create(getGlobalContext(), BRANCH_BODY, currentFunction);
    BasicBlock* nextBlock = BasicBlock::Create(getGlobalContext(), BRANCH_NEXT, currentFunction);
    BasicBlock* endBlock = BasicBlock::Create(getGlobalContext(), BRANCH_END, currentFunction);

    // Add entry/exit blocks to list for continue/break statements.
    continueBlockList->push_back(nextBlock);
    breakBlockList->push_back(endBlock);

    // Generate variable initialization.
    for (deque<ExprAst*>::iterator it = initList->begin(); it != initList->end(); it++) {
        ExprAst* expr = *it;
        expr->generateCode();
    }

    // Create block and branch to loop.
    getBuilder()->CreateBr(loopBlock);
    // Start enty point for loop.
    getBuilder()->SetInsertPoint(loopBlock);
    Value* conditionValue = condExpr->generateCode();

    // Create conditional branch to either body or end.
    getBuilder()->CreateCondBr(conditionValue, bodyBlock, endBlock);

    // Create body block and branch to next.
    getBuilder()->SetInsertPoint(bodyBlock);
    blockExpr->generateCode();
    getBuilder()->CreateBr(nextBlock);

    // Create next block and branch back to loop.
    getBuilder()->SetInsertPoint(nextBlock);
    // Generte variable re-assignments.
    for (deque<ExprAst*>::iterator it = updateList->begin(); it != updateList->end(); it++) {
        ExprAst* expr = *it;
        expr->generateCode();
    }
    getBuilder()->CreateBr(loopBlock);

    // Create return (end) point.
    getBuilder()->SetInsertPoint(endBlock); 

    // Remove entry/exit blocks from list.
    continueBlockList->pop_back();
    breakBlockList->pop_back();
}

WhileBlockExprAst::WhileBlockExprAst(ExprAst* conditionExpression, ExprAst* blockExpression) {
    condExpr = conditionExpression;
    blockExpr = blockExpression;
}
Value* WhileBlockExprAst::generateCode() {
    Function* currentFunction = getBuilder()->GetInsertBlock()->getParent();
    BasicBlock* loopBlock = BasicBlock::Create(getGlobalContext(), BRANCH_LOOP, currentFunction);
    BasicBlock* bodyBlock = BasicBlock::Create(getGlobalContext(), BRANCH_BODY, currentFunction);
    BasicBlock* endBlock = BasicBlock::Create(getGlobalContext(), BRANCH_END, currentFunction);
    
    // Add entry/exit blocks to list for continue/break statements.
    continueBlockList->push_back(loopBlock);
    breakBlockList->push_back(endBlock);

    // Create block and branch to loop.
    getBuilder()->CreateBr(loopBlock);
    // Start entry point for loop.
    getBuilder()->SetInsertPoint(loopBlock);
    Value* conditionValue = condExpr->generateCode();

    // Create conditional branch to either body or end.
    getBuilder()->CreateCondBr(conditionValue, bodyBlock, endBlock);

    // Insert body block code.
    getBuilder()->SetInsertPoint(bodyBlock);
    blockExpr->generateCode();
    getBuilder()->CreateBr(loopBlock);

    // Create return (end) point.
    getBuilder()->SetInsertPoint(endBlock);

    // Remove entry/exit blocks from list.
    continueBlockList->pop_back();
    breakBlockList->pop_back();
}

IfBlockExprAst::IfBlockExprAst(ExprAst* conditionExpression, ExprAst* blockExpression) {
    condExpr = conditionExpression;
    blockExpr = blockExpression;
}
Value* IfBlockExprAst::generateCode() {
    Function* currentFunction = getBuilder()->GetInsertBlock()->getParent();
    BasicBlock* ifstartBlock = BasicBlock::Create(getGlobalContext(), BRANCH_IFSTART, currentFunction);
    BasicBlock* iftrueBlock = BasicBlock::Create(getGlobalContext(), BRANCH_IFTRUE, currentFunction);
    BasicBlock* endBlock = BasicBlock::Create(getGlobalContext(), BRANCH_END, currentFunction);

    // Branch to ifstart.
    getBuilder()->CreateBr(ifstartBlock);

    // Start entry point for ifstart block, and
    // insert conditional expression code.
    getBuilder()->SetInsertPoint(ifstartBlock);
    Value* conditionValue = condExpr->generateCode();

    // Create conditional branch to either iftrue or end.
    getBuilder()->CreateCondBr(conditionValue, iftrueBlock, endBlock);

    // Insert iftrue block code.
    getBuilder()->SetInsertPoint(iftrueBlock);
    blockExpr->generateCode();
    getBuilder()->CreateBr(endBlock);

    // Create return (end) point.
    getBuilder()->SetInsertPoint(endBlock);
}

IfElseBlockExprAst::IfElseBlockExprAst(ExprAst* conditionExpression, ExprAst* trueBlockExpression, ExprAst* falseBlockExpression) {
    condExpr = conditionExpression;
    trueBlockExpr = trueBlockExpression;
    falseBlockExpr = falseBlockExpression;
}
Value* IfElseBlockExprAst::generateCode() {
    Function* currentFunction = getBuilder()->GetInsertBlock()->getParent();
    BasicBlock* ifstartBlock = BasicBlock::Create(getGlobalContext(), BRANCH_IFSTART, currentFunction);
    BasicBlock* iftrueBlock = BasicBlock::Create(getGlobalContext(), BRANCH_IFTRUE, currentFunction);
    BasicBlock* iffalseBlock = BasicBlock::Create(getGlobalContext(), BRANCH_IFFALSE, currentFunction);
    BasicBlock* endBlock = BasicBlock::Create(getGlobalContext(), BRANCH_END, currentFunction);

    // Branch to ifstart.
    getBuilder()->CreateBr(ifstartBlock);
    
    // Start entry point for ifstart block, and
    // insert conditional expression code.
    getBuilder()->SetInsertPoint(ifstartBlock);
    Value* conditionValue = condExpr->generateCode();

    // Create conditional branch to either iftrue or end.
    getBuilder()->CreateCondBr(conditionValue, iftrueBlock, iffalseBlock);

    // Insert ifstart block code.
    getBuilder()->SetInsertPoint(iftrueBlock);
    trueBlockExpr->generateCode();
    getBuilder()->CreateBr(endBlock);

    // Insert iffalse block code.
    getBuilder()->SetInsertPoint(iffalseBlock);
    falseBlockExpr->generateCode();
    getBuilder()->CreateBr(endBlock);

    // Create return (end) point.
    getBuilder()->SetInsertPoint(endBlock);
}

ReturnExprAst::ReturnExprAst(ExprAst* expression) {
    expr = expression;
}
Value* ReturnExprAst::generateCode() {
    Value* value;
    if (expr != NULL) {
        value = expr->generateCode();
        
        // Ensure value is same type as return type.
        Type* returnType = getBuilder()->GetInsertBlock()->getParent()->getReturnType();
        if (value->getType() != returnType) {
            throwError(ERROR_RETURN_MISMATCH, EXIT_COMPUTE_TYPE_MISMATCH);
        }

        getBuilder()->CreateRet(value);
    } else {
        Type* type = getLLVMType(VALUE_VOIDTYPE);
        value = createDefaultReturn(type);
    }
    
    return value;
}

BreakExprAst::BreakExprAst() {
}
Value* BreakExprAst::generateCode() {
    BasicBlock* exitBlock = breakBlockList->back();
    getBuilder()->CreateBr(exitBlock);
}

ContinueExprAst::ContinueExprAst() {
}
Value* ContinueExprAst::generateCode() {
    BasicBlock* nextBlock = continueBlockList->back();
    getBuilder()->CreateBr(nextBlock);
}

ArrayAssignExprAst::ArrayAssignExprAst(char* identifier, ExprAst* indexExpression,  ExprAst* assignExpression) {
    id = identifier;
    indexExpr = indexExpression;
    assignExpr = assignExpression;
}
Value* ArrayAssignExprAst::generateCode() {
    Value* index = indexExpr->generateCode();
    Value* value = assignExpr->generateCode();

    return assignArrayIndex(id, index, value);
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

ArrayExprAst::ArrayExprAst(char* identifier, ExprAst* indexExpression) {
    id = identifier;
    indexExpr = indexExpression;
}
Value* ArrayExprAst::generateCode() {
    Value* index = indexExpr->generateCode();

    return accessArrayIndex(id, index);
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
