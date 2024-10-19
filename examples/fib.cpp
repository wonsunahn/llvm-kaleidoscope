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
  llvm::Value *cond = Builder.CreateICmpSLT(arg, three, "cmptmp");
  llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(TheContext, "then", funcFib);
  llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(TheContext, "else", funcFib);
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(TheContext, "ifcont", funcFib);
  Builder.CreateCondBr(cond, ThenBB, ElseBB);


  // Populate the then branch
  Builder.SetInsertPoint(ThenBB);
  // Insert ret = 1;
  llvm::Value* retVal = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1));
  Builder.CreateStore(retVal, retVar);
  Builder.CreateBr(MergeBB);

  // Populate the else branch
  Builder.SetInsertPoint(ElseBB);
  // ret = fib(x-1) + fib(x-2); on the else branch
  // Call fib(x-1);
  std::vector<llvm::Value *> firstFibParams;
  llvm::Value *argMinusOne = Builder.CreateSub(arg, llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1)), "subtmp");
  firstFibParams.push_back(argMinusOne);
  llvm::Value *firstFib = Builder.CreateCall(funcFib, firstFibParams, "firstFib");
  // Call fib(x-2);
  std::vector<llvm::Value *> secondFibParams;
  llvm::Value *argMinusTwo = Builder.CreateSub(arg, llvm::ConstantInt::get(TheContext, llvm::APInt(32, 2)), "subtmp");
  secondFibParams.push_back(argMinusTwo);
  llvm::Value *secondFib = Builder.CreateCall(funcFib, secondFibParams, "secondFib");
  // Add the two fibs
  retVal = Builder.CreateAdd(firstFib, secondFib, "addtmp");
  // Store to ret
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

  // Define int i = 1;
  llvm::AllocaInst *iVar = CreateEntryBlockAlloca(func, "i");
  Builder.CreateStore(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1)), iVar);

  // Create loop basic blocks and jump to loop condition evaluation basic block
  llvm::BasicBlock *loopCondBB = llvm::BasicBlock::Create(TheContext, "loopcond", func);
  llvm::BasicBlock *loopBodyBB = llvm::BasicBlock::Create(TheContext, "loopbody", func);
  llvm::BasicBlock *loopEndBB = llvm::BasicBlock::Create(TheContext, "loopend", func);
  Builder.CreateBr(loopCondBB);

  // Populate loop condition
  Builder.SetInsertPoint(loopCondBB);
  // Insert while (i < 10)
  llvm::Value *ten = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 10));
  llvm::Value *iVal = Builder.CreateLoad(iVar->getAllocatedType(), iVar, "i");
  llvm::Value *cond = Builder.CreateICmpSLT(iVal, ten, "cmptmp");
  Builder.CreateCondBr(cond, loopBodyBB, loopEndBB);

  // Populate loop body
  Builder.SetInsertPoint(loopBodyBB);
  // Insert i++;
  iVal = Builder.CreateLoad(iVar->getAllocatedType(), iVar, "i");
  llvm::Value *iPlusOne = Builder.CreateAdd(iVal, llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1)), "addtmp");
  Builder.CreateStore(iPlusOne, iVar);
  // Call fib(i);
  std::vector<llvm::Value *> fibCallParams;
  iVal = Builder.CreateLoad(iVar->getAllocatedType(), iVar, "i");
  fibCallParams.push_back(iVal);
  llvm::Value *fib = Builder.CreateCall(funcFib, fibCallParams, "fib");
  // Insert printf("%d\n", fib(i));
  llvm::Value *str = Builder.CreateGlobalStringPtr("%d\n");
  std::vector<llvm::Value *> printfCallParams;
  printfCallParams.push_back(str);
  printfCallParams.push_back(fib);
  Builder.CreateCall(funcPrintf, printfCallParams, "");
  // Loop back to condition
  Builder.CreateBr(loopCondBB);

  // Populate basic block after the loop
  Builder.SetInsertPoint(loopEndBB);
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
