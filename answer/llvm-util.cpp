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
        throw std::runtime_error("Unknown data type.\n");
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
        throw runtime_error("Variable " + string(id) + " does not exist.\n");
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
        throw runtime_error("Variable " + string(id) + " does not exist.\n");
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
        throw runtime_error("Variable " + string(id) + " does not exist.\n");
    }

    return irBuilder->CreateStore(value, variable);
}

/* Access the variable with the provided id. */
Value* accessVariable(char* id) {
    Value* value = getValue(id);
    if (value == NULL) {
        throw runtime_error("Variable " + string(id) + " has not been declared in this scope.\n");
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

    if (strcmp(op, VALUE_OR) == 0) {
        BasicBlock* currentBlock = getBuilder()->GetInsertBlock();
        Function* currentFunction = currentBlock->getParent();
        BasicBlock* noskctBlock = BasicBlock::Create(getGlobalContext(), BRANCH_NOSKCT, currentFunction); 
        BasicBlock* skctendBlock = BasicBlock::Create(getGlobalContext(), BRANCH_SKCTEND, currentFunction);
        
        // If left value evaluates to true, then skip checking right part of expression by
        // branching to skctend (short circuit), otherwise branch to noskct (no short circuit).
        irBuilder->CreateCondBr(leftValue, skctendBlock, noskctBlock);

        // Insert code for noskct.
        irBuilder->SetInsertPoint(noskctBlock);
        Value* result = irBuilder->CreateOr(leftValue, rightValue, "ortmp");
        irBuilder->CreateBr(skctendBlock);
        
        // Insert code for skctend and generate additional code for or expression
        irBuilder->SetInsertPoint(skctendBlock);
        // Create short terminating code.
        PHINode* node = irBuilder->CreatePHI(getLLVMType(VALUE_BOOLTYPE), 2, "phival");
        node->addIncoming(leftValue, currentBlock);
        node->addIncoming(result, noskctBlock);

        value = node;
        
    } else if (strcmp(op, VALUE_AND) == 0) {
        BasicBlock* currentBlock = getBuilder()->GetInsertBlock();
        Function* currentFunction = currentBlock->getParent();
        BasicBlock* noskctBlock = BasicBlock::Create(getGlobalContext(), BRANCH_NOSKCT, currentFunction); 
        BasicBlock* skctendBlock = BasicBlock::Create(getGlobalContext(), BRANCH_SKCTEND, currentFunction);
    
        // If left value evaluates to false, then skip checking right part of expression by
        // branching to skctend (short circuit), otherwise branch to noskct (no short circuit).
        irBuilder->CreateCondBr(leftValue, noskctBlock, skctendBlock);

        // Insert code for noskct.
        irBuilder->SetInsertPoint(noskctBlock);
        Value* result = irBuilder->CreateAnd(leftValue, rightValue, "andtmp");
        irBuilder->CreateBr(skctendBlock);
        
        // Insert code for skctend and generate additional code for or expression
        irBuilder->SetInsertPoint(skctendBlock);
        PHINode* node = irBuilder->CreatePHI(getLLVMType(VALUE_BOOLTYPE), 2, "phival");
        node->addIncoming(leftValue, currentBlock);
        node->addIncoming(result, noskctBlock);

        value = node;
        
    } else if (strcmp(op, VALUE_EQ) == 0) {
        value = irBuilder->CreateICmpEQ(leftValue, rightValue, "eqtmp");
    } else if (strcmp(op, VALUE_NEQ) == 0) {
        value = irBuilder->CreateICmpNE(leftValue, rightValue, "neq");
    } else if (strcmp(op, VALUE_LT) == 0) {
        value = irBuilder->CreateICmpSLT(leftValue, rightValue, "lttmp");
    } else if (strcmp(op, VALUE_LEQ) == 0) {
        value = irBuilder->CreateICmpSLE(leftValue, rightValue, "leqtmp");
    } else if (strcmp(op, VALUE_GT) == 0) {
        value = irBuilder->CreateICmpSGT(leftValue, rightValue, "gttmp");
    } else if (strcmp(op, VALUE_GEQ) == 0) {
        value = irBuilder->CreateICmpSGE(leftValue, rightValue, "geqtmp");
    } else if (strcmp(op, VALUE_PLUS) == 0) {
        value = irBuilder->CreateAdd(leftValue, rightValue, "addtmp");
    } else if (strcmp(op, VALUE_MINUS) == 0) {
        value = irBuilder->CreateSub(leftValue, rightValue, "subtmp");
    } else if (strcmp(op, VALUE_MULT) == 0) {
        value = irBuilder->CreateMul(leftValue, rightValue, "multmp");
    } else if (strcmp(op, VALUE_DIV) == 0) {
        value = irBuilder->CreateSDiv(leftValue, rightValue, "divtmp");
    } else if (strcmp(op, VALUE_MOD) == 0) {
        value = irBuilder->CreateSRem(leftValue, rightValue, "modtmp");
    } else if (strcmp(op, VALUE_RIGHTSHIFT) == 0) {
        value = irBuilder->CreateAShr(leftValue, rightValue, "rshifttmp");
    } else if (strcmp(op, VALUE_LEFTSHIFT) == 0) {
        value = irBuilder->CreateShl(leftValue, rightValue, "lshifttmp");
    } else {
        throw runtime_error("Invalid binary operation.\n");
    }

    return value;
}

// Perform the operation on the provided value.
Value* computeUnaryExpression(char* op, Value* value) {
    Value* returnValue;

    if (strcmp(op, VALUE_NOT) == 0) {
        returnValue = irBuilder->CreateNot(value, "nottmp");
    } else if (strcmp(op, VALUE_NEGATE) == 0) {
        returnValue = irBuilder->CreateSub(irBuilder->getInt32(0), value, "negtmp");
    } else {
        throw runtime_error("Invalid unary operation\n");
    }

    return returnValue;
}
