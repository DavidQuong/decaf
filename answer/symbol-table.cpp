#include "llvm/IR/Value.h"
#include <cstdio>
#include <map>
#include <string>
#include <stdlib.h>
#include <vector>

using namespace llvm;
using namespace std;

static const int INDEX_EXTERN = 0;
static const int INDEX_CLASS = 1;

typedef map<string, Value*> SymbolTable;
typedef vector<SymbolTable*> SymbolTableVector;

static SymbolTableVector symbolTableStack;
static SymbolTableVector symbolTablePool;

void pushSymbolTable() {
    /* symbolTableStack.push_back(new SymbolTable); */
    SymbolTable* test = new SymbolTable;
    symbolTableStack.push_back(test);
    symbolTablePool.push_back(test);
}

void popSymbolTable() {
    symbolTableStack.pop_back();
}

void insertSymbol(char* id, Value* value) {
    if (symbolTableStack.empty()) {
        fprintf(stderr, "symbolTableStack is empty.\n");
        exit(EXIT_FAILURE);
    }

    SymbolTable* symbolTable = symbolTableStack.back();
    string* symbolId = new string(id);  
   
    (*symbolTable)[*symbolId] = value;
}

void insertExternSymbol(char* id, Value* value) {
    if (symbolTableStack.empty()) {
        fprintf(stderr, "symbolTableStack is empty.\n");
        exit(EXIT_FAILURE);
    }

    SymbolTable* symbolTable = symbolTableStack.at(INDEX_EXTERN);
    string* symbolId = new string(id);  
   
    (*symbolTable)[*symbolId] = value;
}

void insertFunctionSymbol(char* id, Function* function) {
    if (symbolTableStack.empty()) {
        fprintf(stderr, "symbolTableStack is empty.\n");
        exit(EXIT_FAILURE);
    }

    SymbolTable* symbolTable = symbolTableStack.at(INDEX_CLASS);
    string* symbolId = new string(id);  
   
    (*symbolTable)[*symbolId] = (Value*) function;
}

Value* getValue(char* id) {
    if (symbolTableStack.empty()) {
        fprintf(stderr, "symbolTableStack is empty.\n");
        exit(EXIT_FAILURE);
    }

    string symbolId(id);
    for (SymbolTableVector::reverse_iterator tableIt = symbolTableStack.rbegin(); tableIt != symbolTableStack.rend(); tableIt++) {
        SymbolTable* table = *(tableIt);
        
        SymbolTable::iterator resultIt = table->find(symbolId);
        // If not equal to the end, then the symbol table contains the symbol.
        if (resultIt != table->end()) {
            return (resultIt->second);
        }
    }

    return NULL;
}

void debug() {
    printf("====================================================================================\n");
    printf("DEBUG\n");
    
    int i = 1;
    for (SymbolTableVector::iterator tableIt = symbolTablePool.begin(); tableIt != symbolTablePool.end(); tableIt++) {
        printf("Symbol Table #%d\n", i);

        SymbolTable* table = *(tableIt);
        for (SymbolTable::iterator symbolIt = table->begin(); symbolIt != table->end(); symbolIt++) {
            printf("\tKey (ID) = %s\n", symbolIt->first.c_str());
        }

        i++;
    }
}
