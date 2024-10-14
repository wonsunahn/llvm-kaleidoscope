#ifndef __UNARY_EXPR_AST_H__
#define __UNARY_EXPR_AST_H__

#include "ast/ExprAST.h"
#include "llvm/IR/IRBuilder.h"
#include "logger/logger.h"

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
  char Opcode;
  std::unique_ptr<ExprAST> Operand;

public:
  UnaryExprAST(char Opcode, std::unique_ptr<ExprAST> Operand)
    : Opcode(Opcode), Operand(std::move(Operand)) {}

  llvm::Value *codegen() override;
};

#endif