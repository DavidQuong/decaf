#include "llvm/Analysis/Passes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm-pass.h"
#include "llvm-util.h"

using namespace llvm;

static FunctionPassManager* functionPassManager;

void initializePassManagers(Module* module) {
    char id = 0;
    functionPassManager = new FunctionPassManager(module);
    
    // Adds GVN support
    functionPassManager->add(createBasicAliasAnalysisPass());

    // Convert stack allocation usage (alloca) into register usage (mem2reg)
    functionPassManager->add(createPromoteMemoryToRegisterPass());
    
    // Simple "peephole" optimization (instruction combining pass)
    functionPassManager->add(createInstructionCombiningPass());
    
    // Re-associate expression
    functionPassManager->add(createReassociatePass());

    // Eliminate common sub-expressions (GVN)
    functionPassManager->add(createGVNPass());
    
    // Simplify the control flow graph (CFG simplification)
    functionPassManager->add(createCFGSimplificationPass());
}

FunctionPassManager* getFunctionPassManager() {
    return functionPassManager;
}
