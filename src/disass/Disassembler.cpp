#include <llvm/IR/LLVMContext.h>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Name.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include "Disassembler.hpp"

NaiveDisassembler::NaiveDisassembler() {
  llvm::LLVMContext context;
  auto os_name = remill::GetOSName(REMILL_OS);
  auto arch_name = remill::GetArchName(REMILL_ARCH);

  this->arch = remill::Arch::Get(context, os_name, arch_name);
  auto module = remill::LoadArchSemantics(this->arch.get());
  std::cout << this->arch->GetInstrinsicTable() << std::endl;
}

std::vector<remill::Instruction> NaiveDisassembler::Disassemble(X86Function function) {
  std::vector<remill::Instruction> instructions;
  uint64_t offset = 0;

  while (offset < function.size) {
    std::cout << "Offset at " << offset << std::endl;

    remill::Instruction instruction;
    this->arch->DecodeInstruction(function.address + offset,
                            function.bytes + offset, instruction,
                            arch->CreateInitialContext());

    instructions.push_back(instruction);
    offset += instruction.NumBytes();
  }

  return instructions;
}
