#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <remill/Arch/Name.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include "./disass/Disassembler.hpp"
#include "./elf.hpp"
#include "./lifter/lifter.hpp"

int main(int argc, char *argv[]) {
  llvm::LLVMContext context;
  auto os_name = remill::GetOSName(REMILL_OS);
  auto arch_name = remill::GetArchName(REMILL_ARCH);

  // Creating the arch object as a unique_ptr
  auto arch = remill::Arch::Get(context, os_name, arch_name);

  // Necessary line - otherwise module gets dropped
  auto module = remill::LoadArchSemantics(arch.get());

  auto disass = NaiveDisassembler(arch);

  auto funcsOptional = getFunctionLocations(argv[1]);

  if (funcsOptional.has_value()) {
    auto funcs = funcsOptional.value();

    for (X86Function func : funcs) {
      std::cout << "Lifted result for function at " << std::hex << func.address
                << std::dec << ":" << std::endl;

      auto instructions = disass.Disassemble(func);


      
      for (remill::Instruction instruction : instructions) {
        std::cout << instruction.Serialize() << std::endl;
      }
      // Pass unique_ptr using std::move
      std::cout << "To LLVM: " << std::endl;
      LiftInstructionsToLLVM(*module, context, std::move(arch), instructions);
      
    
    }
  }
}
