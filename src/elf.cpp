#include <iomanip>
#include <optional>
#include <string>

#include <elfio/elfio.hpp>

#include "elf.hpp"

std::optional<std::vector<X86Function>>
getFunctionLocations(std::string fname) {
  ELFIO::elfio reader;

  if (!reader.load(fname)) {
    return std::nullopt;
  }

  std::vector<X86FunctionMetadata> metadata;
  std::vector<X86Function> functions;
  auto sections = reader.sections;

  for (int i = 0; i < sections.size(); ++i) {
    ELFIO::section *psec = sections[i];

    // Find symbol table (always before code)
    if (psec->get_type() == ELFIO::SHT_SYMTAB) {
      const ELFIO::symbol_section_accessor symbols(reader, psec);

      for (unsigned int j = 0; j < symbols.get_symbols_num(); ++j) {
        std::string name;
        ELFIO::Elf64_Addr value;
        ELFIO::Elf_Xword size;
        unsigned char bind;
        unsigned char type;
        ELFIO::Elf_Half section_index;
        unsigned char other;

        symbols.get_symbol(j, name, value, size, bind, type, section_index,
                           other);

        if (type == ELFIO::STT_FUNC && size != 0) {
          std::cout << "Size of symbol at " << std::hex << value << ": " << size
                    << std::dec << std::endl;
          X86FunctionMetadata f_metadata(value, size);
          metadata.push_back(f_metadata);
        }
      }
    }
  }

  for (int i = 0; i < sections.size(); i++) {
    ELFIO::section *psec = sections[i];

    if (psec->get_name() == ".text") {
      ELFIO::Elf64_Addr offset = psec->get_address();
      const char *text = psec->get_data();

      for (auto f_metadata : metadata) {
        char *bytes = (char *)malloc(sizeof(char) * f_metadata.second);
        memcpy(bytes, text + f_metadata.first - offset, f_metadata.second);

        std::cout << "Bytes: ";
        for (int i = 0; i < f_metadata.second; i++) {
          std::cout << std::hex << std::setfill('0') << std::setw(2) << (int) (bytes[i] & 0xff)
                    << std::dec << " ";
        }
        std::cout << std::endl;

        X86Function function = {f_metadata.first, f_metadata.second, bytes};
        functions.push_back(function);
      }
    }
  }

  return std::optional{functions};
}
