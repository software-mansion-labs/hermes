/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "tdzdedup"
#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/DenseSet.h"

STATISTIC(NumTDZDedup, "Number of TDZ instructions eliminated");

namespace hermes {

namespace {

class TDZDedupContext;

using ScopedHTType = hermes::ScopedHashTable<Value *, bool>;
using ScopeType = hermes::ScopedHashTableScope<Value *, bool>;

// StackNode - contains all the needed information to create a stack for doing
// a depth first traversal of the tree. This includes scopes for values and
// loads as well as the generation. There is a child iterator so that the
// children do not need to be store spearately.
class StackNode : public DomTreeDFS::StackNode {
 public:
  inline StackNode(TDZDedupContext *ctx, const DominanceInfoNode *n);

 private:
  /// RAII to create and pop a scope when the stack node is created and
  /// destroyed.
  ScopeType scope_;
};

/// TDZDedupContext - This pass does a simple depth-first walk of the dominator
/// tree, eliminating trivially redundant instructions.
class TDZDedupContext : public DomTreeDFS::Visitor<TDZDedupContext, StackNode> {
 public:
  TDZDedupContext(Function *F, DominanceInfo &DT)
      : DomTreeDFS::Visitor<TDZDedupContext, StackNode>(DT),
        F_(F),
        builder_(F) {}

  bool run();

  bool processNode(StackNode *SN);

 private:
  friend StackNode;

  Function *const F_;
  IRBuilder builder_;

  /// All TDZ state variables are collected here.
  llvh::DenseSet<Value *> tdzState_{};

  /// AvailableValues - This scoped hash table conceptually contains the
  /// TDZ-obeying values and whether they are known to be non-empty. It maps
  /// from instances of Variable, AllocStackInst, or theoretically an
  /// Instruction * if the value has been SSA-ed, to a boolean. True means the
  /// value is known to be non-empty, false means it could be empty.
  ///
  /// Values can transition between these states as we walk the dominator tree.
  /// Due to the recursive nature of the dominator walk, we must be careful
  /// how we update them. If he value is in the current scope, we update its
  /// value in-place. Otherwise, we insert a new key/value in the current scope.
  ///
  /// We eliminate checks to values that are known to be non-empty at the time.
  ScopedHTType availableValues_{};
};

inline StackNode::StackNode(TDZDedupContext *ctx, const DominanceInfoNode *n)
    : DomTreeDFS::StackNode(n), scope_{ctx->availableValues_} {}

bool TDZDedupContext::run() {
  // First, collect all TDZ state variables.
  for (auto &BB : *F_) {
    for (auto &I : BB) {
      auto *TIU = llvh::dyn_cast<ThrowIfInst>(&I);
      if (!TIU)
        continue;

      Value *checkedValue = TIU->getCheckedValue();
      Value *tdzStorage;

      if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(checkedValue)) {
        tdzStorage = LFI->getLoadVariable();
      } else if (auto *LSI = llvh::dyn_cast<LoadStackInst>(checkedValue)) {
        tdzStorage = LSI->getSingleOperand();
      } else {
        continue;
      }

      tdzState_.insert(tdzStorage);
    }
  }

  return DFS();
}

bool TDZDedupContext::processNode(StackNode *SN) {
  BasicBlock *BB = SN->node()->getBlock();
  bool changed = false;

  // Keep a list of instructions that should be deleted when the basic block
  // is processed.
  IRBuilder::InstructionDestroyer destroyer;

  // See if any instructions in the block can be eliminated.  If so, do it.  If
  // not, add them to AvailableValues.
  for (auto &inst : *BB) {
    // The storage containing the value that can potentially be empty.
    Value *tdzStorage = nullptr;
    ThrowIfInst *TIE = nullptr;
    if ((TIE = llvh::dyn_cast<ThrowIfInst>(&inst)) != nullptr) {
      auto *checkedValue = TIE->getCheckedValue();

      if (auto *LFI = llvh::dyn_cast<LoadFrameInst>(checkedValue)) {
        tdzStorage = LFI->getLoadVariable();
      } else if (auto *LSI = llvh::dyn_cast<LoadStackInst>(checkedValue)) {
        tdzStorage = LSI->getSingleOperand();
      } else {
        tdzStorage = checkedValue;
      }
    } else if (auto *SF = llvh::dyn_cast<StoreFrameInst>(&inst)) {
      tdzStorage = SF->getVariable();
      // Is the target a TDZ state variable?
      if (!tdzState_.count(tdzStorage))
        continue;
      // Check whether it is setting the target to empty, in which case we
      // mark it is "unavailable".
      if (SF->getValue()->getType().canBeEmpty()) {
        availableValues_.setInCurrentScope(tdzStorage, false);
        continue;
      }
    } else if (auto *SS = llvh::dyn_cast<StoreStackInst>(&inst)) {
      tdzStorage = SS->getPtr();
      // Is the target a TDZ state variable?
      if (!tdzState_.count(tdzStorage))
        continue;
      // Check whether it is setting the target to non-empty.
      if (SS->getValue()->getType().canBeEmpty()) {
        availableValues_.setInCurrentScope(tdzStorage, false);
        continue;
      }
    } else {
      continue;
    }

    // If the tdz state is not already known to be set to true, add it to
    // the map.
    if (!availableValues_.lookup(tdzStorage)) {
      availableValues_.setInCurrentScope(tdzStorage, true);
      continue;
    }

    // Handle only ThrowIf from here on.
    if (!TIE)
      continue;

    // The TDZ state is known to be true, so we can eliminate the check
    // instruction.
    destroyer.add(TIE);
    changed = true;
    ++NumTDZDedup;

    // If ThrowIf has no users, we will attempt to destroy the load too, to
    // save work in other passes.
    if (!TIE->hasUsers()) {
      if ((llvh::isa<LoadFrameInst>(TIE->getCheckedValue()) ||
           llvh::isa<LoadStackInst>(TIE->getCheckedValue())) &&
          TIE->getCheckedValue()->hasOneUser()) {
        destroyer.add(llvh::cast<Instruction>(TIE->getCheckedValue()));
      }
    } else {
      builder_.setInsertionPoint(TIE);
      builder_.setLocation(TIE->getLocation());
      auto *cast = builder_.createUnionNarrowTrustedInst(
          TIE->getCheckedValue(), TIE->getType());
      TIE->replaceAllUsesWith(cast);
    }
  }

  return changed;
}

} // end anonymous namespace

Pass *createTDZDedup() {
  class TDZDedup : public FunctionPass {
   public:
    explicit TDZDedup() : FunctionPass("TDZDedup") {}
    ~TDZDedup() override = default;

    bool runOnFunction(Function *F) override {
      DominanceInfo DT{F};
      TDZDedupContext CCtx{F, DT};
      return CCtx.run();
    }
  };
  return new TDZDedup();
}

} // namespace hermes

#undef DEBUG_TYPE
