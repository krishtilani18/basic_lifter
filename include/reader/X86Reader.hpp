#pragma once

#include <string>
#include <vector>

#include <elfio/elfio.hpp>

struct X86FunctionMetadata {
    std::string name;
    uint64_t address;
    uint64_t size;
};

/// @brief Contains the raw code for an x86 function, which includes
/// the bytes of the function, how big the function is, and the address
struct X86Function {
    std::string name;
    uint64_t address;
    uint64_t size;
    const char *bytes;
};

class X86Reader {
  public:
    X86Reader(ELFIO::elfio &_reader) : reader(_reader) {}

    ELFIO::Elf64_Addr GetEntry();
    std::vector<X86Function> GetFunctions();

  private:
    ELFIO::elfio &reader;
};
