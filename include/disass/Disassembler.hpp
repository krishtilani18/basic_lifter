#pragma once

#include <llvm/IR/LLVMContext.h>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Instruction.h>

#include <elf.hpp>

#define MAX_INSTRUCTION_LEN 15

class Disassembler {
public:
  virtual std::vector<remill::Instruction> Disassemble(X86Function) = 0;
};

class NaiveDisassembler : Disassembler {
public:
  NaiveDisassembler(const remill::Arch *arch): arch(arch) {}
  std::vector<remill::Instruction> Disassemble(X86Function) override;
private:
  const remill::Arch *arch;
};
