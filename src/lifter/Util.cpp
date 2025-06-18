#include <llvm/IR/Instructions.h>

#include "lifter/Util.hpp"

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
