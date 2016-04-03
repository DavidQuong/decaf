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
extern const char* BRANCH_END;
extern const char* BRANCH_LOOP;
extern const char* BRANCH_BODY;
extern const char* BRANCH_NEXT;
extern const char* BRANCH_NOSKCT;
extern const char* BRANCH_SKCTEND;
extern const char* BRANCH_IFSTART;
extern const char* BRANCH_IFTRUE;
extern const char* BRANCH_IFFALSE;


void initializeLLVM();
Module* getModule();
IRBuilder<>* getBuilder();
Type* getLLVMType(const char* typeStr);
Value* createDefaultReturn(Type* type);
Value* getIntConstant(int value);
Value* getBoolConstant(bool value);
Value* convertBoolToInt(Value* value);
Function* createExternFunction(Type* returnType, char* id, vector<Type*>* parameterTypes);
Function* createFunctionHeader(Type* returnType, char* id);

Value* createString(const char* str);
Value* createArray(Type* type, char* id, int size);
Value* createGlobalScalar(Type* type, char* id, Constant* value);
Value* assignArrayIndex(char* id, Value* index, Value* value);
Value* accessArrayIndex(char* id, Value* index);
Value* declareVariable(Type* type, char* id);
Value* assignVariable(char* id, Value* value);
Value* accessVariable(char* id);
Value* callFunction(char* id, vector<Value*>* args);
Value* storeParameter(Type* type, char* id, Argument* parameter);
Value* computeBinaryExpression(char* op, Value* leftValue, Value* rightValue);
Value* computeUnaryExpression(char* op, Value* value);

#endif
