#include "ast/FunctionAST.h"

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                          const std::string &VarName) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(llvm::Type::getDoubleTy(TheContext), nullptr, VarName.c_str());
}

// Generates LLVM code for functions declarations
llvm::Function *FunctionAST::codegen() {
  llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction) {
    TheFunction = Proto->codegen();
  }

  if (!TheFunction) {
    return nullptr;
  }

  llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);
  NamedValues.clear();
  for (auto &Arg : TheFunction->args()) {
    // Create an alloca for this variable.
    llvm::AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

    // Store the initial value into the alloca.
    Builder.CreateStore(&Arg, Alloca);

    // Add arguments to variable symbol table.
    NamedValues[std::string(Arg.getName())] = Alloca;
  }

  if (llvm::Value *RetVal = Body->codegen()) {
    Builder.CreateRet(RetVal);
    verifyFunction(*TheFunction);

    return TheFunction;
  }

  TheFunction->eraseFromParent();
  return nullptr;
}
