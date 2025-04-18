/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_IR_IRVISITOR_H
#define HERMES_IR_IRVISITOR_H

#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"

namespace hermes {

/// IRVisitorBase - This is a simple visitor class for Hermes IR nodes, allowing
/// clients to walk over entire IR functions, blocks, or instructions.
template <typename ImplClass, typename ValueRetTy = void>
class IRVisitorBase {
 public:
  ImplClass &asImpl() {
    return static_cast<ImplClass &>(*this);
  }

  /// Top level visitor.
  ValueRetTy visitValue(const Value &V) {
    return ValueRetTy();
  }

/// Define default IR implementations, chaining to parent nodes.
/// Use ValueKinds.def to automatically generate them.
#define DEF_VALUE(CLASS, PARENT)            \
  ValueRetTy visit##CLASS(const CLASS &I) { \
    return asImpl().visit##PARENT(I);       \
  }
#define MARK_FIRST(CLASS, PARENT) DEF_VALUE(CLASS, PARENT)
#define BEGIN_VALUE(CLASS, PARENT) DEF_VALUE(CLASS, PARENT)
#define DEF_TAG(NAME, PARENT)               \
  ValueRetTy visit##NAME(const PARENT &I) { \
    return asImpl().visit##PARENT(I);       \
  }
#include "hermes/IR/ValueKinds.def"
};

/// IRVisitor - This is a simple visitor class for Hermes IR nodes.
/// allowing clients to walk over hermes IRNodes of different types.
template <typename ImplClass, typename ValueRetTy = void>
class IRVisitor : public IRVisitorBase<ImplClass, ValueRetTy> {
 public:
  ImplClass &asImpl() {
    return static_cast<ImplClass &>(*this);
  }

  /// Visit IR nodes.
  ValueRetTy visit(const Value &V) {
    switch (V.getKind()) {
      default:
        llvm_unreachable("Invalid kind");
#define DEF_VALUE(CLASS, PARENT) \
  case ValueKind::CLASS##Kind:   \
    return asImpl().visit##CLASS(*llvh::cast<CLASS>(&V));
#define DEF_TAG(NAME, PARENT) \
  case ValueKind::NAME##Kind: \
    return asImpl().visit##PARENT(*llvh::cast<PARENT>(&V));
#include "hermes/IR/ValueKinds.def"
    }
    llvm_unreachable("Not reachable, all cases handled");
  }
};

/// InstrVisitor - This is a simple visitor class for Hermes IR instructions,
/// allowing clients to walk over hermes Instructions of different types.
template <typename ImplClass, typename ValueRetTy = void>
class InstructionVisitor : public IRVisitorBase<ImplClass, ValueRetTy> {
 public:
  ImplClass &asImpl() {
    return static_cast<ImplClass &>(*this);
  }

  // Perform any required pre-processing before visiting.
  // Sub-classes can reimplement it to provide their custom
  // pre-processing steps.
  void beforeVisitInstruction(const Instruction &V) {}

  /// Visit Instruction.
  ValueRetTy visit(const Instruction &Inst) {
    asImpl().beforeVisitInstruction(Inst);

    switch (Inst.getKind()) {
      default:
        llvm_unreachable("Invalid kind");
#define DEF_VALUE(CLASS, PARENT) \
  case ValueKind::CLASS##Kind:   \
    return asImpl().visit##CLASS(*llvh::cast<CLASS>(&Inst));
#define DEF_TAG(NAME, PARENT) \
  case ValueKind::NAME##Kind: \
    return asImpl().visit##NAME(*llvh::cast<PARENT>(&Inst));
#include "hermes/IR/Instrs.def"
    }
    llvm_unreachable("Not reachable, all cases handled");
  }
};

} // end namespace hermes

#endif
