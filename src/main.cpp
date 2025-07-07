#include <array>
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

#include <lifter/SimpleTraceManager.hpp>
#include <reader/X86Reader.hpp>

// Right now, our lifter extracts code sections from x86 binaries
// and converts them into chunks of instructions, which can (in future)
// then be placed into LLVM functions.
int main(int argc, char *argv[]) {
    /// === EXTRACT PROCEDURE LOCATIONS ===
    ELFIO::elfio elfReader;

    if (!elfReader.load(argv[1])) {
        return 1;
    }

    X86Reader reader(elfReader);
    auto procs = reader.GetProcedures();

    /// === LIFT X86 PROCEDURES INTO LLVM IR ===
    llvm::LLVMContext context;
    auto os_name = remill::GetOSName(REMILL_OS);
    auto arch_name = remill::GetArchName(REMILL_ARCH);

    // Creating the arch object as a unique_ptr
    auto arch = remill::Arch::Get(context, os_name, arch_name);

    // Necessary line - otherwise module gets dropped
    auto module = remill::LoadArchSemantics(arch.get());

    // Initialise lifter
    SimpleTraceManager manager(procs);
    auto lifter = remill::TraceLifter(arch.get(), manager);

    // Lift the program, using the function info we got from elf.hpp
    // as an entry point
    for (X86Procedure func : procs) {
        lifter.Lift(func.address);
    }

    // Optimize the functions we lifted
    remill::OptimizationGuide guide = {};
    remill::OptimizeModule(arch, module, manager.traces, guide);

    // Move lifted functions into new module
    llvm::Module destModule("lifted_code", context);
    arch->PrepareModuleDataLayout(&destModule);

    for (auto trace : manager.traces) {
        remill::MoveFunctionIntoModule(trace.second, &destModule);
    }

    /// === CALL ENTRY POINT ===

    // Create main function
    auto i32Type = llvm::Type::getInt32Ty(context);
    const auto mainFuncType =
        llvm::FunctionType::get(i32Type, false);

    auto mainFunc = llvm::Function::Create(
        mainFuncType, llvm::GlobalValue::ExternalLinkage, "main", destModule);

    // Create global state struct, initialise to 0
    auto stateType = arch->StateStructType();
    auto stateInit = llvm::ConstantAggregateZero::get(stateType);

    const auto statePtr = new llvm::GlobalVariable(
        destModule, stateType, false, llvm::GlobalValue::ExternalLinkage,
        stateInit, "LIFTED.STATE", nullptr,
        llvm::GlobalValue::InitialExecTLSModel);

    // Create memory pointer
    auto memoryType = arch->MemoryPointerType();
    auto memoryPtr = llvm::Constant::getNullValue(memoryType);

    // Create program counter
    auto entryPoint = reader.GetEntry();

    auto wordType = llvm::Type::getIntNTy(
        context, static_cast<unsigned>(arch->address_size));
    auto pc = llvm::ConstantInt::get(wordType, entryPoint);

    // Have main function directly call entry point
    llvm::IRBuilder<> ir(llvm::BasicBlock::Create(context, "", mainFunc));

    std::array<llvm::Value *, remill::kNumBlockArgs> args;
    args[remill::kStatePointerArgNum] = statePtr;
    args[remill::kPCArgNum] = pc;
    args[remill::kMemoryPointerArgNum] = memoryPtr;

    auto entryFunc = manager.GetLiftedTraceDefinition(entryPoint);
    ir.CreateCall(entryFunc, args);

    // Assembly return values are contained in `rax`, return value from there
    auto remillRax = arch->RegisterByName("RAX");
    auto rax = remillRax->AddressOf(statePtr, ir);
    ir.CreateRet(rax);

    /// === OUTPUT MODULE ===
    destModule.print(llvm::outs(), nullptr);
}
