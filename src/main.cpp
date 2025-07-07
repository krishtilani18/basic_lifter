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
#include <lifter/SimpleTraceManager.hpp>

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
