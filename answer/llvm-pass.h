#ifndef DECAF_PASS
#define DECAF_PASS

#include "llvm/IR/Module.h"
#include "llvm/PassManager.h"

using namespace llvm;

void initializePassManagers(Module* module);
FunctionPassManager* getFunctionPassManager();

#endif
