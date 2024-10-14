#include "ast/BinaryExprAST.h"
#include "ast/VariableExprAST.h"
#include "kaleidoscope/kaleidoscope.h"

// Generate LLVM code for binary expressions
llvm::Value *BinaryExprAST::codegen() {
  // Special case '=' because we don't want to emit the LHS as an expression.
  if (Op == '=')
  {
    // This assume we're building without RTTI because LLVM builds that way by
    // default. If you build LLVM with RTTI this can be changed to a
    // dynamic_cast for automatic error checking.
    VariableExprAST *LHSE = static_cast<VariableExprAST *>(LHS.get());
    if (!LHSE)
      return LogErrorV("destination of '=' must be a variable");

    // Codegen the RHS.
    llvm::Value *Val = RHS->codegen();
    if (!Val)
      return nullptr;

    // Look up the name.
    llvm::Value *Variable = NamedValues[LHSE->getName()];
    if (!Variable)
      return LogErrorV("Unknown variable name");

    Builder.CreateStore(Val, Variable);
    return Val;
  }

  llvm::Value *L = LHS->codegen();
  llvm::Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return Builder.CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder.CreateFSub(L, R, "subtmp");
  case '*':
    return Builder.CreateFMul(L, R, "multmp");
  case '<':
    L = Builder.CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder.CreateUIToFP(L, llvm::Type::getDoubleTy(TheContext), "booltmp");
  default:
    break;
  }

  // If it wasn't a builtin binary operator, it must be a user defined one. Emit
  // a call to it.
  llvm::Function *F = getFunction(std::string("binary") + Op);
  assert(F && "binary operator not found!");

  llvm::Value *Ops[2] = { L, R };
  return Builder.CreateCall(F, Ops, "binop");
}