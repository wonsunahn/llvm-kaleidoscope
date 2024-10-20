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
llvm::GlobalVariable *xVar = nullptr;
llvm::GlobalVariable *yVar = nullptr;
llvm::GlobalVariable *pointVar = nullptr;

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

void generateGlobals()
{
  // Add int x, y global variable declarations.
  TheModule->getOrInsertGlobal("x", llvm::Type::getInt32Ty(TheContext));
  xVar = TheModule->getNamedGlobal("x");
  TheModule->getOrInsertGlobal("y", llvm::Type::getInt32Ty(TheContext));
  yVar = TheModule->getNamedGlobal("y");
  // Add point global variable declarations.
  llvm::StructType *pointType = llvm::StructType::create(TheContext, { llvm::Type::getInt32Ty(TheContext), llvm::Type::getInt32Ty(TheContext) }, "Point");
  TheModule->getOrInsertGlobal("point", pointType);
  pointVar = TheModule->getNamedGlobal("point");

  // All LLVM global variables must be initialized:
  // https://llvm.org/docs/LangRef.html#global-variables
  
  // int x = 10, y = 20;
  xVar->setInitializer(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 10)));
  yVar->setInitializer(llvm::ConstantInt::get(TheContext, llvm::APInt(32, 20)));
  pointVar->setInitializer(llvm::ConstantStruct::get(pointType, { llvm::ConstantInt::get(TheContext, llvm::APInt(32, 1)), llvm::ConstantInt::get(TheContext, llvm::APInt(32, 2)) }));
}

void generatePrintStruct()
{
  // Insert printf("point = (%d, %d)\n", point.x, point.y);
  // Read point struct x, y fields
  llvm::Value *firstStructMemberPtr = Builder.CreateStructGEP(pointVar, 0, "point.x.ptr");
  llvm::Value *firstStructMember = Builder.CreateLoad(llvm::Type::getInt32Ty(TheContext), firstStructMemberPtr, "point.x");
  llvm::Value *secondStructMemberPtr = Builder.CreateStructGEP(pointVar, 1, "point.y.ptr");
  llvm::Value *secondStructMember = Builder.CreateLoad(llvm::Type::getInt32Ty(TheContext), secondStructMemberPtr, "point.y");
  // Call printf using above arguments
  llvm::Value *str = Builder.CreateGlobalStringPtr("point = (%d, %d)\n", "format");
  std::vector<llvm::Value *> printfCallParams;
  printfCallParams.push_back(str);
  printfCallParams.push_back(firstStructMember);
  printfCallParams.push_back(secondStructMember);
  Builder.CreateCall(funcPrintf, printfCallParams, "");
}

void generateMain()
{
  // Create main function prototype.
  llvm::FunctionType *funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(TheContext), false);
  llvm::Function *func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", TheModule.get());

  // Create a basic block to start populating the function body.
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(TheContext, "entry", func);
  Builder.SetInsertPoint(bb);

  // Print point struct before updating fields.
  generatePrintStruct();

  // Insert point.x = x; point.y = y;
  llvm::Value *xVal = Builder.CreateLoad(xVar->getValueType(), xVar, "x");
  llvm::Value *yVal = Builder.CreateLoad(xVar->getValueType(), yVar, "y");
  llvm::Value *firstStructMemberPtr = Builder.CreateStructGEP(pointVar, 0, "point.x.ptr");
  llvm::Value *secondStructMemberPtr = Builder.CreateStructGEP(pointVar, 1, "point.y.ptr");
  Builder.CreateStore(xVal, firstStructMemberPtr);
  Builder.CreateStore(yVal, secondStructMemberPtr);

  // Print point struct after updating fields.
  generatePrintStruct();

  // Insert return 0;
  llvm::Value *retVal = llvm::ConstantInt::get(TheContext, llvm::APInt(32, 0));
  Builder.CreateRet(retVal);

  // Validate the generated function, checking for consistency.
  verifyFunction(*func);
}

int main()
{
  TheModule = std::make_unique<llvm::Module>("point.codegen.ll", TheContext);

  generateGlobals();
  generatePrintf();
  generateMain();

  TheModule->print(llvm::outs(), nullptr);

  return 0;
}
