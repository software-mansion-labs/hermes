/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_REGEX_REGEXSERIALIZATION_H
#define HERMES_REGEX_REGEXSERIALIZATION_H

#include "hermes/Regex/RegexSupport.h"
#include "hermes/Support/HermesSafeMath.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/Optional.h"
#include "llvh/ADT/StringRef.h"

#include <deque>
#include <memory>
#include <string>

namespace llvh {
class raw_ostream;
}

/// Support for statically compiling regexps.
namespace hermes {

/// A RegExpTableEntry is simply an (offset, length) pair for bytecode.
struct RegExpTableEntry {
  uint32_t offset;
  uint32_t length;
};

/// Dump a regexp bytecode stream to a raw_ostream \p OS.
void dumpRegexBytecode(llvh::ArrayRef<uint8_t> bytes, llvh::raw_ostream &OS);

/// CompiledRegExp represents a syntax-validated intermediate form of regular
/// expressions.
class CompiledRegExp {
 public:
  ~CompiledRegExp();
  CompiledRegExp(CompiledRegExp &&);
  CompiledRegExp &operator=(CompiledRegExp &&);

  /// Validate a RegExp given by the pattern \p pattern and flags \p flags.
  /// \return a CompiledRegExp if the regexp syntax is valid, llvh::None if it
  /// is invalid, in which case we return an error message by reference in \p
  /// outError (if not null).
  static llvh::Optional<CompiledRegExp> tryCompile(
      llvh::StringRef pattern,
      llvh::StringRef flags,
      llvh::StringRef *outError = nullptr);

  llvh::StringRef getPattern() const {
    return pattern_;
  }

  llvh::StringRef getFlags() const {
    return flags_;
  }

  regex::ParsedGroupNamesMapping &getMapping() {
    return mapping_;
  }

  std::deque<llvh::SmallVector<char16_t, 5>> &getOrderedGroupNames() {
    return orderedGroupNames_;
  }

  /// \return regexp-specific bytecode for the receiver.
  llvh::ArrayRef<uint8_t> getBytecode() const;

 private:
  std::vector<uint8_t> bytecode_;
  std::string pattern_;
  std::string flags_;
  std::deque<llvh::SmallVector<char16_t, 5>> orderedGroupNames_;
  regex::ParsedGroupNamesMapping mapping_;
  CompiledRegExp(
      std::vector<uint8_t> bytecode,
      std::string pattern,
      std::string flags,
      std::deque<llvh::SmallVector<char16_t, 5>> &&orderedGroupNames,
      regex::ParsedGroupNamesMapping &&mapping_);
  CompiledRegExp(const CompiledRegExp &) = delete;
  void operator=(const CompiledRegExp &) = delete;
};

/// A class responsible for assigning IDs to regexps.
class UniquingRegExpTable {
  /// List of pointers to compiled regexps.
  std::vector<CompiledRegExp *> regexps_;

  /// Entry list for the regexp table, can be taken by the BytecodeGenerator.
  std::vector<RegExpTableEntry> entryList_;

  /// Bytecode buffer, pointed to by the keysToIndex_ map.
  std::vector<uint8_t> bytecodeBuffer_;

  /// RegExps are uniqued according to their pattern and flags. Note that a
  /// regexp pattern is logically UCS-2 (or UTF-16 with the 'u' flag). We match
  /// StringStorage in that we represent this as UTF-8, except that UTF-16
  /// surrogate code points are explicitly encoded via the UTF-8 format.
  using KeyType = std::pair<llvh::StringRef, llvh::StringRef>;

  /// Map from compiled regexp (pattern, flags) pairs to its index. The
  /// StringRefs reference data owned by the regexps_ field.
  llvh::DenseMap<KeyType, uint32_t> keysToIndex_;

  /// A UniquingRegExpTable may not be copied.
  UniquingRegExpTable(const UniquingRegExpTable &) = delete;
  void operator=(const UniquingRegExpTable &) = delete;

 public:
  UniquingRegExpTable() = default;
  UniquingRegExpTable(UniquingRegExpTable &&) = default;
  UniquingRegExpTable &operator=(UniquingRegExpTable &&) = default;

  /// Adds a regexp to the table if not already present.
  /// \return the ID of the regexp.
  uint32_t addRegExp(CompiledRegExp *regexp) {
    auto iter = keysToIndex_.find(keyFor(*regexp));
    if (iter != keysToIndex_.end())
      return iter->second;

    uint32_t index = safePossiblyNarrowingCast<uint32_t>(
        regexps_.size(), "Too many regular expressions");
    regexps_.push_back(regexp);
    entryList_.push_back(
        {(uint32_t)bytecodeBuffer_.size(),
         (uint32_t)regexp->getBytecode().size()});
    bytecodeBuffer_.insert(
        bytecodeBuffer_.end(),
        regexp->getBytecode().begin(),
        regexp->getBytecode().end());
    keysToIndex_[keyFor(*regexps_.back())] = index;
    return index;
  }

  /// \return whether the table is empty.
  bool empty() const {
    return regexps_.empty();
  }

  /// Return the regexp entry list.
  llvh::ArrayRef<RegExpTableEntry> getEntryList() const {
    return entryList_;
  }

  /// Return the combined bytecode buffer.
  llvh::ArrayRef<uint8_t> getBytecodeBuffer() const {
    return bytecodeBuffer_;
  }

  /// Disassemble the regexp bytecode, printing the result to the output stream.
  void disassemble(llvh::raw_ostream &OS) const;

 private:
  KeyType keyFor(const CompiledRegExp &re) const {
    return {re.getPattern(), re.getFlags()};
  }
};

} // namespace hermes

#endif
