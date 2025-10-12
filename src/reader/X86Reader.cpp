#include <reader/X86Reader.hpp>

ELFIO::Elf64_Addr X86Reader::GetEntry() {
    return this->reader.get_entry();
}

std::vector<X86Procedure> X86Reader::GetProcedures() {
    std::vector<X86ProcedureMetadata> metadata;
    std::vector<X86Procedure> functions;
    auto sections = this->reader.sections;

    for (int i = 0; i < sections.size(); ++i) {
        ELFIO::section *psec = sections[i];

        // Find symbol table (always before code)
        if (psec->get_type() == ELFIO::SHT_SYMTAB) {
            const ELFIO::symbol_section_accessor symbols(this->reader, psec);

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
                    X86ProcedureMetadata f_metadata{name, value, size};
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

                X86Procedure function = {f_metadata.name, f_metadata.address,
                                        f_metadata.size, bytes};
                functions.push_back(function);
            }
        }
    }

    return functions;
}

std::vector<X86Global> X86Reader::GetGlobals() {
    std::vector<X86Global> globals;

    // Use size/type compatible with ELFIO::Sections
    unsigned int num_sections = this->reader.sections.size();
    for (unsigned int si = 0; si < num_sections; ++si) {
        ELFIO::section* psec = this->reader.sections[si];
        if (!psec) continue;

        // Find symbol table sections
        if (psec->get_type() != ELFIO::SHT_SYMTAB) continue;

        ELFIO::symbol_section_accessor symbols(this->reader, psec);
        for (unsigned i = 0; i < symbols.get_symbols_num(); ++i) {
            std::string name;
            ELFIO::Elf64_Addr value;
            ELFIO::Elf_Xword size;
            unsigned char bind = 0, type = 0, other = 0;
            ELFIO::Elf_Half index = 0;

            symbols.get_symbol(i, name, value, size, bind, type, index, other);

            // Filter for defined global data symbols
            if ((bind == ELFIO::STB_GLOBAL || bind == ELFIO::STB_WEAK) &&
                (type == ELFIO::STT_OBJECT || type == ELFIO::STT_NOTYPE) &&
                index != ELFIO::SHN_UNDEF && size > 0) {

                // `index` is the section index for this symbol; obtain that section
                if (index >= this->reader.sections.size()) continue;
                ELFIO::section* data_sec = this->reader.sections[index];
                if (!data_sec) continue;

                const char* sec_data = data_sec->get_data();
                if (!sec_data) continue;

                ELFIO::Elf64_Off offset = static_cast<ELFIO::Elf64_Off>(value - data_sec->get_address());
                if (offset + size > data_sec->get_size()) continue;

                // Create X86Global; bytes points into section data (do not free)
                X86Global g;
                g.name = name;
                g.address = value;
                g.size = size;
                g.bytes = sec_data + offset;         // pointer into section data
                g.section = data_sec->get_name();

                globals.push_back(g);
            }
        }
    }
    return globals;
}