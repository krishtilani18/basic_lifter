#include <string>

#include <elfio/elfio.hpp>

void getTextSections(std::string fname) {
    ELFIO::elfio reader;

    if (!reader.load(fname)) {
        // Error
    }

    ELFIO::Elf_Half sec_num = reader.sections.size();
    std::cout << "Number of sections: " << sec_num << std::endl;

    for (int i = 0; i < sec_num; ++i) {
        const ELFIO::section* psec = reader.sections[i];
        std::cout << "  [" << i << "] " << psec->get_name()
                  << "\t" << psec->get_size()
                  << std::endl; // Access section's data const
    }
}
