#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <remill/Arch/Arch.h>
#include <vector>

class Lifter {
public:
    Lifter(llvm::Module &module, llvm::LLVMContext &context,
           const remill::Arch *arch)
        : module(module), context(context), arch(arch) {}
    llvm::Function *Lift(std::string name,
                         std::vector<remill::Instruction> instructions);

private:
    llvm::Module &module;
    llvm::LLVMContext &context;
    const remill::Arch *arch;
};
