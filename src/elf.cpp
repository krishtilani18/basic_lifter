#include <iomanip>
#include <optional>
#include <string>

#include <elfio/elfio.hpp>

#include <elf.hpp>

std::optional<std::vector<X86Procedure>> getProcedures(std::string fname) {
    ELFIO::elfio reader;

    if (!reader.load(fname)) {
        return std::nullopt;
    }

    std::vector<X86ProcedureMetadata> metadata;
    std::vector<X86Procedure> procs;
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

                symbols.get_symbol(j, name, value, size, bind, type,
                                   section_index, other);

                if (type == ELFIO::STT_FUNC && size != 0) {
                    X86ProcedureMetadata f_metadata = {name, value, size};
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
                char *bytes = (char *)malloc(sizeof(char) * f_metadata.size);
                memcpy(bytes, text + f_metadata.address - offset,
                       f_metadata.size);
                
                /*
                std::cout << "Proc bytes at address " << f_metadata.address << ":" << std::endl;

                std::cout << std::hex;
                for (uint64_t i = 0; i < f_metadata.size; i++) {
                    std::cout << std::setw(2) << std::setfill('0') << (int) (0xff & bytes[i]);
                }
                std::cout << std::dec << std::endl;
                */

                X86Procedure proc = {f_metadata.name, f_metadata.address,
                                     f_metadata.size, bytes};
                procs.push_back(proc);
            }
        }
    }

    return std::optional{procs};
}
