#include <lifter/SimpleTraceManager.hpp>

SimpleTraceManager::SimpleTraceManager(
    std::vector<X86Procedure> procs,
    std::vector<X86ExternalProcedure> externalProcs,
    std::unique_ptr<llvm::Module> &module)
    : module(module) {
    this->memory = Memory();

    for (X86Procedure proc : procs) {
        // Initialise names and corresponding LLVM functions
        this->names[proc.address] = std::pair{proc.name, false};

        for (uint64_t i = 0; i < proc.size; i++) {
            this->memory[proc.address + i] = proc.bytes[i];
        }
    }

    for (X86ExternalProcedure proc : externalProcs) {
        this->names[proc.address] = std::pair{proc.name, true};
    }
}

std::string SimpleTraceManager::TraceName(uint64_t addr) {
    std::pair<std::string, bool> name = this->names[addr];

    if (name.second) {
        return "EXTERNAL." + name.first;
    } else {
        return "LIFTED." + name.first;
    }
}

// Called when we have lifted, i.e. defined the contents, of a new trace.
// The derived class is expected to do something useful with this.
void SimpleTraceManager::SetLiftedTraceDefinition(uint64_t addr,
                                                  llvm::Function *lifted_func) {
    bool isExternal = this->names[addr].second;

    // External functions are not defined in the binary (because of
    // dynamic linking) - thus, they should not have traces, and are
    // defined in a separate `externals` file
    if (isExternal) {
        return;
    }

    this->traces[addr] = lifted_func;
}

// Get a declaration for a lifted trace. The idea here is that a derived
// class might have additional global info available to them that lets
// them declare traces ahead of time. In order to distinguish between
// stuff we've lifted, and stuff we haven't lifted, we allow the lifter
// to access "defined" vs. "declared" traces.
//
// NOTE: This is permitted to return a function from an arbitrary module.
llvm::Function *SimpleTraceManager::GetLiftedTraceDeclaration(uint64_t addr) {
    auto names_it = this->names.find(addr);
    auto trace_it = this->traces.find(addr);

    if (names_it != this->names.end()) {
        return this->module->getFunction(TraceName(addr));
    } else if (trace_it != this->traces.end()) {
        return trace_it->second;
    } else {
        return nullptr;
    }
}

// Get a definition for a lifted trace.
//
// NOTE: This is permitted to return a function from an arbitrary module.
llvm::Function *SimpleTraceManager::GetLiftedTraceDefinition(uint64_t addr) {
    auto trace_it = this->traces.find(addr);

    if (trace_it != this->traces.end()) {
        // std::cout << "Found trace at addr " << std::hex << addr << std::dec << std::endl;
        return trace_it->second;
    } else {
        return nullptr;
    }
}

// Try to read an executable byte of memory. Returns `true` of the byte
// at address `addr` is executable and readable, and updates the byte
// pointed to by `byte` with the read value.
bool SimpleTraceManager::TryReadExecutableByte(uint64_t addr, uint8_t *byte) {
    auto byte_it = this->memory.find(addr);

    if (byte_it != this->memory.end()) {
        *byte = byte_it->second;
        return true;
    } else {
        return false;
    }
}
