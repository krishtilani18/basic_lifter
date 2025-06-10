#pragma once

#include <optional>
#include <string>

#include <elfio/elfio.hpp>

struct X86FunctionMetadata {
    std::string name;
    ELFIO::Elf64_Addr address;
    ELFIO::Elf_Xword size;
};

/// @brief Contains the raw code for an x86 function, which includes
/// the bytes of the function, how big the function is, and the address
struct X86Function {
    std::string name;
    uint64_t address;
    uint64_t size;
    const char *bytes;
};

std::optional<std::vector<X86Function>> getFunctionLocations(std::string fname);
