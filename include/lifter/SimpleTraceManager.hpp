#pragma once

#include <map>
#include <reader/X86Reader.hpp>
#include <remill/BC/TraceLifter.h>

using Memory = std::map<uint64_t, uint8_t>;

class SimpleTraceManager : public remill::TraceManager {
  public:
    SimpleTraceManager(std::vector<X86Procedure> funcs);

    llvm::Function *GetLiftedTraceDefinition(uint64_t addr) override;

  protected:
    std::string TraceName(uint64_t addr) override;

    void SetLiftedTraceDefinition(uint64_t addr,
                                  llvm::Function *lifted_func) override;

    llvm::Function *GetLiftedTraceDeclaration(uint64_t addr) override;

    bool TryReadExecutableByte(uint64_t addr, uint8_t *byte) override;

  public:
    // Metadata
    Memory memory;
    std::unordered_map<uint64_t, std::string> names;

    // Lifted traces
    std::unordered_map<uint64_t, llvm::Function *> traces;
};
