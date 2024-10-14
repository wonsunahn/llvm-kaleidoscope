#include "ast/VariableExprAST.h"
#include "kaleidoscope/kaleidoscope.h"

// We assume that the variable has already been emitted somewhere
llvm::Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  llvm::AllocaInst *A = NamedValues[Name];
  if (!A)
    return LogErrorV("Unknown variable name");

  // Load the value.
  return Builder.CreateLoad(A->getAllocatedType(), A, Name.c_str());
}