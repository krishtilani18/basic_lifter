/*
 * Copyright (c) 2020 Trail of Bits, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <glog/logging.h>
#include <lifter/TraceLifter.hpp>
#include <llvm/IR/Instructions.h>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Instruction.h>
#include <remill/BC/IntrinsicTable.h>
#include <remill/BC/Util.h>

#include <map>
#include <set>
#include <sstream>

TraceManager::~TraceManager(void) {}

// Return an already lifted trace starting with the code at address
// `addr`.
llvm::Function *TraceManager::GetLiftedTraceDeclaration(uint64_t) {
  return nullptr;
}

// Return an already lifted trace starting with the code at address
// `addr`.
llvm::Function *TraceManager::GetLiftedTraceDefinition(uint64_t) {
  return nullptr;
}

// Apply a callback that gives the decoder access to multiple virtual
// targets of this instruction (indirect call or jump).
void TraceManager::ForEachDevirtualizedTarget(
    const remill::Instruction &,
    std::function<void(uint64_t, DevirtualizedTargetKind)>) {

  // Must be extended.
}

// Figure out the name for the trace starting at address `addr`.
std::string TraceManager::TraceName(uint64_t addr) {
  std::stringstream ss;
  ss << "sub_" << std::hex << addr;
  return ss.str();
}

namespace {

using DecoderWorkList = std::set<uint64_t>;  // For ordering.

}  // namespace

class TraceLifter::Impl {
 public:
  Impl(const remill::Arch *arch_, TraceManager *manager_);

  // Lift one or more traces starting from `addr`. Calls `callback` with each
  // lifted trace.
  bool Lift(uint64_t addr,
            std::function<void(uint64_t, llvm::Function *)> callback);

  // Reads the bytes of an instruction at `addr` into `state.inst_bytes`.
  bool ReadInstructionBytes(uint64_t addr);

  // Return an already lifted trace starting with the code at address
  // `addr`.
  //
  // NOTE: This is guaranteed to return either `nullptr`, or a function
  //       within `module`.
  llvm::Function *GetLiftedTraceDeclaration(uint64_t addr);

  // Return an already lifted trace starting with the code at address
  // `addr`.
  //
  // NOTE: This is guaranteed to return either `nullptr`, or a function
  //       within `module`.
  llvm::Function *GetLiftedTraceDefinition(uint64_t addr);

  llvm::BasicBlock *GetOrCreateBlock(uint64_t block_pc) {
    auto &block = blocks[block_pc];
    if (!block) {
      block = llvm::BasicBlock::Create(context, "", func);
    }
    return block;
  }

  llvm::BasicBlock *GetOrCreateBranchTakenBlock(void) {
    inst_work_list.insert(inst.branch_taken_pc);
    return GetOrCreateBlock(inst.branch_taken_pc);
  }

  llvm::BasicBlock *GetOrCreateBranchNotTakenBlock(void) {
    CHECK(inst.branch_not_taken_pc != 0);
    inst_work_list.insert(inst.branch_not_taken_pc);
    return GetOrCreateBlock(inst.branch_not_taken_pc);
  }

  llvm::BasicBlock *GetOrCreateNextBlock(void) {
    inst_work_list.insert(inst.next_pc);
    return GetOrCreateBlock(inst.next_pc);
  }

  uint64_t PopTraceAddress(void) {
    auto trace_it = trace_work_list.begin();
    const auto trace_addr = *trace_it;
    trace_work_list.erase(trace_it);
    return trace_addr;
  }

  uint64_t PopInstructionAddress(void) {
    auto inst_it = inst_work_list.begin();
    const auto inst_addr = *inst_it;
    inst_work_list.erase(inst_it);
    return inst_addr;
  }

  const remill::Arch *const arch;
  const remill::IntrinsicTable *intrinsics;
  llvm::Type *word_type;
  llvm::LLVMContext &context;
  llvm::Module *const module;
  const uint64_t addr_mask;
  TraceManager &manager;

  llvm::Function *func;
  llvm::BasicBlock *block;
  llvm::SwitchInst *switch_inst;
  const size_t max_inst_bytes;
  std::string inst_bytes;
  remill::Instruction inst;
  DecoderWorkList trace_work_list;
  DecoderWorkList inst_work_list;
  std::map<uint64_t, llvm::BasicBlock *> blocks;
};

TraceLifter::Impl::Impl(const remill::Arch *arch_, TraceManager *manager_)
    : arch(arch_),
      intrinsics(arch->GetInstrinsicTable()),
      word_type(arch->AddressType()),
      context(word_type->getContext()),
      module(intrinsics->async_hyper_call->getParent()),
      addr_mask(arch->address_size >= 64 ? ~0ULL
                                         : (~0ULL >> arch->address_size)),
      manager(*manager_),
      func(nullptr),
      block(nullptr),
      switch_inst(nullptr),
      // TODO(Ian): The trace lfiter is not supporting contexts
      max_inst_bytes(arch->MaxInstructionSize(arch->CreateInitialContext())) {

  inst_bytes.reserve(max_inst_bytes);
}

// Return an already lifted trace starting with the code at address
// `addr`.
llvm::Function *TraceLifter::Impl::GetLiftedTraceDeclaration(uint64_t addr) {
  auto func = manager.GetLiftedTraceDeclaration(addr);
  if (!func || func->getParent() == module) {
    return func;
  }

  return nullptr;
}

// Return an already lifted trace starting with the code at address
// `addr`.
llvm::Function *TraceLifter::Impl::GetLiftedTraceDefinition(uint64_t addr) {
  auto func = manager.GetLiftedTraceDefinition(addr);
  if (!func || func->getParent() == module) {
    return func;
  }

  CHECK_EQ(&(func->getContext()), &context);

  auto func_type = llvm::dyn_cast<llvm::FunctionType>(
      remill::RecontextualizeType(func->getFunctionType(), context));

  // Handle the different module situation by declaring the trace in
  // this module to be external, with the idea that it will link to
  // another module.
  auto extern_func = module->getFunction(func->getName());
  if (!extern_func || extern_func->getFunctionType() != func_type) {
    extern_func = llvm::Function::Create(
        func_type, llvm::GlobalValue::ExternalLinkage, func->getName(), module);

  } else if (extern_func->isDeclaration()) {
    extern_func->setLinkage(llvm::GlobalValue::ExternalLinkage);
  }

  return extern_func;
}

TraceLifter::~TraceLifter(void) {}

TraceLifter::TraceLifter(const remill::Arch *arch_, TraceManager *manager_)
    : impl(new Impl(arch_, manager_)) {}

void TraceLifter::NullCallback(uint64_t, llvm::Function *) {}

// Reads the bytes of an instruction at `addr` into `inst_bytes`.
bool TraceLifter::Impl::ReadInstructionBytes(uint64_t addr) {
  inst_bytes.clear();
  for (size_t i = 0; i < max_inst_bytes; ++i) {
    const auto byte_addr = (addr + i) & addr_mask;
    if (byte_addr < addr) {
      break;  // 32- or 64-bit address overflow.
    }
    uint8_t byte = 0;
    if (!manager.TryReadExecutableByte(byte_addr, &byte)) {
      DLOG(WARNING) << "Couldn't read executable byte at " << std::hex
                    << byte_addr << std::dec;
      break;
    }
    inst_bytes.push_back(static_cast<char>(byte));
  }
  return !inst_bytes.empty();
}

// Lift one or more traces starting from `addr`.
bool TraceLifter::Lift(
    uint64_t addr, std::function<void(uint64_t, llvm::Function *)> callback) {
  return impl->Lift(addr, callback);
}

// Lift one or more traces starting from `addr`.
bool TraceLifter::Impl::Lift(
    uint64_t addr, std::function<void(uint64_t, llvm::Function *)> callback) {
  // Reset the lifting state.
  trace_work_list.clear();
  inst_work_list.clear();
  blocks.clear();
  inst_bytes.clear();
  func = nullptr;
  switch_inst = nullptr;
  block = nullptr;
  inst.Reset();
  
  // Get a trace head that the manager knows about, or that we
  // will eventually tell the trace manager about.
  auto get_trace_decl = [=](uint64_t trace_addr) -> llvm::Function * {
    if (auto trace = GetLiftedTraceDeclaration(trace_addr)) {
      return trace;
    } else if (trace_work_list.count(trace_addr)) {
      return arch->DeclareLiftedFunction(manager.TraceName(trace_addr), module);
    } else {
      return nullptr;
    }
  };

  trace_work_list.insert(addr);
  while (!trace_work_list.empty()) {
    const auto trace_addr = PopTraceAddress();

    // Already lifted.
    func = GetLiftedTraceDefinition(trace_addr);
    if (func) {
      continue;
    }

    DLOG(INFO) << "Lifting trace at address " << std::hex << trace_addr
               << std::dec;

    func = get_trace_decl(trace_addr);
    blocks.clear();

    if (!func || !func->isDeclaration()) {
      func = arch->DeclareLiftedFunction(manager.TraceName(trace_addr), module);
    }

    CHECK(func->isDeclaration());

    // Fill in the function, and make sure the block with all register
    // variables jumps to the block that will contain the first instruction
    // of the trace.
    arch->InitializeEmptyLiftedFunction(func);

    auto state_ptr = remill::NthArgument(func, remill::kStatePointerArgNum);

    if (auto entry_block = &(func->front())) {
      auto pc = remill::LoadProgramCounterArg(func);
      auto [next_pc_ref, next_pc_ref_type] =
          this->arch->DefaultLifter(*this->intrinsics)
              ->LoadRegAddress(entry_block, state_ptr, remill::kNextPCVariableName);

      // Initialize `NEXT_PC`.
      (void) new llvm::StoreInst(pc, next_pc_ref, entry_block);

      // Branch to the first basic block.
      llvm::BranchInst::Create(GetOrCreateBlock(trace_addr), entry_block);
    }

    CHECK(inst_work_list.empty());
    inst_work_list.insert(trace_addr);

    // Decode instructions.
    while (!inst_work_list.empty()) {
      const auto inst_addr = PopInstructionAddress();

      block = GetOrCreateBlock(inst_addr);
      switch_inst = nullptr;

      // We have already lifted this instruction block.
      if (!block->empty()) {
        continue;
      }

      // Check to see if this instruction corresponds with an existing
      // trace head, and if so, tail-call into that trace directly without
      // decoding or lifting the instruction.
      if (inst_addr != trace_addr) {
        if (auto inst_as_trace = get_trace_decl(inst_addr)) {
          AddTerminatingTailCall(block, inst_as_trace, *intrinsics);
          continue;
        }
      }

      // No executable bytes here.
      if (!ReadInstructionBytes(inst_addr)) {
        AddTerminatingTailCall(block, intrinsics->missing_block, *intrinsics);
        continue;
      }

      inst.Reset();

      // TODO(Ian): not passing context around in trace lifter
      std::ignore = arch->DecodeInstruction(inst_addr, inst_bytes, inst,
                                            this->arch->CreateInitialContext());

      auto lift_status =
          inst.GetLifter()->LiftIntoBlock(inst, block, state_ptr);
      if (remill::kLiftedInstruction != lift_status) {
        AddTerminatingTailCall(block, intrinsics->error, *intrinsics);
        continue;
      }

      // Connect together the basic blocks.
      switch (inst.category) {
        case remill::Instruction::kCategoryInvalid:
        case remill::Instruction::kCategoryError:
          AddTerminatingTailCall(block, intrinsics->error, *intrinsics);
          break;

        case remill::Instruction::kCategoryNormal:
        case remill::Instruction::kCategoryNoOp:
          llvm::BranchInst::Create(GetOrCreateNextBlock(), block);
          break;

        // Direct jumps could either be local or could be tail-calls. In the
        // case of a tail call, we'll assume that the trace manager contains
        // advanced knowledge of this, and so when we go to make a block for
        // the targeted instruction, we'll either tail call to the target
        // trace, or we'll just extend out the current trace. Either way, no
        // sacrifice in correctness is made.
        case remill::Instruction::kCategoryDirectJump:
          llvm::BranchInst::Create(GetOrCreateBranchTakenBlock(), block);
          break;

        case remill::Instruction::kCategoryIndirectJump: {
          AddTerminatingTailCall(block, intrinsics->jump, *intrinsics);
          break;
        }

        case remill::Instruction::kCategoryAsyncHyperCall:
          AddCall(block, intrinsics->async_hyper_call, *intrinsics);
          goto check_call_return;

        case remill::Instruction::kCategoryIndirectFunctionCall: {
          const auto fall_through_block =
              llvm::BasicBlock::Create(context, "", func);

          const auto ret_pc_ref =
              remill::LoadReturnProgramCounterRef(fall_through_block);
          const auto next_pc_ref =
              remill::LoadNextProgramCounterRef(fall_through_block);
          llvm::IRBuilder<> ir(fall_through_block);
          ir.CreateStore(ir.CreateLoad(word_type, ret_pc_ref), next_pc_ref);
          ir.CreateBr(GetOrCreateBranchNotTakenBlock());

          AddCall(block, intrinsics->function_call, *intrinsics);
          llvm::BranchInst::Create(fall_through_block, block);
          block = fall_through_block;
          continue;
        }

        case remill::Instruction::kCategoryConditionalIndirectFunctionCall: {
          auto taken_block = llvm::BasicBlock::Create(context, "", func);
          auto not_taken_block = GetOrCreateBranchNotTakenBlock();
          const auto orig_not_taken_block = not_taken_block;

          llvm::BranchInst::Create(taken_block, not_taken_block,
                                   remill::LoadBranchTaken(block), block);

          AddCall(taken_block, intrinsics->function_call, *intrinsics);

          const auto ret_pc_ref = remill::LoadReturnProgramCounterRef(taken_block);
          const auto next_pc_ref = remill::LoadNextProgramCounterRef(taken_block);
          llvm::IRBuilder<> ir(taken_block);
          ir.CreateStore(ir.CreateLoad(word_type, ret_pc_ref), next_pc_ref);
          ir.CreateBr(orig_not_taken_block);
          block = orig_not_taken_block;
          continue;
        }

        // In the case of a direct function call, we try to handle the
        // pattern of a call to the next PC as a way of getting access to
        // an instruction pointer. It is the case where a call to the next
        // PC could also be something more like a call to a `noreturn` function
        // and that is OK, because either a user of the trace manager has
        // already told us that the next PC is a trace head (and we'll pick
        // that up when trying to lift it), or we'll just have a really big
        // trace for this function without sacrificing correctness.
        case remill::Instruction::kCategoryDirectFunctionCall: {
        direct_func_call:
          if (inst.branch_not_taken_pc != inst.branch_taken_pc) {
            trace_work_list.insert(inst.branch_taken_pc);
            auto target_trace = get_trace_decl(inst.branch_taken_pc);
            AddCall(block, target_trace, *intrinsics);
          }

          const auto ret_pc_ref = remill::LoadReturnProgramCounterRef(block);
          const auto next_pc_ref = remill::LoadNextProgramCounterRef(block);
          llvm::IRBuilder<> ir(block);
          ir.CreateStore(ir.CreateLoad(word_type, ret_pc_ref), next_pc_ref);
          ir.CreateBr(GetOrCreateBranchNotTakenBlock());

          continue;
        }

        case remill::Instruction::kCategoryConditionalDirectFunctionCall: {
          if (inst.branch_not_taken_pc == inst.branch_taken_pc) {
            goto direct_func_call;
          }

          auto taken_block = llvm::BasicBlock::Create(context, "", func);
          auto not_taken_block = GetOrCreateBranchNotTakenBlock();
          const auto orig_not_taken_block = not_taken_block;

          llvm::BranchInst::Create(taken_block, not_taken_block,
                                   remill::LoadBranchTaken(block), block);

          trace_work_list.insert(inst.branch_taken_pc);
          auto target_trace = get_trace_decl(inst.branch_taken_pc);

          AddCall(taken_block, intrinsics->function_call, *intrinsics);
          AddCall(taken_block, target_trace, *intrinsics);

          const auto ret_pc_ref = remill::LoadReturnProgramCounterRef(taken_block);
          const auto next_pc_ref = remill::LoadNextProgramCounterRef(taken_block);
          llvm::IRBuilder<> ir(taken_block);
          ir.CreateStore(ir.CreateLoad(word_type, ret_pc_ref), next_pc_ref);
          ir.CreateBr(orig_not_taken_block);
          block = orig_not_taken_block;
          continue;
        }

        // Lift an async hyper call to check if it should do the hypercall.
        // If so, it will jump to the `do_hyper_call` block, otherwise it will
        // jump to the block associated with the next PC. In the case of the
        // `do_hyper_call` block, we assign it to `state.block`, then go
        // to `check_call_return` to add the hyper call into that block,
        // checking if the hyper call returns to the next PC or not.
        //
        // TODO(pag): Delay slots?
        case remill::Instruction::kCategoryConditionalAsyncHyperCall: {
          auto do_hyper_call = llvm::BasicBlock::Create(context, "", func);
          llvm::BranchInst::Create(do_hyper_call, GetOrCreateNextBlock(),
                                   remill::LoadBranchTaken(block), block);
          block = do_hyper_call;
          AddCall(block, intrinsics->async_hyper_call, *intrinsics);
          goto check_call_return;
        }

        check_call_return:
          do {
            auto pc = LoadProgramCounter(block, *intrinsics);
            auto ret_pc =
                llvm::ConstantInt::get(intrinsics->pc_type, inst.next_pc);

            llvm::IRBuilder<> ir(block);
            auto eq = ir.CreateICmpEQ(pc, ret_pc);
            auto unexpected_ret_pc =
                llvm::BasicBlock::Create(context, "", func);
            ir.CreateCondBr(eq, GetOrCreateNextBlock(), unexpected_ret_pc);
            AddTerminatingTailCall(unexpected_ret_pc, intrinsics->missing_block,
                                   *intrinsics);
          } while (false);
          break;

        case remill::Instruction::kCategoryFunctionReturn:
          AddTerminatingTailCall(block, intrinsics->function_return,
                                 *intrinsics);
          break;

        case remill::Instruction::kCategoryConditionalFunctionReturn: {
          auto taken_block = llvm::BasicBlock::Create(context, "", func);
          auto not_taken_block = GetOrCreateBranchNotTakenBlock();
          const auto orig_not_taken_block = not_taken_block;

          llvm::BranchInst::Create(taken_block, not_taken_block,
                                   remill::LoadBranchTaken(block), block);

          AddTerminatingTailCall(taken_block, intrinsics->function_return,
                                 *intrinsics);
          block = orig_not_taken_block;
          continue;
        }

        case remill::Instruction::kCategoryConditionalBranch: {
          auto taken_block = GetOrCreateBranchTakenBlock();
          auto not_taken_block = GetOrCreateBranchNotTakenBlock();

          llvm::BranchInst::Create(taken_block, not_taken_block,
                                   remill::LoadBranchTaken(block), block);
          break;
        }
        case remill::Instruction::kCategoryConditionalIndirectJump: {
          auto taken_block = llvm::BasicBlock::Create(context, "", func);
          auto not_taken_block = GetOrCreateBranchNotTakenBlock();
          const auto orig_not_taken_block = not_taken_block;

          llvm::BranchInst::Create(taken_block, not_taken_block,
                                   remill::LoadBranchTaken(block), block);

          AddTerminatingTailCall(taken_block, intrinsics->jump, *intrinsics);
          block = orig_not_taken_block;
          continue;
        }
      }
    }

    for (auto &block : *func) {
      if (!block.getTerminator()) {
        AddTerminatingTailCall(&block, intrinsics->missing_block, *intrinsics);
      }
    }

    callback(trace_addr, func);
    manager.SetLiftedTraceDefinition(trace_addr, func);
  }

  return true;
}
