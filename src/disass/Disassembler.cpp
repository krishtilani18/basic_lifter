#include <remill/Arch/Name.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include "Disassembler.hpp"

std::vector<remill::Instruction> NaiveDisassembler::Disassemble(X86Function function) {
  std::vector<remill::Instruction> instructions;
  uint64_t offset = 0;

  while (offset < function.size) {
    std::cout << "Offset at " << offset << std::endl;

    remill::Instruction instruction;

    bool decoded = this->arch->DecodeInstruction(function.address + offset,
                            function.bytes + offset, instruction,
                            arch->CreateInitialContext());

    if (decoded) {
      instructions.push_back(instruction);
      offset += instruction.NumBytes();
    } else {
      std::cout << "Instruction with category " << instruction.category << " found" << std::endl;
      break;
    }
  }

  return instructions;
}
