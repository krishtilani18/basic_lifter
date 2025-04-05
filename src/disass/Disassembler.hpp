#pragma once

#include <remill/Arch/Arch.h>
#include <remill/Arch/Instruction.h>

#include "../elf.hpp"

class Disassembler {
public:
  virtual std::vector<remill::Instruction> Disassemble(X86Function) = 0;
};

class NaiveDisassembler : Disassembler {
public:
  NaiveDisassembler();
  std::vector<remill::Instruction> Disassemble(X86Function) override;
private:
  remill::Arch::ArchPtr arch;
};
