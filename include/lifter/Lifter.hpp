#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <remill/Arch/Arch.h>
#include <unordered_map>
#include <vector>

class Lifter {
public:
    Lifter(llvm::Module &module, llvm::LLVMContext &context,
           const remill::Arch *arch)
        : module(module), context(context), arch(arch) {}

    void AddFunctionName(uint64_t address, std::string name);

    llvm::Function *Lift(std::string name,
                         std::vector<remill::Instruction> instructions);

    void LiftDirectCall(const remill::Instruction &inst, llvm::Function *func,
                        llvm::IRBuilder<> &builder,
                        remill::IntrinsicTable &intrinsics);

private:
    llvm::Module &module;
    llvm::LLVMContext &context;
    const remill::Arch *arch;

    std::unordered_map<uint64_t, std::string> names;
};
