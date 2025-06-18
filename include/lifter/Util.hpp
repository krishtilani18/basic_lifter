#pragma once

#include <llvm/IR/Value.h>
#include <optional>
#include <string>

/// @brief Find the LLVM representation of a variable in `func`.
/// @param func the function we want to find our variable in
/// @param name the name of the variable
/// @return the actual `llvm::Value *` corresponding to the variable, or
/// `std::nullopt` if no such variable exists
std::optional<llvm::Value *> FindVarInFunction(llvm::Function *func,
                                               std::string_view name);


/// @brief Find the LLVM representation of the `index`-th argument in `func`.
/// Mostly used to extract the `state`, `memory` and `pc` pointers in a Remill
/// ABI-compliant LLVM function.
/// @param func the function we want to extract an argument from
/// @param index the index of the argument
/// @return an `llvm::Argument` corresponding to that indexed argument
llvm::Argument *NthArgument(llvm::Function *func, unsigned int index);
