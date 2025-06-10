#include <llvm/IR/IRBuilder.h>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Instruction.h>
#include <remill/BC/IntrinsicTable.h>

#include <lifter/lifter.hpp>

void LiftInstructionsToLLVM(
    llvm::Module &module,
    llvm::LLVMContext &context,
    const remill::Arch *arch,
    std::string name,
    const std::vector<remill::Instruction> &instructions) {

    // Defines a default Remill function
    llvm::Function* func = arch->DeclareLiftedFunction(name, &module);
    arch->InitializeEmptyLiftedFunction(func);

    // LLVM requires instructions to live in blocks.
    // Needed for LiftIntoBlock.
    llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", func);
    llvm::IRBuilder<> builder(block);

    // Remill uses to use LLVM helper functions to define instructions behaviour.
    remill::IntrinsicTable intrinsics(&module);
    auto lifter = std::make_shared<remill::InstructionLifter>(arch, &intrinsics);
    auto state_ptr = llvm::UndefValue::get(arch->StateStructType()->getPointerTo());


    // Lift and print each instruction:
    for (const auto &inst : instructions) {
        size_t old_size = block->size();
        
        // uses the instruction, generated IR block and state pointer to generate the corresponding LLVM instruction.
        auto status = lifter->LiftIntoBlock(const_cast<remill::Instruction &>(inst), block, state_ptr);

        // valid instruction
        if (status != remill::kLiftedInstruction) {
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
        }
    }

    func->print(llvm::outs());
}
