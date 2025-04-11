#ifndef LIFT_INSTRUCTIONS_H
#define LIFT_INSTRUCTIONS_H

#include <remill/Arch/Arch.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <vector>
#include <remill/Arch/Instruction.h>

// Function declaration with proper return type (llvm::Module)
void LiftInstructionsToLLVM(
    llvm::Module &module,
    llvm::LLVMContext &context,
    std::unique_ptr<const remill::Arch> arch,
    const std::vector<remill::Instruction> &instructions);
#endif // LIFT_INSTRUCTIONS_H
