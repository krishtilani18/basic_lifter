#include <inttypes.h>
#include <iomanip>
#include <iostream>

#include "./disass/Disassembler.hpp"
#include "./elf.hpp"

int main(int argc, char *argv[]) {
  NaiveDisassembler disass;

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
    }
  }
}
