// kaleidoscope headers
#include "kaleidoscope/kaleidoscope.h"

// LLVM headers
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

// stdlib headers
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace llvm;

int main()
{
  TheModule = std::make_unique<Module>("helloworld.ll", TheContext);

  // Add printf declaration.
  llvm::Function *funcPrintf = TheModule->getFunction("printf");
  if (!funcPrintf)
  {
    FunctionType *funcPrintfType = FunctionType::get(IntegerType::get(TheContext, 32), true);
    funcPrintf = llvm::Function::Create(funcPrintfType, llvm::Function::ExternalLinkage, "printf", TheModule.get());
    funcPrintf->setCallingConv(CallingConv::C);
  }

  // Create function prototype.
  llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), false);
  llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", TheModule.get());

  // Create a basic block to start populating the function body.
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(TheContext, "entry", func);
  Builder.SetInsertPoint(bb);

  // Insert printf("Hello world\n");
  Value *str = Builder.CreateGlobalStringPtr("Hello world\n");
  std::vector<Value *> callParams;
  callParams.push_back(str);
  CallInst *callInst = CallInst::Create(funcPrintf, callParams, "call", bb);

  // Insert return 0;
  llvm::Value* retVal = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
  Builder.CreateRet(retVal);

  // Validate the generated function, checking for consistency.
  verifyFunction(*func);

  TheModule->print(outs(), nullptr);

  return 0;
}
