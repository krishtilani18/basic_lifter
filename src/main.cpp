#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Name.h>
#include <remill/BC/Optimizer.h>
#include <remill/BC/TraceLifter.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>
#include <unordered_map>

#include <elf.hpp>

using Memory = std::map<uint64_t, uint8_t>;

class SimpleTraceManager : public remill::TraceManager {
  public:
    virtual ~SimpleTraceManager(void) = default;

    SimpleTraceManager(std::vector<X86Function> funcs) {
        this->memory = Memory();

        for (X86Function func : funcs) {
            for (uint64_t i = 0; i < func.size; i++) {
                this->memory[func.address + i] = func.bytes[i];
            }
        }
    }

  protected:
    // Called when we have lifted, i.e. defined the contents, of a new trace.
    // The derived class is expected to do something useful with this.
    void SetLiftedTraceDefinition(uint64_t addr,
                                  llvm::Function *lifted_func) override {
        traces[addr] = lifted_func;
    }

    // Get a declaration for a lifted trace. The idea here is that a derived
    // class might have additional global info available to them that lets
    // them declare traces ahead of time. In order to distinguish between
    // stuff we've lifted, and stuff we haven't lifted, we allow the lifter
    // to access "defined" vs. "declared" traces.
    //
    // NOTE: This is permitted to return a function from an arbitrary module.
    llvm::Function *GetLiftedTraceDeclaration(uint64_t addr) override {
        auto trace_it = traces.find(addr);
        if (trace_it != traces.end()) {
            return trace_it->second;
        } else {
            return nullptr;
        }
    }

    // Get a definition for a lifted trace.
    //
    // NOTE: This is permitted to return a function from an arbitrary module.
    llvm::Function *GetLiftedTraceDefinition(uint64_t addr) override {
        return GetLiftedTraceDeclaration(addr);
    }

    // Try to read an executable byte of memory. Returns `true` of the byte
    // at address `addr` is executable and readable, and updates the byte
    // pointed to by `byte` with the read value.
    bool TryReadExecutableByte(uint64_t addr, uint8_t *byte) override {
        auto byte_it = memory.find(addr);
        if (byte_it != memory.end()) {
            *byte = byte_it->second;
            return true;
        } else {
            return false;
        }
    }

  public:
    Memory memory;
    std::unordered_map<uint64_t, llvm::Function *> traces;
};

// Right now, our lifter extracts code sections from x86 binaries
// and converts them into chunks of instructions, which can (in future)
// then be placed into LLVM functions.
int main(int argc, char *argv[]) {
    /// === EXTRACT FUNCTION LOCATIONS ===
    auto funcsOptional = getFunctionLocations(argv[1]);

    if (!funcsOptional.has_value()) {
        return 0;
    }

    auto funcs = funcsOptional.value();

    /// === LIFT BYTES INTO X86 INSTRUCTIONS ===
    llvm::LLVMContext context;
    auto os_name = remill::GetOSName(REMILL_OS);
    auto arch_name = remill::GetArchName(REMILL_ARCH);

    // Creating the arch object as a unique_ptr
    auto arch = remill::Arch::Get(context, os_name, arch_name);

    // Necessary line - otherwise module gets dropped
    auto module = remill::LoadArchSemantics(arch.get());

    // Initialise lifter
    SimpleTraceManager manager(funcs);
    auto lifter = remill::TraceLifter(arch.get(), manager);

    // Lift the program, using the function info we got from elf.hpp
    // as an entry point
    for (X86Function func : funcs) {
        lifter.Lift(func.address);
    }

    // Optimize the functions we lifted
    remill::OptimizationGuide guide = {};
    remill::OptimizeModule(arch, module, manager.traces, guide);

    // Move lifted functions into new module
    llvm::Module dest_module("lifted_code", context);
    arch->PrepareModuleDataLayout(&dest_module);

    for (auto trace : manager.traces) {
        remill::MoveFunctionIntoModule(trace.second, &dest_module);
    }

    // Output lifted result
    dest_module.print(llvm::outs(), nullptr);
}
