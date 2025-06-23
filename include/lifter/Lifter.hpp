#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <remill/Arch/Arch.h>
#include <remill/BC/ABI.h>
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

    void LiftDirectCall(std::string name, remill::IntrinsicTable &intrinsics);

    void CallPlaceholder();

    std::array<llvm::Value *, remill::kNumBlockArgs> GetArgs();

    void LiftReturn(llvm::Value *dest, remill::IntrinsicTable &intrinsics);

  private:
    // Set on initialisation
    llvm::Module &module;
    llvm::LLVMContext &context;
    const remill::Arch *arch;

    std::unordered_map<uint64_t, std::string> names;

    // Current func/basic block/IR builder
    llvm::Function *func;
    llvm::BasicBlock *block;
};
