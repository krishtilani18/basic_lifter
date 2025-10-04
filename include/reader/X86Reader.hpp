#pragma once

#include <string>
#include <vector>

#include <elfio/elfio.hpp>

/// @brief The metadata for an x86 procedure, extracted from an ELF file's
/// symbol table
struct X86ProcedureMetadata {
    std::string name;
    uint64_t address;
    uint64_t size;
};

/// @brief Contains the raw code for an x86 procedure, which includes
/// the bytes of the procedure, how big the procedure is, and the address
struct X86Procedure {
    std::string name;
    uint64_t address;
    uint64_t size;
    const char *bytes;
};

/// @brief A call to a dynamically-linked external procedure
struct X86ExternalProcedure {
    std::string name;
    uint64_t address;
};

class X86Reader {
  public:
    X86Reader(ELFIO::elfio &_reader) : reader(_reader) {}

    std::vector<X86Procedure> GetProcedures();
    std::vector<X86ExternalProcedure> GetExternalProcedures();

  private:
    ELFIO::elfio &reader;
};
