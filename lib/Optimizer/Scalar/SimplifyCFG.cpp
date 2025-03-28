/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "simplifycfg"

#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/IREval.h"
#include "hermes/IR/IRUtils.h"
#include "hermes/Optimizer/PassManager/Pass.h"
#include "hermes/Optimizer/Scalar/Utils.h"
#include "hermes/Support/Statistic.h"

#include "llvh/ADT/SmallPtrSet.h"
#include "llvh/ADT/SmallVector.h"

using namespace hermes;

STATISTIC(NumSB, "Number of static branches simplified");

/// \returns true if the block \b BB is an input to a PHI node.
static bool isUsedInPhiNode(BasicBlock *BB) {
  for (auto use : BB->getUsers())
    if (llvh::isa<PhiInst>(use))
      return true;

  return false;
}

static void removeEntryFromPhi(BasicBlock *BB, BasicBlock *edge) {
  // For all PHI nodes in block:
  for (auto &it : *BB) {
    auto *P = llvh::dyn_cast<PhiInst>(&it);
    if (!P)
      continue;
    // For each Phi entry:
    for (int i = 0, e = P->getNumEntries(); i < e; i++) {
      auto E = P->getEntry(i);
      // Remove the incoming edge.
      if (E.second == edge) {
        P->removeEntry(i);
        break;
      }
    }
  }
}

/// Delete the conditional branch and create a new direct branch to the
/// destination block \p dest.
static void replaceCondBranchWithDirectBranch(
    CondBranchInst *CB,
    BasicBlock *dest) {
  BasicBlock *currentBlock = CB->getParent();
  auto *trueDest = CB->getTrueDest();
  auto *falseDest = CB->getFalseDest();

  if (trueDest != dest)
    removeEntryFromPhi(trueDest, currentBlock);
  if (falseDest != dest)
    removeEntryFromPhi(falseDest, currentBlock);

  IRBuilder builder(currentBlock->getParent());
  builder.setInsertionBlock(currentBlock);
  builder.createBranchInst(dest);
  CB->eraseFromParent();
}

/// Try to remove a branch used by phi nodes.
static bool attemptBranchRemovalFromPhiNodes(BasicBlock *BB) {
  // Only handle blocks that are a single, unconditional branch.
  if (BB->getTerminator() != &*(BB->begin()) ||
      BB->getTerminator()->getKind() != ValueKind::BranchInstKind) {
    return false;
  }

  // Find our parents and also ensure that there aren't
  // any instructions we can't handle.
  llvh::SmallPtrSet<BasicBlock *, 8> blockParents;
  // Keep unique parents by the original order, which is deterministic.
  llvh::SmallVector<BasicBlock *, 8> orderedParents;
  for (const auto *user : BB->getUsers()) {
    switch (user->getKind()) {
      case ValueKind::BranchInstKind:
      case ValueKind::CondBranchInstKind:
      case ValueKind::SwitchInstKind:
      case ValueKind::GetPNamesInstKind:
      case ValueKind::GetNextPNameInstKind:
        // This is an instruction where the branch argument is a simple
        // jump target that can be substituted for any other branch.
        if (blockParents.count(user->getParent()) == 0) {
          orderedParents.push_back(user->getParent());
        }
        blockParents.insert(user->getParent());
        break;
      case ValueKind::PhiInstKind:
        // The branch argument is not a jump target, but we know how
        // to rewrite them.
        break;
      default:
        // This is some other instruction where we don't know whether we can
        // unconditionally substitute another branch. Bail for safety.
        return false;
    }
  }

  if (blockParents.empty()) {
    return false;
  }

  BasicBlock *phiBlock = nullptr;

  // Verify that we'll be able to rewrite all relevant Phi nodes.
  for (auto *user : BB->getUsers()) {
    if (auto *phi = llvh::dyn_cast<PhiInst>(user)) {
      if (phiBlock && phi->getParent() != phiBlock) {
        // We have PhiInsts in multiple blocks referencing BB, but BB is a
        // single static branch. This is invalid, but the bug is elsewhere.
        llvm_unreachable(
            "Different BBs use Phi for BB with single static jump");
      }
      phiBlock = phi->getParent();

      Value *ourValue = nullptr;
      for (unsigned int i = 0; i < phi->getNumEntries(); i++) {
        auto entry = phi->getEntry(i);
        if (entry.second == BB) {
          if (ourValue) {
            // The incoming phi node is invalid. The problem is not here.
            llvm_unreachable("Phi node has multiple entries for a branch.");
          }
          ourValue = entry.first;
        }
      }

      if (!ourValue) {
        llvm_unreachable("getUsers returned a non-user PhiInst");
      }

      for (unsigned int i = 0; i < phi->getNumEntries(); i++) {
        auto entry = phi->getEntry(i);
        if (blockParents.count(entry.second)) {
          // We have a PhiInst referencing our block BB and its parent, e.g.
          // %BB0:
          // CondBranchInst %1, %BB1, %BB2
          // %BB1:
          // BranchInst %BB2
          // %BB2:
          // PhiInst ??, %BB0, ??, %BB1
          if (entry.first == ourValue) {
            // Fortunately, the two values are equal, so we can rewrite to:
            // PhiInst ??, %BB0
          } else {
            // Unfortunately, the value is different in each case,
            // which naively would have led to an invalid rewrite like:
            // PhiInst %1, %BB0, %2, %BB0
            return false;
          }
        }
      }
    }
  }
  if (!phiBlock) {
    llvm_unreachable("We shouldn't be in this function if there are no Phis.");
  }

  // This branch is removable. Start by rewriting the Phi nodes.
  for (auto *user : BB->getUsers()) {
    if (auto *phi = llvh::dyn_cast<PhiInst>(user)) {
      Value *ourValue = nullptr;

      const unsigned int numEntries = phi->getNumEntries();
      for (unsigned int i = 0; i < numEntries; i++) {
        auto entry = phi->getEntry(i);
        if (entry.second == BB) {
          ourValue = entry.first;
        }
      }
      assert(ourValue && "getUsers returned a non-user PhiInst");

      for (int i = phi->getNumEntries() - 1; i >= 0; i--) {
        auto pair = phi->getEntry(i);
        if (pair.second == BB || blockParents.count(pair.second)) {
          phi->removeEntry(i);
        }
      }

      // Add parents back in sorted order to avoid any non-determinism
      for (BasicBlock *parent : orderedParents) {
        phi->addEntry(ourValue, parent);
      }
    }
  }
  // We verified earlier that all uses are branches and phis, so now that
  // we've rewritten the phis, we can have all branches jump there directly.
  BB->replaceAllUsesWith(phiBlock);
  BB->eraseFromParent();
  return true;
}

/// Perform a strict quality check on two literals. Literals are uniqued by
/// type, so we can just compare pointers, except for numbers, where we need to
/// perform a numeric comparison to ensure that NaNs and -0 are handled.
static bool literalStrictEquality(Literal *L1, Literal *L2) {
  if (llvh::isa<LiteralNumber>(L1) && llvh::isa<LiteralNumber>(L2)) {
    return llvh::cast<LiteralNumber>(L1)->getValue() ==
        llvh::cast<LiteralNumber>(L2)->getValue();
  }
  return L1 == L2;
}

/// Remove switch targets that are known to be unreachable via the switch
/// due to \p SI having a literal operand.
static bool simplifySwitchInst(SwitchInst *SI) {
  auto *thisBlock = SI->getParent();
  IRBuilder builder(thisBlock->getParent());
  builder.setInsertionBlock(thisBlock);

  Value *input = SI->getInputValue();
  auto *litInput = llvh::dyn_cast<Literal>(input);

  // If input of switch is not literal, nothing can be done.
  if (!litInput) {
    return false;
  }

  auto *destination = SI->getDefaultDestination();

  for (unsigned i = 0, e = SI->getNumCasePair(); i < e; i++) {
    auto switchCase = SI->getCasePair(i);

    // Look for a case which matches input.
    if (literalStrictEquality(switchCase.first, litInput)) {
      destination = switchCase.second;
      break;
    }
  }

  // Rewrite all phi nodes that no longer have incoming arrows from this block.
  for (unsigned i = 0, e = SI->getNumSuccessors(); i < e; i++) {
    auto *successor = SI->getSuccessor(i);
    if (successor == destination)
      continue;
    deleteIncomingBlockFromPhis(successor, thisBlock);
  }

  auto *newBranch = builder.createBranchInst(destination);
  SI->replaceAllUsesWith(newBranch);
  SI->eraseFromParent();
  return true;
}

/// Get rid of trampolines and merge basic blocks that are split by static
/// non-conditional branches.
static bool optimizeStaticBranches(Function *F) {
  bool changed = false;
  IRBuilder builder(F);

  // Remove conditional branches with a constant condition.
  for (auto &it : *F) {
    BasicBlock *BB = &it;

    // Handle SwitchInst, which can have a static branch and allow us to remove
    // multiple arms.
    if (auto *switchInst = llvh::dyn_cast<SwitchInst>(BB->getTerminator())) {
      changed |= simplifySwitchInst(switchInst);
      continue;
    }

    auto *cbr = llvh::dyn_cast<CondBranchInst>(BB->getTerminator());
    if (!cbr)
      continue;

    BasicBlock *trueDest = cbr->getTrueDest();
    BasicBlock *falseDest = cbr->getFalseDest();

    // If both sides of the branch point to the same block then just jump to it
    // directly.
    if (trueDest == falseDest) {
      replaceCondBranchWithDirectBranch(cbr, trueDest);
      changed = true;
      ++NumSB;
      continue;
    }

    // If the condition is optimized to a literal bool then replace the branch
    // with a non-conditional branch.
    auto *cond = cbr->getCondition();
    BasicBlock *dest = nullptr;

    if (LiteralBool *B = evalToBoolean(builder, cond)) {
      if (B->getValue()) {
        dest = trueDest;
      } else {
        dest = falseDest;
      }
    }

    if (dest != nullptr) {
      replaceCondBranchWithDirectBranch(cbr, dest);
      ++NumSB;
      changed = true;
      continue;
    }
  } // for all blocks.

  // Check if a basic block is a simple trampoline (empty non-conditional branch
  // to another basic block) and get rid of it. Replace all uses of the current
  // block with the destination of this block.
  for (auto &it : *F) {
    BasicBlock *BB = &it;
    auto *br = llvh::dyn_cast<BranchInst>(BB->getTerminator());
    if (!br)
      continue;

    BasicBlock *dest = br->getBranchDest();

    // Don't try to optimize infinite loops or unreachable blocks.
    if (dest == BB)
      continue;

    // NOTE: we know that this edge cannot go across any catch regions because
    // only TryEndInst can leave a try block.

    // Handle branches used in phi nodes specially.
    if (isUsedInPhiNode(BB)) {
      if (attemptBranchRemovalFromPhiNodes(BB)) {
        ++NumSB;
        changed = true;
        break; // Iterator invalidated.
      }
      continue;
    }

    // Check if the terminator is the only instruction in the block.
    bool isSingleInstr = (&*BB->begin() == br);

    // If the first and only instruction is a static branch, and it does not
    // cross a catch boundary then redirect all predecessors to the destination.
    if (isSingleInstr && !pred_empty(BB)) {
      BB->replaceAllUsesWith(dest);
      ++NumSB;
      changed = true;
      assert(pred_count(BB) == 0);
      continue;
    }

    // If the source block is not empty then try to slurp the destination block
    // and eliminate it altogether.
    if (pred_count(dest) == 1 && dest != BB) {
      // Slurp the instructions from the destination block one by one.
      while (dest->begin() != dest->end()) {
        dest->begin()->moveBefore(br);
      }

      // Now that we moved all of the instructions from the destination block we
      // can delete the original terminator and delete the destination block.
      dest->replaceAllUsesWith(BB);
      br->eraseFromParent();
      dest->eraseFromParent();
      ++NumSB;
      changed = true;

      // We are invalidating the iterator here. Stop the scan and continue
      // afresh in the next iteration.
      break;
    }
  } // for all blocks.

  return changed;
}

/// Insert an UnreachableInst after calls to noReturn functions.
/// Update Phis so that they reference the newly inserted block.
static bool terminateNoReturnCalls(Function *F) {
  Module *M = F->getParent();
  bool changed = false;
  IRBuilder builder(F);

  // Adding to the BasicBlock list by splitting as we iterate.
  for (auto bbit = F->begin(); bbit != F->end(); ++bbit) {
    BasicBlock *BB = &*bbit;
    for (auto it = BB->begin(), e = BB->end(); it != e; ++it) {
      Instruction *inst = &*it;
      CallInst *CI = llvh::dyn_cast<CallInst>(inst);
      if (!CI)
        continue;
      Function *target = llvh::dyn_cast<Function>(CI->getTarget());
      if (!target)
        continue;
      if (!target->getAttributes(M).noReturn)
        continue;

      // Skip call instruction.
      ++it;

      // Split block after call instruction.
      splitBasicBlock(BB, it);
      builder.setInsertionBlock(BB);
      builder.createUnreachableInst();
      changed = true;
      break;
    }
  }

  return changed;
}

static bool runSimplifyCFG(Module *M) {
  bool changed = false;

  for (auto &F : *M) {
    if (F.getAttributes(M).unreachable) {
      replaceBodyWithUnreachable(&F);
      changed = true;
      continue;
    }

    if (terminateNoReturnCalls(&F)) {
      changed = true;
      deleteUnreachableBasicBlocks(&F);
    }

    // Keep iterating over deleting unreachable code and removing trampolines as
    // long as we are making progress.
    bool iterChanged = false;
    do {
      iterChanged =
          optimizeStaticBranches(&F) || deleteUnreachableBasicBlocks(&F);
      changed |= iterChanged;
    } while (iterChanged);
  }

  changed |= deleteUnusedFunctionsAndVariables(M);
  return changed;
}

Pass *hermes::createSimplifyCFG() {
  class ThisPass : public ModulePass {
   public:
    explicit ThisPass() : ModulePass("SimplifyCFG") {}
    ~ThisPass() override = default;
    bool runOnModule(Module *M) override {
      return runSimplifyCFG(M);
    }
  };
  return new ThisPass();
}

#undef DEBUG_TYPE
