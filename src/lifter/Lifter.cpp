#include <iostream>
#include <optional>

#include <lifter/Lifter.hpp>
#include <lifter/Util.hpp>
#include <remill/BC/ABI.h>

/// JUMPS
// [ ] kCategoryDirectJump
// [ ] kCategoryIndirectJump
// [ ] kCategoryConditionalIndirectJump
/// FUNCTION CALLS/RETURNS
// [x] kCategoryDirectFunctionCall
// [ ] kCategoryConditionalDirectFunctionCall
// [ ] kCategoryIndirectFunctionCall
// [ ] kCategoryConditionalIndirectFunctionCall
// [ ] kCategoryFunctionReturn
// [ ] kCategoryConditionalFunctionReturn
/// BRANCHING
// [ ] kCategoryConditionalBranch
/// INTERRUPTS
// [ ] kCategoryAsyncHyperCall
// [ ] kCategoryConditionalAsyncHyperCall

void Lifter::AddFunctionName(uint64_t address, std::string name) {
    this->names[address] = name;
}

llvm::Function *Lifter::Lift(std::string name,
                             std::vector<remill::Instruction> instructions) {
    // Defines a default Remill function
    llvm::Function *func =
        this->arch->DeclareLiftedFunction(name, &this->module);
    this->arch->InitializeEmptyLiftedFunction(func);

    // LLVM requires instructions to live in blocks.
    // Needed for LiftIntoBlock.
    llvm::BasicBlock *codeBlock =
        llvm::BasicBlock::Create(this->context, "code", func);
    llvm::IRBuilder<> builder(codeBlock);

    // Remill uses to use LLVM helper functions to define instructions
    // behaviour.
    remill::IntrinsicTable intrinsics(&this->module);
    auto instLifter =
        std::make_shared<remill::InstructionLifter>(this->arch, &intrinsics);
    auto statePtr =
        llvm::UndefValue::get(this->arch->StateStructType()->getPointerTo());

    // Have register initialisation block branch properly to code block
    if (auto entryBlock = &(func->front())) {
        // TODO: keep for now, but see if this assigning to NEXT_PC is necessary
        // when `InitializeEmptyLiftedFunction` already does this
        auto pc = NthArgument(func, remill::kPCArgNum);
        auto [nextPc, _] =
            this->arch->DefaultLifter(intrinsics)
                ->LoadRegAddress(entryBlock, statePtr,
                                 remill::kNextPCVariableName);

        // Initialize `NEXT_PC`
        (void)new llvm::StoreInst(pc, nextPc, entryBlock);

        // Branch to the first basic block.
        llvm::BranchInst::Create(codeBlock, entryBlock);
    }

    // Lift and print each instruction
    for (const auto &inst : instructions) {
        // uses the instruction, generated IR block and state pointer to
        // generate the corresponding LLVM instruction.
        auto status = instLifter->LiftIntoBlock(
            const_cast<remill::Instruction &>(inst), codeBlock, statePtr);

        // Invalid instruction
        if (status != remill::kLiftedInstruction) {
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
            break;
        }

        switch (inst.category) {
        case remill::Instruction::kCategoryNormal:
        case remill::Instruction::kCategoryNoOp:
            // Normal operations and no-ops do not require any extra LLVM
            // manipulation to map functionality
            break;
        case remill::Instruction::kCategoryDirectFunctionCall:
            LiftDirectCall(inst, func, builder, intrinsics);
            break;
        default:
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
        }
    }

    // Return memory pointer to satisfy LLVM basic block requirements
    auto memoryPtr = FindVarInFunction(func, remill::kMemoryVariableName);
    builder.CreateRet(memoryPtr.value());

    return func;
}

void Lifter::LiftDirectCall(const remill::Instruction &inst,
                            llvm::Function *func, llvm::IRBuilder<> &builder,
                            remill::IntrinsicTable &intrinsics) {
    std::string name = this->names[inst.branch_taken_pc];
    auto target = this->module.getFunction(name);

    // Function doesn't exist yet, generate a default call based on Remill ABI
    if (target == nullptr) {
        std::array<llvm::Value *, remill::kNumBlockArgs> args;
        args[remill::kMemoryPointerArgNum] =
            NthArgument(func, remill::kMemoryPointerArgNum);
        args[remill::kStatePointerArgNum] =
            NthArgument(func, remill::kStatePointerArgNum);
        args[remill::kPCArgNum] = NthArgument(func, remill::kPCArgNum);

        llvm::Type *arg_types[remill::kNumBlockArgs];
        arg_types[remill::kStatePointerArgNum] =
            args[remill::kStatePointerArgNum]->getType();
        arg_types[remill::kMemoryPointerArgNum] =
            args[remill::kMemoryPointerArgNum]->getType();
        arg_types[remill::kPCArgNum] = args[remill::kPCArgNum]->getType();
        auto func_type = llvm::FunctionType::get(
            arg_types[remill::kMemoryPointerArgNum], arg_types, false);

        llvm::FunctionCallee callee(func_type, target);
        builder.CreateCall(callee, args);
        return;
    }

    // Lift Remill ABI function arguments
    std::array<llvm::Value *, remill::kNumBlockArgs> args;

    auto memoryPtr = FindVarInFunction(func, remill::kMemoryVariableName);
    auto memoryLoad =
        builder.CreateLoad(intrinsics.mem_ptr_type, memoryPtr.value());
    args[remill::kMemoryPointerArgNum] = memoryLoad;

    args[remill::kStatePointerArgNum] =
        NthArgument(func, remill::kStatePointerArgNum);

    auto pcPtr = FindVarInFunction(func, remill::kPCVariableName);
    auto pcLoad = builder.CreateLoad(intrinsics.pc_type, pcPtr.value());
    args[remill::kPCArgNum] = pcLoad;

    // Make call statement in block
    builder.CreateCall(target, args);
}
