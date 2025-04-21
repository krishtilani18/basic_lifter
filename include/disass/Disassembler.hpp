#pragma once

#include <remill/Arch/Instruction.h>

#include <elf.hpp>

#define MAX_INSTRUCTION_LEN 15

class Disassembler {
public:
  virtual std::vector<remill::Instruction> Disassemble(X86Function) = 0;
};
