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

std::vector<X86Global> X86Reader::GetGlobals() {
    std::vector<X86Global> globals;

    unsigned int num_sections = this->reader.sections.size();
    for (unsigned int si = 0; si < num_sections; ++si) {
        ELFIO::section* psec = this->reader.sections[si];
        if (!psec) continue;

        const char* sec_data = psec->get_data();
        if (!sec_data) continue;

        ELFIO::Elf_Xword sec_size = psec->get_size();

        std::string sec_name = psec->get_name();
        unsigned flags = psec->get_flags();

        // Only include .data/.bss globals from symbols
        if (psec->get_type() == ELFIO::SHT_SYMTAB) {
            ELFIO::symbol_section_accessor symbols(this->reader, psec);
            for (unsigned i = 0; i < symbols.get_symbols_num(); ++i) {
                std::string name;
                ELFIO::Elf64_Addr value;
                ELFIO::Elf_Xword size;
                unsigned char bind = 0, type = 0, other = 0;
                ELFIO::Elf_Half index = 0;

                symbols.get_symbol(i, name, value, size, bind, type, index, other);

                if (index == ELFIO::SHN_UNDEF || size == 0) continue;
                if (index >= this->reader.sections.size()) continue;

                ELFIO::section* data_sec = this->reader.sections[index];
                if (!data_sec) continue;

                const char* data_ptr = data_sec->get_data();
                if (!data_ptr) continue;

                ELFIO::Elf64_Off offset = static_cast<ELFIO::Elf64_Off>(value - data_sec->get_address());
                if (offset + size > data_sec->get_size()) continue;

                bool is_global_data = (bind == ELFIO::STB_GLOBAL || bind == ELFIO::STB_WEAK) &&
                                      (type == ELFIO::STT_OBJECT || type == ELFIO::STT_NOTYPE);

                if (is_global_data &&
                    (data_sec->get_name() == ".data" || data_sec->get_name() == ".bss")) {

                    X86Global g;
                    g.name = name;
                    g.address = value;
                    g.size = size;
                    g.bytes = data_ptr + offset;
                    g.section = data_sec->get_name();

                    globals.push_back(g);
                }
            }
        }

    if ((flags & ELFIO::SHF_ALLOC) && !(flags & ELFIO::SHF_WRITE) &&
        sec_name == ".rodata") {

        std::vector<char> printable_bytes;
        for (ELFIO::Elf_Xword i = 0; i < sec_size; ++i) {
            unsigned char c = sec_data[i];
            if (isprint(c)) {
                printable_bytes.push_back(c);
            }
        }

            if (!printable_bytes.empty()) {
                char* filtered_data = new char[printable_bytes.size()];
                std::memcpy(filtered_data, printable_bytes.data(), printable_bytes.size());

                X86Global g;
                g.name = "rodata_" + std::to_string(printable_bytes.size()); 
                g.address = psec->get_address();          
                g.size = printable_bytes.size();
                g.bytes = filtered_data;                 
                g.section = sec_name;

                globals.push_back(g);
            }
        }
    }
    return globals;
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
            ELFIO::Elf_Xword index;
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

            // x86 is little-endian
            unsigned int offset =
                (((uint8_t) psec->get_data()[7]) << 8) + ((uint8_t) psec->get_data()[6]);

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
