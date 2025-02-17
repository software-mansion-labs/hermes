/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define DEBUG_TYPE "hoiststartgenerator"

#include "hermes/Optimizer/Scalar/HoistStartGenerator.h"

#include "hermes/IR/Instrs.h"

using namespace hermes;

bool HoistStartGenerator::runOnFunction(Function *F) {
  // StartGenerator is only in unlowered inner generator functions.
  if (F->getParent()->areGeneratorsLowered()) {
    return false;
  }
  if (!F->isInnerGenerator()) {
    return false;
  }

  for (BasicBlock &bb : *F) {
    for (Instruction &inst : bb) {
      if (auto *startGen = llvh::dyn_cast<StartGeneratorInst>(&inst)) {
        startGen->moveBefore(&*F->front().begin());
        // GeneratorInnerFunction may only have one StartGeneratorInst,
        // so we are done.
        return true;
      }
    }
  }

  return false;
}

Pass *hermes::createHoistStartGenerator() {
  return new HoistStartGenerator();
}

#undef DEBUG_TYPE
