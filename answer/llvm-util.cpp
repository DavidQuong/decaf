#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm-util.h"
#include "symbol-table.h"
#include "value-constants.h"
#include <stdexcept>
#include <string>
#include <vector>

using namespace llvm;
using namespace std;

const char* MODULE_NAME = "Test";
const char* BRANCH_ENTRY = "entry";
const char* BRANCH_END = "end";
const char* BRANCH_LOOP = "loop";
const char* BRANCH_BODY = "body";
const char* BRANCH_NEXT = "next";
const char* BRANCH_SKCTEND = "skctend";
const char* BRANCH_NOSKCT = "noskct";
const char* BRANCH_IFSTART = "ifstart";
const char* BRANCH_IFTRUE = "iftrue";
const char* BRANCH_IFFALSE = "iffalse";

static Module* codeModule;
static IRBuilder<>* irBuilder;

/* Initialize LLVM components. */
void initializeLLVM() {
    codeModule = new Module(MODULE_NAME, getGlobalContext());
    irBuilder = new IRBuilder<>(getGlobalContext());
}

/* Return the created module, used to reference current code context. */
Module* getModule() {
    return codeModule;
}

/* Return the builder, used to build code generation. */
IRBuilder<>* getBuilder() {
    return irBuilder;
}

/* Return the the type corresponding to the provided string. */
Type* getLLVMType(const char* typeStr) {
    Type* type = NULL;

    if (strcmp(typeStr, VALUE_VOIDTYPE) == 0) {
        type = irBuilder->getVoidTy();
    } else if (strcmp(typeStr, VALUE_INTTYPE) == 0) {
        type = irBuilder->getInt32Ty();
    } else if (strcmp(typeStr, VALUE_BOOLTYPE) == 0) {
        type = irBuilder->getInt1Ty();
    } else if (strcmp(typeStr, VALUE_STRINGTYPE) == 0) {
        type = irBuilder->getInt8PtrTy();
    } else {
        throw runtime_error("Unknown data type.\n");
    }

    return type;
}

/* Create the default return statement for the provided Type. */
Value* createDefaultReturn(Type* type) {
    Value* returnVal;
    if (type == getLLVMType(VALUE_INTTYPE)) {
        returnVal = getBuilder()->CreateRet(getIntConstant(EXIT_SUCCESS));
    } else if (type == getLLVMType(VALUE_BOOLTYPE)) {
        returnVal = getBuilder()->CreateRet(getBoolConstant(EXIT_SUCCESS));
    } else if (type == getLLVMType(VALUE_VOIDTYPE)) {
        returnVal = getBuilder()->CreateRetVoid();
    } else {
        throw runtime_error("Invalid return type.\n");
    }

    return returnVal;
}


/* Return the LLVM value representation of the provided integer. */
Value* getIntConstant(int value) {
    return irBuilder->getInt32(value);
}

/* Return the LLVm value representation of the provided boolean. */
Value* getBoolConstant(bool value) {
    return irBuilder->getInt1(value);
}

/* Convert the provided value, which should be of boolean type, to an integer. 
   This works for both constants and variables. */
Value* convertBoolToInt(Value* value) {
    return irBuilder->CreateZExt(value, getLLVMType(VALUE_INTTYPE), "zexttmp");
}


/* Create an extern function with the provided return type, name and parameter types. */
Function* createExternFunction(Type* returnType, char* id, vector<Type*>* parameterTypes) {
    FunctionType* type = FunctionType::get(returnType, *parameterTypes, false);
    Function* function = Function::Create(type, Function::ExternalLinkage, id, codeModule);
    
    insertExternSymbol(id, function);
    return function;
}

/* Declare a function with the following return type and id. */
Function* createFunctionHeader(Type* returnType, char* id) {
    FunctionType* type = FunctionType::get(returnType, false);
    Function* function = Function::Create(type, Function::ExternalLinkage, id, codeModule);

    insertFunctionSymbol(id, function);
    return function;
}

/* Create a global string definition for the provided string and return the created value. */
Value* createString(const char* str) {
    Value* value = irBuilder->CreateGlobalString(str, "globalstring");
    return irBuilder->CreateConstGEP2_32(value, 0, 0, "cast");
}

/* Create an array of the provided type with the provided id and size. */
Value* createArray(Type* type, char* id, int size) {
    ArrayType* arrayType = ArrayType::get(type, size);
    
    // Initialize all values to zeroes.
    Constant* zeroInitializer = Constant::getNullValue(arrayType);
    
    //Create global variable to array.
    GlobalVariable* variable = new GlobalVariable(*codeModule, arrayType, false, GlobalValue::ExternalLinkage, zeroInitializer, id);
    insertSymbol(id, variable);

    return variable;
}

/* Create a global scalar variable with the provided type and id, and initialize it with the provided value. */
Value* createGlobalScalar(Type* type, char* id, Constant* value) {
    GlobalVariable* variable = new GlobalVariable(*codeModule, type, false, GlobalValue::ExternalLinkage, value, id);
    insertSymbol(id, variable); 

    return variable;
}

/* Assign the provided value to the array with the provided id and index. */
Value* assignArrayIndex(char* id, Value* index, Value* value) {
    Value* array = getValue(id);
    if (array == NULL) {
    }

    // Get array location.
    Value* arrayLoc = irBuilder->CreateStructGEP(array, 0, "arrayloc");

    // Get location of array index.
    Value* arrayIndex = irBuilder->CreateGEP(arrayLoc, index, "arrayindex");

    // Store the provided value at the location of the array index.
    return irBuilder->CreateStore(value, arrayIndex);
}

/* Access the array with the provided id at the provided index. */
Value* accessArrayIndex(char* id, Value* index) {
    Value* array = getValue(id);
    if (array == NULL) {
        throwError(ERROR_VARIABLE_UNDECLARED, EXIT_VARIABLE_UNDECLARED);
    }

    // Get array location.
    Value* arrayLoc = irBuilder->CreateStructGEP(array, 0, "arrayloc");

    // Get location of array index.
    Value* arrayIndex = irBuilder->CreateGEP(arrayLoc, index, "arrayindex");

    // Store results in temporary variable, and return it for use.
    return irBuilder->CreateLoad(arrayIndex, "arrayval");
}

/* Declare a variable with the provided type and id. */
Value* declareVariable(Type* type, char* id) {
    Value* variable = irBuilder->CreateAlloca(type, 0, id);
    insertSymbol(id, variable);
    return variable;
}

/* Assign the variable with the provided id with the provided value. */
Value* assignVariable(char* id, Value* value) {
    Value* variable = getValue(id);
    if (variable == NULL) {
        throwError(ERROR_VARIABLE_UNDECLARED, EXIT_VARIABLE_UNDECLARED);
    }

    // Check if variable and value are same type
    Type* variableType = variable->getType()->getContainedType(0);
    Type* valueType = value->getType();
    if (variableType != valueType) { 
        if (variableType == getLLVMType(VALUE_INTTYPE)) {
            throwError(ERROR_BOOL_TO_INT, EXIT_ASSIGN_TYPE_MISMATCH);
        } else if (variableType == getLLVMType(VALUE_BOOLTYPE)) {
            throwError(ERROR_INT_TO_BOOL, EXIT_ASSIGN_TYPE_MISMATCH);
        } else {
            throw runtime_error("Type mismatch.\n");
        }
    }

    return irBuilder->CreateStore(value, variable);
}

/* Access the variable with the provided id. */
Value* accessVariable(char* id) {
    Value* value = getValue(id);
    if (value == NULL) {
        throwError(ERROR_VARIABLE_UNDECLARED, EXIT_VARIABLE_UNDECLARED);
    }

    return irBuilder->CreateLoad(value, false, id);
}

/* Call the function with the provided id, using the providing parameters. */
Value* callFunction(char* id, vector<Value*>* args) {
    // Ensure function exists within scope.
    Function* function = (Function*) getValue(id);
    if (function == NULL) {
        throw runtime_error("Function " + string(id) + " not found.\n");
    }
        
    // Ensure same number of arguments are provided.
    if (args->size() != function->getArgumentList().size()) {
        throw runtime_error("Invalid arguments for function " + string(id) + ".\n");
    }
        
    // Implicitly convert booleans to integers if the function parameter indicates to.
    vector<Value*> convertedArgs;
    int i = 0;
    for (Function::arg_iterator paramIt = function->arg_begin(); paramIt != function->arg_end(); paramIt++) {
        Argument* param = paramIt;
        Value* arg = args->at(i);
        
        if (param->getType() == getLLVMType(VALUE_INTTYPE) && arg->getType() == getLLVMType(VALUE_BOOLTYPE)) {
            Value* convertedArg = convertBoolToInt(arg);
            convertedArgs.push_back(convertedArg);
        } else {
            convertedArgs.push_back(arg);
        }

        i++;
    }

    return irBuilder->CreateCall(function, convertedArgs);
}

/* Create a store value for the provided parameter, as well as store in the symbol table. */
Value* storeParameter(Type* type, char* id, Argument* parameter) {
    // Allocate pointer
    Value* allocVal = irBuilder->CreateAlloca(type, 0, id);
    
    // Store argument in pointer
    irBuilder->CreateStore(parameter, allocVal);
    insertSymbol(id, allocVal);

    return allocVal;
}

// Perform the operation on the provided left and right values.
Value* computeBinaryExpression(char* op, Value* leftValue, Value* rightValue) {
    Value* value;

    if (strcmp(op, VALUE_EQ) == 0) {
        validateBothSameType(leftValue, rightValue);
        value = irBuilder->CreateICmpEQ(leftValue, rightValue, "eqtmp");
    } else if (strcmp(op, VALUE_NEQ) == 0) {
        validateBothSameType(leftValue, rightValue);
        value = irBuilder->CreateICmpNE(leftValue, rightValue, "neq");
    } else if (strcmp(op, VALUE_LT) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateICmpSLT(leftValue, rightValue, "lttmp");
    } else if (strcmp(op, VALUE_LEQ) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateICmpSLE(leftValue, rightValue, "leqtmp");
    } else if (strcmp(op, VALUE_GT) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateICmpSGT(leftValue, rightValue, "gttmp");
    } else if (strcmp(op, VALUE_GEQ) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateICmpSGE(leftValue, rightValue, "geqtmp");
    } else if (strcmp(op, VALUE_PLUS) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateAdd(leftValue, rightValue, "addtmp");
    } else if (strcmp(op, VALUE_MINUS) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateSub(leftValue, rightValue, "subtmp");
    } else if (strcmp(op, VALUE_MULT) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateMul(leftValue, rightValue, "multmp");
    } else if (strcmp(op, VALUE_DIV) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateSDiv(leftValue, rightValue, "divtmp");
    } else if (strcmp(op, VALUE_MOD) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateSRem(leftValue, rightValue, "modtmp");
    } else if (strcmp(op, VALUE_RIGHTSHIFT) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateAShr(leftValue, rightValue, "rshifttmp");
    } else if (strcmp(op, VALUE_LEFTSHIFT) == 0) {
        validateBothIntType(leftValue, rightValue);
        value = irBuilder->CreateShl(leftValue, rightValue, "lshifttmp");
    } else {
        throw runtime_error("Invalid binary operation.\n");
    }

    return value;
}

// Ensure both values are of int type.
void validateBothIntType(Value* leftValue, Value* rightValue) {
    Type* intType = getLLVMType(VALUE_INTTYPE); 
    Type* leftType = leftValue->getType();
    Type* rightType = rightValue->getType();
    
    if (leftType != intType || rightType != intType) {
        throwError(ERROR_INVALID_BOOL_OP, EXIT_COMPUTE_TYPE_MISMATCH);
    }
}

// Ensure both values are of bool type.
void validateBothBoolType(Value* leftValue, Value* rightValue) {
    Type* boolType = getLLVMType(VALUE_BOOLTYPE);
    Type* leftType = leftValue->getType();
    Type* rightType = rightValue->getType();
    
    if (leftType != boolType || rightType != boolType) {
        throwError(ERROR_INVALID_INT_OP, EXIT_COMPUTE_TYPE_MISMATCH);
    }
}

// Ensure both values are of the same type.
void validateBothSameType(Value* leftValue, Value* rightValue) {
    Type* leftType = leftValue->getType();
    Type* rightType = rightValue->getType();
    
    if (leftType != rightType) {
        throwError(ERROR_BINARY_OP_TYPE_MISMATCH, EXIT_COMPUTE_TYPE_MISMATCH);
    }
}

// Perform the operation on the provided value.
Value* computeUnaryExpression(char* op, Value* value) {
    Type* valueType = value->getType();
    Value* returnValue;

    if (strcmp(op, VALUE_NOT) == 0) {
        if (valueType == getLLVMType(VALUE_INTTYPE)) {
            throwError(ERROR_NOT_INT, EXIT_COMPUTE_TYPE_MISMATCH);
        }
        returnValue = irBuilder->CreateNot(value, "nottmp");
    } else if (strcmp(op, VALUE_NEGATE) == 0) {
        if (valueType == getLLVMType(VALUE_BOOLTYPE)) {
            throwError(ERROR_NEGATE_BOOL, EXIT_COMPUTE_TYPE_MISMATCH);
        }
        returnValue = irBuilder->CreateSub(irBuilder->getInt32(0), value, "negtmp");
    } else {
        throw runtime_error("Invalid unary operation\n");
    }

    return returnValue;
}
