#pragma once

#include <optional>
#include <string>

#include <elfio/elfio.hpp>

typedef std::pair<ELFIO::Elf64_Addr, ELFIO::Elf_Xword> X86FunctionMetadata;

struct X86Function {
    uint64_t address;
    uint64_t size;
    const char* bytes;
};

std::optional<std::vector<X86Function>> getFunctionLocations(std::string fname);
