#ifndef __VAR_EXPR_AST_H__
#define __VAR_EXPR_AST_H__

#include "ast/ExprAST.h"
#include "llvm/IR/IRBuilder.h"

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
  std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
  std::unique_ptr<ExprAST> Body;

public:
  VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames,
             std::unique_ptr<ExprAST> Body)
    : VarNames(std::move(VarNames)), Body(std::move(Body)) {}

  llvm::Value *codegen() override;
};

#endif