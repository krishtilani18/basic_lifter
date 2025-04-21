#pragma once

#include <optional>
#include <string>

#include <elfio/elfio.hpp>

typedef std::pair<ELFIO::Elf64_Addr, ELFIO::Elf_Xword> X86FunctionMetadata;

/// @brief Contains the raw code for an x86 function, which includes
/// the bytes of the function, how big the function is, and the address
struct X86Function {
    uint64_t address;
    uint64_t size;
    const char* bytes;
};

std::optional<std::vector<X86Function>> getFunctionLocations(std::string fname);
