/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CompileJS.h"

#include "hermes/BCGen/HBC/HBC.h"
#include "hermes/SourceMap/SourceMapParser.h"
#include "hermes/Support/Algorithms.h"

#include "llvh/Support/SHA1.h"

namespace hermes {

// Enbles using DiagnosticHandler with SourceErrorManager::DiagHandlerTy's API.
static void diagHandlerAdapter(const llvh::SMDiagnostic &d, void *context) {
  auto diagHandler = static_cast<DiagnosticHandler *>(context);
  DiagnosticHandler::Kind kind = DiagnosticHandler::Note;
  switch (d.getKind()) {
    case llvh::SourceMgr::DK_Error:
      kind = DiagnosticHandler::Error;
      break;
    case llvh::SourceMgr::DK_Warning:
      kind = DiagnosticHandler::Warning;
      break;
    case llvh::SourceMgr::DK_Note:
      kind = DiagnosticHandler::Note;
      break;
    default:
      assert(false && "Hermes compiler produced unexpected diagnostic kind");
  };
  // Use 1-based indexing for column to match what is used in formatted errors.
  diagHandler->handle(
      {kind,
       d.getLineNo(),
       d.getColumnNo() + 1,
       d.getMessage(),
       d.getRanges()});
}

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    const CompileJSOptions &compileJSOptions,
    DiagnosticHandler *diagHandler,
    std::optional<std::string_view> sourceMapBuf) {
  hbc::CompileFlags flags{};
  flags.debug = compileJSOptions.debug;
  flags.format = EmitBundle;
  flags.emitAsyncBreakCheck = compileJSOptions.emitAsyncBreakCheck;
  flags.inlineMaxSize = compileJSOptions.inlineMaxSize;

  // If there is a source map, ensure that it is null terminated, copying it if
  // needed.
  std::string smCopy;
  llvh::StringRef smRef;
  if (sourceMapBuf.has_value()) {
    if (sourceMapBuf->back() != '\0') {
      smCopy = *sourceMapBuf;
      smRef = {smCopy.data(), smCopy.size() + 1};
    } else {
      smRef = {sourceMapBuf->data(), sourceMapBuf->size()};
    }
  }

  // Note that we are relying the zero termination provided by str.data(),
  // because the parser requires it.
  auto res = hbc::createBCProviderFromSrc(
      std::make_unique<hermes::Buffer>((const uint8_t *)str.data(), str.size()),
      sourceURL,
      smRef,
      flags,
      "global",
      diagHandler ? diagHandlerAdapter : nullptr,
      diagHandler,
      compileJSOptions.optimize ? hbc::fullOptimizationPipeline : nullptr);
  if (!res.first)
    return false;

  llvh::raw_string_ostream bcstream(bytecode);

  BytecodeGenerationOptions opts(::hermes::EmitBundle);
  opts.optimizationEnabled = compileJSOptions.optimize;

  assert(
      res.first->getBytecodeModule() &&
      "BCProviderFromSrc must have a bytecode module");
  hbc::serializeBytecodeModule(
      *res.first->getBytecodeModule(),
      llvh::SHA1::hash(llvh::makeArrayRef(
          reinterpret_cast<const uint8_t *>(str.data()), str.size())),
      bcstream,
      opts);

  // Flush to string.
  bcstream.flush();
  return true;
}

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize,
    bool emitAsyncBreakCheck,
    DiagnosticHandler *diagHandler,
    std::optional<std::string_view> sourceMapBuf,
    bool debug) {
  CompileJSOptions options;
  options.optimize = optimize;
  options.emitAsyncBreakCheck = emitAsyncBreakCheck;
  options.debug = debug;
  return compileJS(
      str, sourceURL, bytecode, options, diagHandler, sourceMapBuf);
}

bool compileJS(const std::string &str, std::string &bytecode, bool optimize) {
  return compileJS(str, "", bytecode, optimize, false, nullptr);
}

bool compileJS(
    const std::string &str,
    const std::string &sourceURL,
    std::string &bytecode,
    bool optimize) {
  return compileJS(str, sourceURL, bytecode, optimize, false, nullptr);
}

} // namespace hermes
