#include <lifter/SimpleTraceManager.hpp>

SimpleTraceManager::SimpleTraceManager(std::vector<X86Procedure> procs,
                                       std::unique_ptr<llvm::Module> &module)
    : module(module) {
    this->memory = Memory();

    for (X86Procedure proc : procs) {
        // Initialise names and corresponding LLVM functions
        this->names[proc.address] = proc.name;

        for (uint64_t i = 0; i < proc.size; i++) {
            this->memory[proc.address + i] = proc.bytes[i];
        }
    }
}

std::string SimpleTraceManager::TraceName(uint64_t addr) {
    return "LIFTED." + this->names[addr];
}

// Called when we have lifted, i.e. defined the contents, of a new trace.
// The derived class is expected to do something useful with this.
void SimpleTraceManager::SetLiftedTraceDefinition(uint64_t addr,
                                                  llvm::Function *lifted_func) {
    traces[addr] = lifted_func;
}

// Get a declaration for a lifted trace. The idea here is that a derived
// class might have additional global info available to them that lets
// them declare traces ahead of time. In order to distinguish between
// stuff we've lifted, and stuff we haven't lifted, we allow the lifter
// to access "defined" vs. "declared" traces.
//
// NOTE: This is permitted to return a function from an arbitrary module.
llvm::Function *SimpleTraceManager::GetLiftedTraceDeclaration(uint64_t addr) {
    auto names_it = names.find(addr);
    auto trace_it = traces.find(addr);

    if (names_it != names.end()) {
        return this->module->getFunction(TraceName(addr));
    } else if (trace_it != traces.end()) {
        return trace_it->second;
    } else {
        return nullptr;
    }
}

// Get a definition for a lifted trace.
//
// NOTE: This is permitted to return a function from an arbitrary module.
llvm::Function *SimpleTraceManager::GetLiftedTraceDefinition(uint64_t addr) {
    auto trace_it = traces.find(addr);

    if (trace_it != traces.end()) {
        return trace_it->second;
    } else {
        return nullptr;
    }
}

// Try to read an executable byte of memory. Returns `true` of the byte
// at address `addr` is executable and readable, and updates the byte
// pointed to by `byte` with the read value.
bool SimpleTraceManager::TryReadExecutableByte(uint64_t addr, uint8_t *byte) {
    auto byte_it = memory.find(addr);
    
    if (byte_it != memory.end()) {
        *byte = byte_it->second;
        return true;
    } else {
        return false;
    }
}
