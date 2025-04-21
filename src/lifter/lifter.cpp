#include <llvm/IR/IRBuilder.h>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Instruction.h>
#include <remill/BC/IntrinsicTable.h>

#include <lifter/lifter.hpp>

void LiftInstructionsToLLVM(
    llvm::Module &module,
    llvm::LLVMContext &context,
    const remill::Arch *arch,
    const std::vector<remill::Instruction> &instructions) {

    // Defines a dummy function type and creates an LLVM function
    llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
    llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "lifted_func", module);

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
        if (status == remill::kLiftedInstruction) {
            auto it = block->begin();
            std::advance(it, old_size); 

            for (; it != block->end(); ++it) {
                // only prints call instructions
                if (llvm::isa<llvm::CallInst>(&*it)) {
                    it->print(llvm::outs());
                    llvm::outs() << "\n";
                }
                // it->print(llvm::outs());
                // llvm::outs() << "\n";
            }
        } else {
            llvm::errs() << "Failed to lift: " << inst.Serialize() << "\n";
        }
    }
}
