#include <inttypes.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>

#include <Zydis/Zydis.h>

/// @brief Extract the bytes from an executable. Currently only supports
/// ELF binaries.
/// @param fname the name of the file to be read
/// @return the file's raw bytes
std::vector<ZyanU8> get_zydis_bytes(std::string fname) {
  std::ifstream input(fname);

  std::vector<char> bytes((std::istreambuf_iterator<char>(input)),
                          (std::istreambuf_iterator<char>()));
  int length = bytes.size();

  std::vector<ZyanU8> zydis_bytes(length);

  for (int i = 0; i < length; i++) {
    zydis_bytes[i] = (ZyanU8)bytes[i];
  }

  return zydis_bytes;
}

/// @brief Fetches all instructions from an ELF file, represented
/// as a buffer of bytes.
/// @param buffer the bytes of an ELF file
/// @return a list of disassembled x86 instructions
std::vector<ZydisDisassembledInstruction> get_zydis_insts(ZyanU8 *buffer) {
  std::vector<ZydisDisassembledInstruction> instructions;
  ZyanU64 runtime_address = 0x1000;
  ZyanUSize offset = 0x1000;

  while (true) {
    ZydisDisassembledInstruction instruction;
    ZyanUSize length = std::min(sizeof(buffer) - offset,
                                (ZyanUSize)ZYDIS_MAX_INSTRUCTION_LENGTH);
    auto status =
        ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, runtime_address,
                              buffer + offset, length, &instruction);

    if (offset > 0x1144 || ZYAN_FAILED(status)) {
      break;
    }

    instructions.push_back(instruction);

    offset += instruction.info.length;
    runtime_address += instruction.info.length;
  }

  return instructions;
}

int main() {
  std::string fname = "../bin/exe/empty";

  std::vector<ZyanU8> zydis_bytes = get_zydis_bytes(fname);
  ZyanU8 *zydis_buffer = new ZyanU8[zydis_bytes.size()];
  std::copy(zydis_bytes.begin(), zydis_bytes.end(), zydis_buffer);

  std::vector<ZydisDisassembledInstruction> instructions =
      get_zydis_insts(zydis_buffer);

  for (auto inst : instructions) {
    ZydisEncoderRequest enc_req;
    ZydisEncoderDecodedInstructionToEncoderRequest(
        &inst.info, inst.operands, inst.info.operand_count_visible, &enc_req);

    uint8_t new_bytes[ZYDIS_MAX_INSTRUCTION_LENGTH];
    ZyanUSize new_instr_length = sizeof(new_bytes);
    ZydisEncoderEncodeInstruction(&enc_req, new_bytes, &new_instr_length);

    std::cout << std::hex << inst.runtime_address << "\t";

    for (int i = 0; i < new_instr_length; i++) {
      std::cout << std::setfill('0') << std::setw(2) << std::uppercase
                << std::hex << (0xFF & new_bytes[i]) << " ";
    }

    std::cout << "\t\t";

    std::cout << inst.text << std::endl;
  }
}
