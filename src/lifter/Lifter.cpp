#include <iostream>
#include <optional>

#include <lifter/Lifter.hpp>
#include <lifter/Util.hpp>

/// JUMPS
// [ ] kCategoryDirectJump
// [ ] kCategoryIndirectJump
// [ ] kCategoryConditionalIndirectJump
/// FUNCTION CALLS/RETURNS
// [x] kCategoryDirectFunctionCall
// [ ] kCategoryConditionalDirectFunctionCall
// [ ] kCategoryIndirectFunctionCall
// [ ] kCategoryConditionalIndirectFunctionCall
// [x] kCategoryFunctionReturn
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
    this->func = this->arch->DeclareLiftedFunction(name, &this->module);
    this->arch->InitializeEmptyLiftedFunction(this->func);

    // LLVM requires instructions to live in blocks.
    // Needed for LiftIntoBlock.
    this->block = llvm::BasicBlock::Create(this->context, "code", this->func);
    llvm::IRBuilder<> builder(this->block);

    // Remill uses to use LLVM helper functions to define instructions
    // behaviour.
    remill::IntrinsicTable intrinsics(&this->module);
    auto instLifter =
        std::make_shared<remill::InstructionLifter>(this->arch, &intrinsics);
    auto statePtr =
        llvm::UndefValue::get(this->arch->StateStructType()->getPointerTo());

    // Have register initialisation block branch properly to code block
    if (auto entryBlock = &(this->func->front())) {
        // TODO: keep for now, but see if this assigning to NEXT_PC is necessary
        // when `InitializeEmptyLiftedFunction` already does this
        auto pc = NthArgument(this->func, remill::kPCArgNum);
        auto [nextPc, _] = this->arch->DefaultLifter(intrinsics)
                               ->LoadRegAddress(entryBlock, statePtr,
                                                remill::kNextPCVariableName);

        // Initialize `NEXT_PC`
        (void)new llvm::StoreInst(pc, nextPc, entryBlock);

        // Branch to the first basic block.
        llvm::BranchInst::Create(this->block, entryBlock);
    }

    // Lift and print each instruction
    for (const auto &inst : instructions) {
        // uses the instruction, generated IR block and state pointer to
        // generate the corresponding LLVM instruction.
        auto status = instLifter->LiftIntoBlock(
            const_cast<remill::Instruction &>(inst), this->block, statePtr);

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
        case remill::Instruction::kCategoryDirectFunctionCall: {
            std::string name = this->names[inst.branch_taken_pc];
            LiftDirectCall(name, intrinsics);
            break;
        }
        case remill::Instruction::kCategoryFunctionReturn:
            LiftReturn(intrinsics.function_return, intrinsics);
            break;
        default:
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
        }
    }

    // If the block doesn't have a terminator, add a return statement
    if (!this->block->getTerminator()) {
        auto memoryPtr = FindVarInFunction(func, remill::kMemoryVariableName);
        builder.CreateRet(memoryPtr.value());
    }

    return this->func;
}

void Lifter::CallPlaceholder() {
    llvm::IRBuilder<> builder(this->block);

    auto args = GetArgs();

    llvm::Type *arg_types[remill::kNumBlockArgs];
    arg_types[remill::kStatePointerArgNum] =
        args[remill::kStatePointerArgNum]->getType();
    arg_types[remill::kMemoryPointerArgNum] =
        args[remill::kMemoryPointerArgNum]->getType();
    arg_types[remill::kPCArgNum] = args[remill::kPCArgNum]->getType();
    auto func_type = llvm::FunctionType::get(
        arg_types[remill::kMemoryPointerArgNum], arg_types, false);

    llvm::FunctionCallee callee(func_type, nullptr);
    builder.CreateCall(callee, args);
}

std::array<llvm::Value *, remill::kNumBlockArgs> Lifter::GetArgs() {
    std::array<llvm::Value *, remill::kNumBlockArgs> args;

    args[remill::kMemoryPointerArgNum] =
        NthArgument(this->func, remill::kMemoryPointerArgNum);
    args[remill::kStatePointerArgNum] =
        NthArgument(this->func, remill::kStatePointerArgNum);
    args[remill::kPCArgNum] = NthArgument(this->func, remill::kPCArgNum);

    return args;
}

void Lifter::LiftDirectCall(std::string name,
                            remill::IntrinsicTable &intrinsics) {
    llvm::IRBuilder<> builder(this->block);

    auto target = this->module.getFunction(name);

    // Function doesn't exist yet, generate a default call based on Remill ABI
    if (target == nullptr) {
        CallPlaceholder();
        return;
    }

    // Lift Remill ABI function arguments
    std::array<llvm::Value *, remill::kNumBlockArgs> args;

    auto memoryPtr = FindVarInFunction(this->func, remill::kMemoryVariableName);
    auto memoryLoad =
        builder.CreateLoad(intrinsics.mem_ptr_type, memoryPtr.value());
    args[remill::kMemoryPointerArgNum] = memoryLoad;

    args[remill::kStatePointerArgNum] =
        NthArgument(func, remill::kStatePointerArgNum);

    auto pcPtr = FindVarInFunction(this->func, remill::kPCVariableName);
    auto pcLoad = builder.CreateLoad(intrinsics.pc_type, pcPtr.value());
    args[remill::kPCArgNum] = pcLoad;

    // Make call statement in block
    builder.CreateCall(target, args);
}

void Lifter::LiftReturn(llvm::Value *dest, remill::IntrinsicTable &intrinsics) {
    llvm::IRBuilder<> builder(this->block);

    // Set `NEXT_PC` to `PC`
    auto nextPc = FindVarInFunction(this->func, remill::kNextPCVariableName);
    auto nextPcLoad = builder.CreateLoad(intrinsics.pc_type, nextPc.value());

    auto pcPtr = FindVarInFunction(this->func, remill::kPCVariableName);
    (void)new llvm::StoreInst(nextPcLoad, pcPtr.value(),
                              builder.GetInsertBlock());

    // NOTE: Remill's `TraceLifter` lifts an extra call to `@__remill_function_return`,
    // which our lifter currently does not do.
    // TODO: consider implementing our custom intrinsic instead of ignoring call?
    auto memoryPtr = FindVarInFunction(func, remill::kMemoryVariableName);
    builder.CreateRet(memoryPtr.value());
}
