#include <iostream>
#include <optional>

#include <lifter/Lifter.hpp>
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
    llvm::Function *func = arch->DeclareLiftedFunction(name, &this->module);
    this->arch->InitializeEmptyLiftedFunction(func);

    // LLVM requires instructions to live in blocks.
    // Needed for LiftIntoBlock.
    llvm::BasicBlock *block =
        llvm::BasicBlock::Create(this->context, "entry", func);
    llvm::IRBuilder<> builder(block);

    // Remill uses to use LLVM helper functions to define instructions
    // behaviour.
    remill::IntrinsicTable intrinsics(&this->module);
    auto lifter =
        std::make_shared<remill::InstructionLifter>(this->arch, &intrinsics);
    auto state_ptr =
        llvm::UndefValue::get(this->arch->StateStructType()->getPointerTo());

    // Lift and print each instruction:
    for (const auto &inst : instructions) {
        size_t old_size = block->size();

        // uses the instruction, generated IR block and state pointer to
        // generate the corresponding LLVM instruction.
        auto status = lifter->LiftIntoBlock(
            const_cast<remill::Instruction &>(inst), block, state_ptr);

        // Invalid instruction
        if (status != remill::kLiftedInstruction) {
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
            break;
        }

        // Lift function calls properly
        if (inst.category == remill::Instruction::kCategoryDirectFunctionCall) {
            LiftDirectCall(inst, func, builder, intrinsics);
        }
    }

    return func;
}

// Copied from Remill internals, with parts looking in global scope stripped out
std::optional<llvm::Value *> FindVarInFunction(llvm::Function *func,
                                               std::string_view name) {
    llvm::StringRef ref(name.data(), name.size());

    for (auto &inst : func->getEntryBlock()) {
        if (inst.getName() == ref) {
            if (auto *alloca = llvm::dyn_cast<llvm::AllocaInst>(&inst)) {
                return alloca;
            }
            if (auto *gep = llvm::dyn_cast<llvm::GetElementPtrInst>(&inst)) {
                return gep;
            }
        }
    }

    return std::nullopt;
}

llvm::Argument *NthArgument(llvm::Function *func, unsigned int index) {
    auto it = func->arg_begin();

    if (index >= static_cast<size_t>(std::distance(it, func->arg_end()))) {
        return nullptr;
    }

    std::advance(it, index);
    return &*it;
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
