/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LoweringPasses.h"
#include "hermes/IR/IRBuilder.h"

namespace hermes::sh {

namespace {

/// Check whether a particular operand of an instruction must stay
/// as literal and hence cannot be lowered into load_const instruction.
bool operandMustBeLiteral(Instruction *Inst, unsigned opIndex) {
  // HBCLoadConstInst is meant to load a constant
  if (llvh::isa<HBCLoadConstInst>(Inst))
    return true;

  // The operand of LoadParamInst is a literal index.
  if (llvh::isa<LoadParamInst>(Inst))
    return true;

  if (llvh::isa<HBCAllocObjectFromBufferInst>(Inst))
    return true;

  // All operands of AllocArrayInst are literals.
  if (llvh::isa<AllocArrayInst>(Inst))
    return true;

  if (llvh::isa<AllocFastArrayInst>(Inst))
    return true;

  if (llvh::isa<CacheNewObjectInst>(Inst))
    return opIndex >= CacheNewObjectInst::FirstKeyIdx;

  // SwitchInst's rest of the operands are case values,
  // hence they will stay as constant.
  if (llvh::isa<SwitchInst>(Inst) && opIndex > 0)
    return true;

  // DefineOwnPropertyInst and DefineNewOwnPropertyInst.
  if (auto *SOP = llvh::dyn_cast<BaseDefineOwnPropertyInst>(Inst)) {
    if (opIndex == BaseDefineOwnPropertyInst::PropertyIdx) {
      if (llvh::isa<DefineNewOwnPropertyInst>(Inst)) {
        // In DefineNewOwnPropertyInst the property name must be a literal.
        return true;
      }

      // If the propery is a LiteralNumber, the property is enumerable, and it
      // is a valid array index, it is coming from an array initialization and
      // we will emit it as DefineOwnByIndex.
      if (auto *LN = llvh::dyn_cast<LiteralNumber>(Inst->getOperand(opIndex))) {
        if (SOP->getIsEnumerable() && LN->convertToArrayIndex().hasValue())
          return true;
      }

      // LiteralStrings are optimized, when they are enumerable.
      if (llvh::isa<LiteralString>(Inst->getOperand(opIndex)) &&
          SOP->getIsEnumerable()) {
        return true;
      }
    }

    // DefineOwnPropertyInst's isEnumerable is a boolean constant.
    if (opIndex == DefineOwnPropertyInst::IsEnumerableIdx)
      return true;

    return false;
  }

  // If StorePropertyInst's property ID is a LiteralString, we will keep it
  // untouched and emit try_put_by_id eventually. Unless it is specifically a
  // StorePropertyWithReceiverInst. That instruction has no by_id variant.
  if (llvh::isa<BaseStorePropertyInst>(Inst) &&
      !llvh::isa<StorePropertyWithReceiverInst>(Inst) &&
      opIndex == BaseStorePropertyInst::PropertyIdx &&
      llvh::isa<LiteralString>(Inst->getOperand(opIndex)))
    return true;

  if (llvh::isa<StorePropertyWithReceiverInst>(Inst) &&
      opIndex == StorePropertyWithReceiverInst::IsStrictIdx) {
    return true;
  }

  // If LoadPropertyInst's property ID is a LiteralString, we will keep it
  // untouched and emit try_put_by_id eventually.
  if (llvh::isa<BaseLoadPropertyInst>(Inst) &&
      opIndex == BaseLoadPropertyInst::PropertyIdx &&
      llvh::isa<LiteralString>(Inst->getOperand(opIndex)))
    return true;

  // For properties that valid index-like literals, there's a get_by_index
  // variant that encodes the property as an immediate.
  if (auto *loadPropInst = llvh::dyn_cast_or_null<LoadPropertyInst>(Inst)) {
    if (opIndex == LoadPropertyInst::PropertyIdx) {
      if (auto *litNum =
              llvh::dyn_cast<LiteralNumber>(loadPropInst->getProperty());
          litNum && doubleToArrayIndex(litNum->getValue())) {
        return true;
      }
    }
  }

  // DefineOwnGetterSetterInst's isEnumerable is a boolean constant.
  if (llvh::isa<DefineOwnGetterSetterInst>(Inst) &&
      opIndex == DefineOwnGetterSetterInst::IsEnumerableIdx)
    return true;

  // Both pattern and flags operands of the CreateRegExpInst
  // are literal strings.
  if (llvh::isa<CreateRegExpInst>(Inst))
    return true;

  if (llvh::isa<SwitchImmInst>(Inst) &&
      (opIndex == SwitchImmInst::MinValueIdx ||
       opIndex == SwitchImmInst::SizeIdx ||
       opIndex >= SwitchImmInst::FirstCaseIdx))
    return true;

  /// CallBuiltin's callee, new.target, "this" should always be literals.
  if (llvh::isa<CallBuiltinInst>(Inst) &&
      (opIndex == CallBuiltinInst::CalleeIdx ||
       opIndex == CallBuiltinInst::NewTargetIdx ||
       opIndex == CallBuiltinInst::ThisIdx))
    return true;
  if (llvh::isa<BranchIfBuiltinInst>(Inst) &&
      opIndex == BranchIfBuiltinInst::BuiltinIdx)
    return true;

  /// GetBuiltinClosureInst's builtin index is always literal.
  if (llvh::isa<GetBuiltinClosureInst>(Inst) &&
      opIndex == GetBuiltinClosureInst::BuiltinIndexIdx)
    return true;

  if (llvh::isa<IteratorCloseInst>(Inst) &&
      opIndex == IteratorCloseInst::IgnoreInnerExceptionIdx) {
    return true;
  }

  if (llvh::isa<DeclareGlobalVarInst>(Inst) &&
      opIndex == DeclareGlobalVarInst::NameIdx) {
    return true;
  }

  if (llvh::isa<DirectEvalInst>(Inst) &&
      opIndex == DirectEvalInst::StrictCallerIdx) {
    return true;
  }

  if (llvh::isa<GetTemplateObjectInst>(Inst) &&
      (opIndex == GetTemplateObjectInst::TemplateObjIDIdx ||
       opIndex == GetTemplateObjectInst::DupIdx)) {
    return true;
  }

  if (llvh::isa<PrLoadInst>(Inst) &&
      (opIndex == PrLoadInst::PropIndexIdx ||
       opIndex == PrLoadInst::PropNameIdx)) {
    return true;
  }
  if (llvh::isa<PrStoreInst>(Inst) &&
      (opIndex == PrStoreInst::PropIndexIdx ||
       opIndex == PrStoreInst::PropNameIdx ||
       opIndex == PrStoreInst::NonPointerIdx)) {
    return true;
  }

  if (llvh::isa<NativeCallInst>(Inst) &&
      (opIndex == NativeCallInst::CalleeIdx ||
       opIndex == NativeCallInst::SignatureIdx)) {
    return true;
  }

  if (llvh::isa<CheckedTypeCastInst>(Inst) &&
      opIndex == CheckedTypeCastInst::SpecifiedTypeIdx) {
    return true;
  }
  if (llvh::isa<ThrowIfInst>(Inst) && opIndex == ThrowIfInst::InvalidTypesIdx) {
    return true;
  }

  if (llvh::isa<LIRResolveScopeInst>(Inst) &&
      opIndex == LIRResolveScopeInst::NumLevelsIdx) {
    return true;
  }

  if (llvh::isa<TypeOfIsInst>(Inst) && opIndex == TypeOfIsInst::TypesIdx) {
    return true;
  }

  if (llvh::isa<BaseCallInst>(Inst) &&
      opIndex == BaseCallInst::CalleeIsAlwaysClosure) {
    return true;
  }

  if (const auto *callInst = llvh::dyn_cast<BaseCallInst>(Inst)) {
    if (callInst->getAttributes(Inst->getModule()).isMetroRequire &&
        opIndex == callInst->getThisIdx() + 1) {
      return true;
    }
  }

  return false;
}

bool loadConstants(Function *F) {
  IRBuilder builder(F);
  bool changed = false;

  /// Inserts and returns a load instruction for \p literal before \p where.
  auto createLoadLiteral = [&builder](Literal *literal, Instruction *where) {
    builder.setInsertionPoint(where);
    return llvh::isa<GlobalObject>(literal)
        ? cast<Instruction>(builder.createHBCGetGlobalObjectInst())
        : cast<Instruction>(builder.createHBCLoadConstInst(literal));
  };

  for (BasicBlock &BB : *F) {
    for (auto &I : BB) {
      if (auto *phi = llvh::dyn_cast<PhiInst>(&I)) {
        // Since PhiInsts must always be at the start of a basic block, we have
        // to insert the load instruction in the predecessor. This lowering is
        // sub-optimal: for conditional branches, the load constant operation
        // will be performed before the branch decides which path to take.
        for (unsigned i = 0, e = phi->getNumEntries(); i < e; ++i) {
          auto [val, bb] = phi->getEntry(i);
          if (auto *literal = llvh::dyn_cast<Literal>(val)) {
            auto *load = createLoadLiteral(literal, bb->getTerminator());
            phi->updateEntry(i, load, bb);
            changed = true;
          }
        }
        continue;
      }
      // For all other instructions, insert load constants right before the they
      // are needed. This minimizes their live range and therefore reduces
      // register pressure. CodeMotion and CSE can later hoist and deduplicate
      // them.
      for (unsigned i = 0, e = I.getNumOperands(); i < e; ++i) {
        if (auto *literal = llvh::dyn_cast<Literal>(I.getOperand(i))) {
          if (!operandMustBeLiteral(&I, i)) {
            auto *load = createLoadLiteral(literal, &I);
            I.setOperand(load, i);
            changed = true;
          }
        }
      }
    }
  }
  return changed;
}

} // namespace

Pass *createLoadConstants() {
  class ThisPass : public FunctionPass {
   public:
    explicit ThisPass() : FunctionPass("LoadConstants") {}
    bool runOnFunction(Function *F) override {
      return loadConstants(F);
    }
  };
  return new ThisPass();
}

} // namespace hermes::sh
