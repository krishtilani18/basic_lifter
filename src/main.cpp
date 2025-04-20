#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <remill/Arch/Name.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include <disass/Disassembler.hpp>
#include <elf.hpp>
#include <lifter/lifter.hpp>

// Right now, our lifter extracts code sections from x86 binaries
// and converts them into chunks of instructions, which can (in future)
// then be placed into LLVM functions.
int main(int argc, char *argv[]) {
  /// === EXTRACT FUNCTION LOCATIONS ===
  auto funcsOptional = getFunctionLocations(argv[1]);

  if (!funcsOptional.has_value()) {
    return 0;
  }

  auto funcs = funcsOptional.value();

  /// === LIFT BYTES INTO X86 INSTRUCTIONS ===
  llvm::LLVMContext context;
  auto os_name = remill::GetOSName(REMILL_OS);
  auto arch_name = remill::GetArchName(REMILL_ARCH);

  // Creating the arch object as a unique_ptr
  auto arch = remill::Arch::Get(context, os_name, arch_name);

  // Necessary line - otherwise module gets dropped
  auto module = remill::LoadArchSemantics(arch.get());

  auto disass = NaiveDisassembler(arch.get());

  for (X86Function func : funcs) {
    std::cout << "Lifted result for function at " << std::hex << func.address
              << std::dec << ":" << std::endl;

    auto instructions = disass.Disassemble(func);

    for (remill::Instruction instruction : instructions) {
      std::cout << instruction.Serialize() << std::endl;
    }

    /// === LIFT X86 INSTRUCTIONS INTO LLVM INSTRUCTIONS ===
    // Pass unique_ptr using std::move
    std::cout << "To LLVM: " << std::endl;
    LiftInstructionsToLLVM(*module, context, arch.get(), instructions);
  }
}
