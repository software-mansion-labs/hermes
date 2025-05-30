/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/IRVerifier.h"
#include "hermes/AST/Context.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/IRBuilder.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Utils/Dumper.h"

#include "gtest/gtest.h"

using llvh::errs;

using namespace hermes;

namespace {

// The verifier is only enabled if HERMES_SLOW_DEBUG is enabled
#ifdef HERMES_SLOW_DEBUG

TEST(IRVerifierTest, BasicBlockTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "forEach", Function::DefinitionKind::ES5Function, true);
  auto Arg1 = Builder.createJSDynamicParam(F, "num");
  auto Arg2 = Builder.createJSDynamicParam(F, "value");

  auto Entry = Builder.createBasicBlock(F);
  auto Loop = Builder.createBasicBlock(F);
  auto Body = Builder.createBasicBlock(F);
  auto Exit = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Entry);
  Builder.createBranchInst(Loop);

  Builder.setInsertionBlock(Loop);
  Builder.createCondBranchInst(Arg1, Body, Exit);

  Builder.setInsertionBlock(Body);
  Builder.createBranchInst(Loop);

  Builder.setInsertionBlock(Exit);
  Builder.createReturnInst(Arg2);

  // So far so good, this will pass
  EXPECT_TRUE(verifyModule(M));

  auto Bad = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(Bad);
  Builder.createReturnInst(Arg2);

  // A dead basic block was added, and hence will fail to verify
  EXPECT_FALSE(verifyModule(M, &errs(), VerificationMode::IR_OPTIMIZED));
}

TEST(IRVerifierTest, ReturnInstTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "testReturn", Function::DefinitionKind::ES5Function, true);
  auto Arg1 = Builder.createJSDynamicParam(F, "num");
  Arg1->setType(Type::createNumber());

  auto Body = Builder.createBasicBlock(F);
  Builder.setInsertionBlock(Body);
  Builder.createReturnInst(Arg1);

  // Everything should pass so far
  EXPECT_TRUE(verifyModule(M));

  Builder.createReturnInst(Arg1);
  // This will also fail as there are now multiple return instrs in the BB
  EXPECT_FALSE(verifyModule(M));
}

TEST(IRVerifierTest, BranchInstTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "testBranch", Function::DefinitionKind::ES5Function, true);

  auto BB1 = Builder.createBasicBlock(F);
  auto BB2 = Builder.createBasicBlock(F);
  auto BB3 = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(BB1);
  Builder.createBranchInst(BB2);

  Builder.setInsertionBlock(BB2);
  Builder.createBranchInst(BB3);

  Builder.setInsertionBlock(BB3);
  Builder.createBranchInst(BB2);

  // Everything should pass
  EXPECT_TRUE(verifyModule(M));

  Builder.createBranchInst(BB2);

  // This will fail as there are now multple branch instrs in the same BB
  EXPECT_FALSE(verifyModule(M));
}

TEST(IRVerifierTest, DominanceTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "testBranch", Function::DefinitionKind::ES5Function, true);
  auto Arg1 = Builder.createJSDynamicParam(F, "num");

  auto Body = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(Body);
  auto AsString = Builder.createAddEmptyStringInst(Arg1);
  AsString->setType(Type::createString());
  Builder.createReturnInst(AsString);

  // This tries to verify that if an instruction A is an operand of another
  // instruction B, A should dominate B.
  EXPECT_TRUE(verifyModule(M, &errs()));
}

TEST(IRVerifierTest, TryStructureTest) {
  auto Ctx = std::make_shared<Context>();
  Module M{Ctx};
  IRBuilder Builder(&M);
  auto F = Builder.createFunction(
      "testBranch", Function::DefinitionKind::ES5Function, true);

  auto entry = Builder.createBasicBlock(F);
  // This BB will be reachable from both outside of a try and inside of a try.
  auto illegalBB = Builder.createBasicBlock(F);
  auto tryStartBB = Builder.createBasicBlock(F);
  auto tryBodyBB = Builder.createBasicBlock(F);
  auto catchBB = Builder.createBasicBlock(F);

  Builder.setInsertionBlock(entry);
  // Here we reach illegalBB from outside a try.
  Builder.createCondBranchInst(
      Builder.getLiteralBool(true), tryStartBB, illegalBB);

  Builder.setInsertionBlock(tryStartBB);
  Builder.createTryStartInst(tryBodyBB, catchBB);

  // Here we reach illegalBB from inside a try.
  Builder.setInsertionBlock(tryBodyBB);
  Builder.createBranchInst(illegalBB);

  Builder.setInsertionBlock(catchBB);
  Builder.createReturnInst(Builder.getLiteralUndefined());

  Builder.setInsertionBlock(illegalBB);
  Builder.createReturnInst(Builder.getLiteralUndefined());

  EXPECT_FALSE(verifyModule(M, &llvh::nulls()));
}

#endif // HERMES_SLOW_DEBUG

} // end anonymous namespace
