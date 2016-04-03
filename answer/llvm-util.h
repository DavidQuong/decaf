#ifndef LLVM_UTIL_H
#define LLVM_UTIL_H

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "value-constants.h"
#include <stdexcept>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

extern const char* MODULE_NAME;
extern const char* BRANCH_ENTRY;

void initializeLLVM();
Module* getModule();
IRBuilder<>* getBuilder();
Type* getLLVMType(const char* typeStr);
Value* createReturn(Type* type);
Value* getIntConstant(int value);
Value* getBoolConstant(bool value);
Value* convertBoolToInt(Value* value);
Function* createExternFunction(Type* returnType, char* id, vector<Type*>* parameterTypes);
Function* createFunctionHeader(Type* returnType, char* id);
Value* createString(const char* str);
Value* createGlobalScalar(Type* type, char* id, Constant* value);

Value* declareVariable(Type* type, char* id);
Value* assignVariable(char* id, Value* value);
Value* accessVariable(char* id);
Value* callFunction(char* id, vector<Value*>* args);
Value* storeParameter(Type* type, char* id, Argument* parameter);
Value* computeBinaryExpression(char* op, Value* leftValue, Value* rightValue);
Value* computeUnaryExpression(char* op, Value* value);

#endif
