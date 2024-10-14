#include "ast/PrototypeAST.h"
#include "kaleidoscope/kaleidoscope.h"

// Generates LLVM code for externals calls
llvm::Function *PrototypeAST::codegen() {
  std::vector<llvm::Type *> Doubles(Args.size(), llvm::Type::getDoubleTy(TheContext));
  llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext), Doubles, false);
  llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name.c_str(), TheModule.get());

  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(Args[Idx++].c_str());
  }

  return F;
}
