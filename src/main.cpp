#include <Zydis/Zydis.h>
#include <inttypes.h>
#include <stdio.h>

int main() {
  ZyanU8 data[] = {0x51, 0x8D, 0x45, 0xFF, 0x50, 0xFF, 0x75, 0x0C, 0xFF,
                   0x75, 0x08, 0xFF, 0x15, 0xA0, 0xA5, 0x48, 0x76, 0x85,
                   0xC0, 0x0F, 0x88, 0xFC, 0xDA, 0x02, 0x00};

  // The runtime address (instruction pointer) was chosen arbitrarily here in
  // order to better visualize relative addressing. In your actual program, set
  // this to e.g. the memory address that the code being disassembled was read
  // from.
  ZyanU64 runtime_address = 0x007FFFFFFF400000;

  // Loop over the instructions in our buffer.
  ZyanUSize offset = 0;
  ZydisDisassembledInstruction instruction;
  while (ZYAN_SUCCESS(ZydisDisassembleIntel(
      /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
      /* runtime_address: */ runtime_address,
      /* buffer:          */ data + offset,
      /* length:          */ sizeof(data) - offset,
      /* instruction:     */ &instruction))) {
    printf("%016" PRIX64 "  %s\n", runtime_address, instruction.text);
    offset += instruction.info.length;
    runtime_address += instruction.info.length;
  }

  return 0;
}
