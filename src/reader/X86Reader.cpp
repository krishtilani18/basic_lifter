#include <unordered_map>

#include <reader/X86Reader.hpp>

std::vector<X86Procedure> X86Reader::GetProcedures() {
    std::vector<X86ProcedureMetadata> metadata;
    std::vector<X86Procedure> procedures;

    auto sections = this->reader.sections;

    // Find all procedure names
    for (unsigned int i = 0; i < sections.size(); i++) {
        ELFIO::section *psec = sections[i];

        // Find symbol table
        if (psec->get_type() != ELFIO::SHT_SYMTAB) {
            continue;
        }

        const ELFIO::symbol_section_accessor symbols(this->reader, psec);

        for (unsigned int j = 0; j < symbols.get_symbols_num(); j++) {
            std::string name;
            ELFIO::Elf64_Addr value;
            ELFIO::Elf_Xword size;
            unsigned char bind;
            unsigned char type;
            ELFIO::Elf_Half section_index;
            unsigned char other;

            symbols.get_symbol(j, name, value, size, bind, type, section_index,
                               other);

            // _start is a function called before everything else in C-like
            // environments - since we're compiling back down using Clang to
            // test, we can safely get rid of _start
            if (type == ELFIO::STT_FUNC && size != 0) {
                X86ProcedureMetadata f_metadata{name, value, size};
                metadata.push_back(f_metadata);
            }
        }

        // There is only one symbol table per binary
        break;
    }

    // Convert them to individual bytes
    for (unsigned int i = 0; i < sections.size(); i++) {
        ELFIO::section *psec = sections[i];

        if (psec->get_name() == ".text") {
            ELFIO::Elf64_Addr offset = psec->get_address();
            const char *text = psec->get_data();

            for (auto p_metadata : metadata) {
                char *bytes = (char *)malloc(sizeof(char) * p_metadata.size);
                memcpy(bytes, text + p_metadata.address - offset,
                       p_metadata.size);

                X86Procedure proc = {p_metadata.name, p_metadata.address,
                                     p_metadata.size, bytes};
                procedures.push_back(proc);
            }
        }
    }

    return procedures;
}

std::vector<X86ExternalProcedure> X86Reader::GetExternalProcedures() {
    std::unordered_map<ELFIO::Elf64_Addr, std::string> jumps;
    std::vector<X86ExternalProcedure> procedures;

    auto sections = this->reader.sections;

    // Find all jumps to dynamically-linked libraries
    for (unsigned int i = 0; i < sections.size(); i++) {
        ELFIO::section *psec = sections[i];

        if (psec->get_type() != ELFIO::SHT_RELA) {
            continue;
        }

        const ELFIO::relocation_section_accessor symbols(this->reader, psec);

        for (unsigned int j = 0; j < symbols.get_entries_num(); j++) {
            ELFIO::Elf64_Addr offset;
            ELFIO::Elf64_Addr symbolValue;
            std::string symbolName;
            unsigned type;
            ELFIO::Elf_Sxword addend;
            ELFIO::Elf_Sxword calcValue;

            symbols.get_entry(j, offset, symbolValue, symbolName, type, addend,
                              calcValue);

            if (type == ELFIO::R_X86_64_GLOB_DAT ||
                type == ELFIO::R_X86_64_JUMP_SLOT) {
                jumps[offset + addend] = symbolName;
            }
        }
    }

    // Convert jumps to references to external procedures
    for (unsigned int i = 0; i < sections.size(); i++) {
        ELFIO::section *psec = sections[i];
        std::string psecName = psec->get_name();

        // NOTE: `gcc` flavour of compilation is used - for finding
        // where @plt functions are called in Clang, go to `.plt` instead
        if (!(psecName == ".plt.sec" || psecName == ".plt.got")) {
            continue;
        }

        // Each external call is 0x10 bytes long, with three function calls
        // (in GCC):
        // endbr64 (4 bytes)
        // jmp     (6 bytes) <- the info we need
        // nopw    (6 bytes)
        unsigned int numExternals = psec->get_size() / 0x10;

        for (unsigned int j = 0; j < numExternals; j++) {
            unsigned int callsite = psec->get_address() + (j * 0x10);
            unsigned int jumpAddress = callsite + 0xa;

            // Offset address is stored on bytes 6 and 7, little-endian
            uint8_t smallByte = (uint8_t)psec->get_data()[j * 0x10 + 0x6];
            uint8_t largeByte = (uint8_t)psec->get_data()[j * 0x10 + 0x7];
            unsigned int offset = (largeByte << 8) + smallByte;

            auto found = jumps.find(jumpAddress + offset);

            if (found == jumps.end()) {
                continue;
            }

            X86ExternalProcedure externalProc{(*found).second, callsite};
            procedures.push_back(externalProc);
        }
    }

    return procedures;
}
