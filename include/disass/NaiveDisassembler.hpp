#pragma once

#include <llvm/IR/LLVMContext.h>
#include <remill/Arch/Arch.h>

#include "./Disassembler.hpp"

class NaiveDisassembler : Disassembler {
public:
    NaiveDisassembler(const remill::Arch *arch) : arch(arch) {}
    std::vector<remill::Instruction> Disassemble(X86Procedure) override;

private:
    const remill::Arch *arch;
};
