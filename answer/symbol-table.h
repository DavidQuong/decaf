#ifndef DECAF_SYMTBL_H
#define DECAF_SYMTBL_H

#include "llvm/IR/Value.h"
#include <cstdio>
#include <map>
#include <string>
#include <stdlib.h>
#include <vector>

using namespace llvm;
using namespace std;

void pushSymbolTable(); 
void popSymbolTable();
void insertSymbol(char* id, Value* value);
void insertExternSymbol(char* id, Value* value);
void insertFunctionSymbol(char* id, Function* function);
Value* getValue(char* id);
void debug();

#endif
