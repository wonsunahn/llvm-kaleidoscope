#include "ast/UnaryExprAST.h"
#include "kaleidoscope/kaleidoscope.h"

llvm::Value *UnaryExprAST::codegen() {
  llvm::Value *OperandV = Operand->codegen();
  if (!OperandV)
    return nullptr;

  llvm::Function *F = getFunction(std::string("unary") + Opcode);
  if (!F)
    return LogErrorV("Unknown unary operator");

  return Builder.CreateCall(F, OperandV, "unop");
}