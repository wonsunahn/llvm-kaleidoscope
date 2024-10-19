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

static llvm::Function *funcPrintf = nullptr;
static llvm::Function *funcFib = nullptr;

/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function *TheFunction,
                                          const std::string &VarName) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                 TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(llvm::Type::getInt32Ty(TheContext), nullptr, VarName.c_str());
}

static void generatePrintf()
{
  // Add printf declaration.
  funcPrintf = TheModule->getFunction("printf");
  if (!funcPrintf)
  {
    llvm::FunctionType *funcPrintfType = llvm::FunctionType::get(llvm::IntegerType::get(TheContext, 32), true);
    funcPrintf = llvm::Function::Create(funcPrintfType, llvm::Function::ExternalLinkage, "printf", TheModule.get());
    funcPrintf->setCallingConv(llvm::CallingConv::C);
  }
}

static void generateFib()
{
  // Create main function prototype.
  std::vector<llvm::Type *> oneInt(1, llvm::Type::getInt32Ty(TheContext));
  llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), oneInt, false);
  funcFib = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "fib", TheModule.get());

  // Create a basic block to start populating the function body.
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(TheContext, "entry", funcFib);
  Builder.SetInsertPoint(bb);
  // Define int ret;
  llvm::AllocaInst *retVar = CreateEntryBlockAlloca(funcFib, "ret");

  // Insert if (x < 3)
  llvm::Value *three = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 3));
  llvm::Value *arg = funcFib->args().begin();
  llvm::Value *cond = Builder.CreateICmpULT(arg, three, "cmptmp");
  llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", funcFib);
  llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else", funcFib);
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "ifcont", funcFib);
  Builder.CreateCondBr(cond, ThenBB, ElseBB);


  // Insert ret = 1; on the then branch
  Builder.SetInsertPoint(ThenBB);
  llvm::Value* retVal = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1));
  Builder.CreateStore(retVal, retVar);
  Builder.CreateBr(MergeBB);

  // Insert ret = fib(x-1) + fib(x-2); on the else branch
  Builder.SetInsertPoint(ElseBB);
  // Call fib(x-1);
  std::vector<llvm::Value *> firstFibParams;
  llvm::Value *argMinusOne = Builder.CreateSub(arg, llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1)), "subtmp");
  firstFibParams.push_back(argMinusOne);
  llvm::Value *firstFib = llvm::CallInst::Create(funcFib, firstFibParams, "firstFib", ElseBB);
  // Call fib(x-2);
  std::vector<llvm::Value *> secondFibParams;
  llvm::Value *argMinusTwo = Builder.CreateSub(arg, llvm::ConstantInt::get(TheContext, llvm::APInt(32, 2)), "subtmp");
  secondFibParams.push_back(argMinusTwo);
  llvm::Value *secondFib = llvm::CallInst::Create(funcFib, secondFibParams, "secondFib", ElseBB);
  // Add the two fibs
  retVal = Builder.CreateAdd(firstFib, secondFib, "addtmp");
  Builder.CreateStore(retVal, retVar);
  Builder.CreateBr(MergeBB);
  
  // Insert return ret;
  Builder.SetInsertPoint(MergeBB);
  retVal = Builder.CreateLoad(retVar->getAllocatedType(), retVar, "loadtmp");
  Builder.CreateRet(retVal);

  // Validate the generated function, checking for consistency.
  verifyFunction(*funcFib);
}

void generateMain()
{
  // Create main function prototype.
  llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), false);
  llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", TheModule.get());

  // Create a basic block to start populating the function body.
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(TheContext, "entry", func);
  Builder.SetInsertPoint(bb);

  // Insert ret = fib(5);
  std::vector<llvm::Value *> fibCallParams;
  fibCallParams.push_back(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 6)));
  llvm::Value *ret = llvm::CallInst::Create(funcFib, fibCallParams, "ret", bb);
  
  // Insert printf("%d\n", ret);
  llvm::Value *str = Builder.CreateGlobalStringPtr("%d\n");
  std::vector<llvm::Value *> callParams;
  callParams.push_back(str);
  callParams.push_back(ret);
  llvm::CallInst::Create(funcPrintf, callParams, "call", bb);

  // Insert return 0;
  llvm::Value* retVal = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
  Builder.CreateRet(retVal);

  // Validate the generated function, checking for consistency.
  verifyFunction(*func);
}

int main()
{
  TheModule = std::make_unique<llvm::Module>("fib.ll", TheContext);

  generatePrintf();
  generateFib();
  generateMain();

  TheModule->print(llvm::outs(), nullptr);

  return 0;
}
