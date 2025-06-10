#include <lifter/Lifter.hpp>

llvm::Function *Lifter::Lift(std::string name,
                             std::vector<remill::Instruction> instructions) {
    // Defines a default Remill function
    llvm::Function *func = arch->DeclareLiftedFunction(name, &this->module);
    arch->InitializeEmptyLiftedFunction(func);

    // LLVM requires instructions to live in blocks.
    // Needed for LiftIntoBlock.
    llvm::BasicBlock *block = llvm::BasicBlock::Create(this->context, "entry", func);
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

        // valid instruction
        if (status != remill::kLiftedInstruction) {
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
        }
    }

    return func;
}
