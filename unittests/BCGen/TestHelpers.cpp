/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Optimizer/PassManager/Pipeline.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Sema/SemContext.h"
#include "hermes/Sema/SemResolve.h"
#include "hermes/Utils/Options.h"
#include "llvh/Support/SHA1.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::hbc;

/// Compile source code \p source into Hermes bytecode.
/// \return the bytecode as a vector of bytes.
std::unique_ptr<hbc::BytecodeModule> hermes::bytecodeModuleForSource(
    const char *source,
    BytecodeGenerationOptions opts,
    bool optimize) {
  /* Parse source */
  SourceErrorManager sm;
  CodeGenerationSettings codeGenOpts;
  OptimizationSettings optSettings;
  optSettings.staticBuiltins = opts.staticBuiltinsEnabled;
  auto context =
      std::make_shared<Context>(sm, std::move(codeGenOpts), optSettings);
  parser::JSParser jsParser(*context, source);
  auto parsed = jsParser.parse();
  assert(parsed.hasValue() && "Failed to parse source");
  sema::SemContext semCtx{*context};
  auto validated = resolveAST(*context, semCtx, *parsed);
  (void)validated;
  assert(validated && "Failed to validate source");
  auto *ast = parsed.getValue();

  /* Generate IR */
  Module M(context);
  hermes::generateIRFromESTree(&M, semCtx, ast);

  if (optimize)
    runFullOptimizationPasses(M);

  /* Generate bytecode module */
  auto BM = generateBytecodeModule(&M, M.getTopLevelFunction(), opts);
  assert(BM != nullptr && "Failed to generate bytecode module");

  return BM;
}

/// Compile source code \p source into Hermes bytecode.
/// \return the bytecode as a vector of bytes.
std::vector<uint8_t> hermes::bytecodeForSource(
    const char *source,
    BytecodeGenerationOptions opts,
    bool optimize) {
  auto BM = bytecodeModuleForSource(source, opts, optimize);

  /* Serialize it */
  llvh::SmallVector<char, 0> bytecodeVector;
  llvh::raw_svector_ostream OS(bytecodeVector);
  hbc::serializeBytecodeModule(
      *BM,
      llvh::SHA1::hash(llvh::ArrayRef<uint8_t>{
          reinterpret_cast<const uint8_t *>(source), strlen(source)}),
      OS,
      opts);
  return std::vector<uint8_t>{bytecodeVector.begin(), bytecodeVector.end()};
}
