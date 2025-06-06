/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "auditor"

#include "hermes/Optimizer/Scalar/Auditor.h"
#include "hermes/IR/Analysis.h"
#include "hermes/IR/CFG.h"
#include "hermes/Support/Statistic.h"

#include "llvh/Support/Debug.h"

using namespace hermes;
using llvh::dbgs;

STATISTIC(
    CallsCreate,
    "Number of call instructions: callee obtained from definition");
STATISTIC(
    CallsProp,
    "Number of call instructions: callee obtained from prop lookup");
STATISTIC(
    CallsFrame,
    "Number of call instructions: callee obtained from frame");
STATISTIC(
    CallsStack,
    "Number of call instructions: callee obtained from stack");
STATISTIC(CallsPhi, "Number of call instructions: callee obtained from phi");
STATISTIC(
    CallsParameter,
    "Number of call instructions: callee obtained from a parameter");
STATISTIC(
    CallsCall,
    "Number of call instructions: callee obtained from a call");
STATISTIC(
    CallsOther,
    "Number of call instructions: callee obtained in other ways");
STATISTIC(Calls, "Number of call instructions of all kinds");
STATISTIC(Functions, "Number of functions");

STATISTIC(TypeUndefined, "Number of instructions with type undefined");
STATISTIC(TypeNull, "Number of instructions with type null");
STATISTIC(TypeBool, "Number of instructions with type boolean");
STATISTIC(TypeString, "Number of instructions with type string");
STATISTIC(TypeNumber, "Number of instructions with type number");
STATISTIC(TypeObject, "Number of instructions with type object");
STATISTIC(TypeAny, "Number of instructions with type any");
STATISTIC(TypeOther, "Number of instructions with type other");

static void auditCallInstructions(Function *F) {
  for (BasicBlock &BB : *F) {
    for (Instruction &II : BB) {
      if (II.getKind() == ValueKind::CallInstKind) {
        auto *CI = cast<CallInst>(&II);
        Value *callee = CI->getCallee();

        Calls += 1;

        switch (callee->getKind()) {
          case ValueKind::CreateFunctionInstKind:
            CallsCreate += 1;
            break;
          case ValueKind::LoadPropertyInstKind:
            CallsProp += 1;
            break;
          case ValueKind::LoadFrameInstKind: {
            CallsFrame += 1;
          } break;
          case ValueKind::LoadStackInstKind:
            CallsStack += 1;
            break;
          case ValueKind::PhiInstKind:
            CallsPhi += 1;
            break;
          case ValueKind::JSDynamicParamKind:
            CallsParameter += 1;
            break;
          case ValueKind::CallInstKind:
            CallsCall += 1;
            break;
          default:
            // Other cases; for example, property enumeration.
            CallsOther += 1;
            break;
        }
      }
    }
  }
}

static void auditInferredTypes(Function *F) {
  for (BasicBlock &BB : *F) {
    for (Instruction &II : BB) {
      Type t = II.getType();

      if (t.isUndefinedType()) {
        TypeUndefined++;
      } else if (t.isNullType()) {
        TypeNull++;
      } else if (t.isBooleanType()) {
        TypeBool++;
      } else if (t.isStringType()) {
        TypeString++;
      } else if (t.isNumberType()) {
        TypeNumber++;
      } else if (t.isObjectType()) {
        TypeObject++;
      } else if (t.canBeAny()) {
        TypeAny++;
      } else {
        // Other cases not counted above, e.g. union types.
        TypeOther++;
      }
    }
  }
}

bool Auditor::runOnFunction(Function *F) {
  LLVM_DEBUG(dbgs() << "Auditing calls in " << F->getInternalNameStr() << "\n");

  auditCallInstructions(F);

  LLVM_DEBUG(
      dbgs() << "Auditing instruction return types in "
             << F->getInternalNameStr() << "\n");

  auditInferredTypes(F);

  Functions++;

  return false;
}

Pass *hermes::createAuditor() {
  return new Auditor();
}

#undef DEBUG_TYPE
