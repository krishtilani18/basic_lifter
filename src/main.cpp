#include <inttypes.h>
#include <iomanip>
#include <iostream>
#include <remill/Arch/Name.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include <disass/NaiveDisassembler.hpp>
#include <elf.hpp>
#include <lifter/Lifter.hpp>

// Right now, our lifter extracts code sections from x86 binaries
// and converts them into chunks of instructions, which can (in future)
// then be placed into LLVM functions.
int main(int argc, char *argv[]) {
    /// === EXTRACT FUNCTION LOCATIONS ===
    auto procsOptional = getProcedures(argv[1]);

    if (!procsOptional.has_value()) {
        return 0;
    }

    auto procs = procsOptional.value();

    /// === LIFT BYTES INTO X86 INSTRUCTIONS ===
    llvm::LLVMContext context;
    auto os_name = remill::GetOSName(REMILL_OS);
    auto arch_name = remill::GetArchName(REMILL_ARCH);

    // Creating the arch object as a unique_ptr
    auto arch = remill::Arch::Get(context, os_name, arch_name);

    // Necessary line - otherwise module gets dropped
    auto module = remill::LoadArchSemantics(arch.get());

    auto disass = NaiveDisassembler(arch.get());
    auto lifter = Lifter(*module, context, arch.get());

    for (X86Procedure proc : procs) {
        lifter.AddFunctionName(proc.address, proc.name);
    }

    std::vector<llvm::Function *> funcs;

    for (X86Procedure proc : procs) {
        auto instructions = disass.Disassemble(proc);

        /// === LIFT X86 INSTRUCTIONS INTO LLVM INSTRUCTIONS ===
        auto func = lifter.Lift(proc.name, instructions);
        funcs.push_back(func);
    }

    module->print(llvm::outs(), nullptr);
}
