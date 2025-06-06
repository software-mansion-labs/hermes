/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestHelpers1.h"
#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/BCGen/HBC/UniquingStringLiteralTable.h"
#include "hermes/IRGen/IRGen.h"
#include "hermes/Parser/JSParser.h"
#include "hermes/Sema/SemContext.h"
#include "hermes/Sema/SemResolve.h"
#include "hermes/Utils/Options.h"
#include "llvh/Support/SHA1.h"

#include "gtest/gtest.h"

using namespace hermes;
using namespace hermes::hbc;

/// Compile source code \p source into Hermes bytecode.
/// Populate source mappings in \p sourceMapGen if provided.
/// \return the bytecode as a vector of bytes.
std::vector<uint8_t> hermes::bytecodeForSource(
    const char *source,
    TestCompileFlags flags,
    SourceMapGenerator *sourceMapGen) {
  /* Parse source */
  SourceErrorManager sm;
  CodeGenerationSettings codeGenOpts;
  OptimizationSettings optSettings;
  optSettings.staticBuiltins = flags.staticBuiltins;
  auto context =
      std::make_shared<Context>(sm, std::move(codeGenOpts), optSettings);
  if (sourceMapGen) {
    context->setDebugInfoSetting(DebugInfoSetting::SOURCE_MAP);
    sourceMapGen->addSource("JavaScript");
  }
  auto sourceBuf = llvh::MemoryBuffer::getMemBuffer(source, "JavaScript");
  parser::JSParser jsParser(*context, std::move(sourceBuf));
  auto parsed = jsParser.parse();
  assert(parsed.hasValue() && "Failed to parse source");
  sema::SemContext semCtx(*context);
  auto validated = resolveAST(*context, semCtx, *parsed);
  (void)validated;
  assert(validated && "Failed to validate source");
  auto *ast = parsed.getValue();

  /* Generate IR */
  Module M(context);
  hermes::generateIRFromESTree(&M, semCtx, ast);

  /* Generate and serialize bytecode module */
  auto bytecodeGenOpts = BytecodeGenerationOptions::defaults();
  bytecodeGenOpts.format = OutputFormatKind::EmitBundle;
  bytecodeGenOpts.staticBuiltinsEnabled = flags.staticBuiltins;
  bytecodeGenOpts.stripDebugInfoSection = sourceMapGen != nullptr;
  auto sourceHash = llvh::SHA1::hash(llvh::ArrayRef<uint8_t>{
      reinterpret_cast<const uint8_t *>(source), strlen(source)});
  llvh::SmallVector<char, 0> bytecodeVector;
  llvh::raw_svector_ostream OS(bytecodeVector);
  std::unique_ptr<BytecodeModule> BM = generateBytecodeModule(
      &M, M.getTopLevelFunction(), bytecodeGenOpts, llvh::None, sourceMapGen);
  assert(BM != nullptr && "Failed to generate bytecode module");

  if (bytecodeGenOpts.format == OutputFormatKind::EmitBundle) {
    hbc::serializeBytecodeModule(*BM, sourceHash, OS, bytecodeGenOpts);
  }
  // Now that the BytecodeFunctions know their offsets into the stream, we
  // can populate the source map.
  if (sourceMapGen)
    BM->populateSourceMap(sourceMapGen);

  return std::vector<uint8_t>{bytecodeVector.begin(), bytecodeVector.end()};
}
