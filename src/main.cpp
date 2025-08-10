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
    auto module = remill::LoadArchSemantics(arch.get());

    // Declare empty functions in module first, which the lifter
    // can see (for recursive functions)
    for (X86Procedure proc : procs) {
        arch->DeclareLiftedFunction("LIFTED." + proc.name, module.get());
    }

    // Initialise lifter
    SimpleTraceManager manager(procs, module);
    auto lifter = remill::TraceLifter(arch.get(), manager);

    // Lift the program, using the function info we got from elf.hpp
    // as an entry point
    for (X86Procedure proc : procs) {
        lifter.Lift(proc.address);
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
    auto i32Type = llvm::Type::getInt32Ty(context);
    auto i64Type = llvm::Type::getInt64Ty(context);

    // Create main function
    const auto mainFuncType = llvm::FunctionType::get(i32Type, false);

    auto mainFunc = llvm::Function::Create(
        mainFuncType, llvm::GlobalValue::ExternalLinkage, "main", destModule);

    // Create global state struct, initialise to 0
    auto stateType = arch->StateStructType();
    auto stateInit = llvm::ConstantAggregateZero::get(stateType);

    const auto statePtr = new llvm::GlobalVariable(
        destModule, stateType, false, llvm::GlobalValue::ExternalLinkage,
        stateInit, "LIFTED.STATE", nullptr,
        llvm::GlobalValue::InitialExecTLSModel);

    // Create program counter
    auto entryPoint = reader.GetEntry();

    auto wordType = llvm::Type::getIntNTy(
        context, static_cast<unsigned>(arch->address_size));
    auto pc = llvm::ConstantInt::get(wordType, entryPoint);

    // === CONSTRUCT MAIN FUNCTION ===
    llvm::IRBuilder<> ir(llvm::BasicBlock::Create(context, "", mainFunc));

    // Initialise memory map
    auto memoryType = arch->MemoryPointerType();

    auto initMemoryFunc = destModule.getOrInsertFunction(
        "__lifter_init_memory",
        llvm::FunctionType::get(memoryType, false));

    auto memoryPtr = ir.CreateCall(initMemoryFunc);

    // Call 
    std::array<llvm::Value *, remill::kNumBlockArgs> args;
    args[remill::kStatePointerArgNum] = statePtr;
    args[remill::kPCArgNum] = pc;
    args[remill::kMemoryPointerArgNum] = memoryPtr;

    auto entryFunc = manager.GetLiftedTraceDefinition(entryPoint);
    ir.CreateCall(entryFunc, args);

    // Assembly return values are contained in `rax`, return value from there
    auto remillEax = arch->RegisterByName("EAX");
    auto eax = remillEax->AddressOf(statePtr, ir);

    auto loadInst = ir.CreateLoad(i32Type, eax);

    // Make sure to destroy pointer, preventing memory leak
    auto freeMemoryFunc = destModule.getOrInsertFunction(
        "__lifter_free_memory",
        llvm::FunctionType::get(llvm::Type::getVoidTy(context), memoryType,
                                false));

    std::array<llvm::Value *, 1> freeMemoryArgs;
    freeMemoryArgs[0] = memoryPtr;
    ir.CreateCall(freeMemoryFunc, freeMemoryArgs);

    ir.CreateRet(loadInst);

    /// === OUTPUT MODULE ===
    destModule.print(llvm::outs(), nullptr);
}
