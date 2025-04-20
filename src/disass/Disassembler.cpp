#include <remill/Arch/Name.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include <disass/Disassembler.hpp>

std::vector<remill::Instruction>
NaiveDisassembler::Disassemble(X86Function function) {
  std::vector<remill::Instruction> instructions;
  uint64_t offset = 0;

  while (offset < function.size) {
    remill::Instruction instruction;

    uint64_t max_size;
    if (function.size - offset < MAX_INSTRUCTION_LEN) {
      max_size = function.size - offset;
    } else {
      max_size = MAX_INSTRUCTION_LEN;
    }

    std::string_view bytes(function.bytes + offset, max_size);

    bool decoded = this->arch->DecodeInstruction(function.address + offset,
                                                 bytes, instruction,
                                                 this->arch->CreateInitialContext());

    if (decoded) {
      instructions.push_back(instruction);
      offset += instruction.NumBytes();
    } else {
      std::cout << "Instruction with category " << instruction.category
                << " found" << std::endl;
      break;
    }
  }

  return instructions;
}
