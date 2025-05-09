/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSONLexer.h"

#include "hermes/VM/StringPrimitive.h"
#include "llvh/ADT/ScopeExit.h"

#include "dtoa/dtoa.h"

namespace hermes {
namespace vm {

static const char *TrueString = "true";
static const char *FalseString = "false";
static const char *NullString = "null";

static bool isJSONWhiteSpace(char16_t ch) {
  // JSONWhiteSpace includes <TAB>, <CR>, <LF>, <SP>.
  return (ch == u'\t' || ch == u'\r' || ch == u'\n' || ch == u' ');
}

ExecutionStatus JSONLexer::advance() {
  return advanceHelper(false);
}

ExecutionStatus JSONLexer::advanceStrAsSymbol() {
  return advanceHelper(true);
}

ExecutionStatus JSONLexer::advanceHelper(bool forKey) {
  // Skip whitespaces.
  while (curCharPtr_.hasChar() && isJSONWhiteSpace(*curCharPtr_)) {
    ++curCharPtr_;
  }

  // End of buffer.
  if (!curCharPtr_.hasChar()) {
    token_.setEof();
    return ExecutionStatus::RETURNED;
  }

  token_.setFirstChar(*curCharPtr_);

#define PUNC(ch, tok)          \
  case ch:                     \
    token_.setPunctuator(tok); \
    ++curCharPtr_;             \
    return ExecutionStatus::RETURNED

#define WORD(ch, word, tok) \
  case ch:                  \
    return scanWord(word, tok)

  switch (*curCharPtr_) {
    PUNC(u'{', JSONTokenKind::LBrace);
    PUNC(u'}', JSONTokenKind::RBrace);
    PUNC(u'[', JSONTokenKind::LSquare);
    PUNC(u']', JSONTokenKind::RSquare);
    PUNC(u',', JSONTokenKind::Comma);
    PUNC(u':', JSONTokenKind::Colon);
    WORD(u't', TrueString, JSONTokenKind::True);
    WORD(u'f', FalseString, JSONTokenKind::False);
    WORD(u'n', NullString, JSONTokenKind::Null);

    case u'-':
    case u'0':
    case u'1':
    case u'2':
    case u'3':
    case u'4':
    case u'5':
    case u'6':
    case u'7':
    case u'8':
    case u'9':
      return scanNumber();

    case u'"':
      if (forKey) {
        return scanString<StrAsSymbol>();
      } else {
        return scanString<StrAsValue>();
      }

    default:
      return errorWithChar(u"Unexpected character: ", *curCharPtr_);
  }
}

CallResult<char16_t> JSONLexer::consumeUnicode() {
  uint16_t val = 0;
  for (unsigned i = 0; i < 4; ++i) {
    if (!curCharPtr_.hasChar()) {
      return error("Unexpected end of input");
    }
    int ch = *curCharPtr_ | 32;
    if (ch >= '0' && ch <= '9') {
      ch -= '0';
    } else if (ch >= 'a' && ch <= 'f') {
      ch -= 'a' - 10;
    } else {
      return errorWithChar(u"Invalid unicode point character: ", *curCharPtr_);
    }
    val = (val << 4) + ch;
    ++curCharPtr_;
  }

  return static_cast<char16_t>(val);
}

ExecutionStatus JSONLexer::scanNumber() {
  llvh::SmallVector<char, 32> str8;
  while (curCharPtr_.hasChar()) {
    auto ch = *curCharPtr_;
    if (!(ch == u'-' || ch == u'+' || ch == u'.' || (ch | 32) == u'e' ||
          (ch >= u'0' && ch <= u'9'))) {
      break;
    }
    str8.push_back(ch);
    ++curCharPtr_;
  }

  size_t len = str8.size();
  assert(len > 0 && "scanNumber must be called on a number-looking char");
  if (str8[0] == '0' && len > 1 && str8[1] >= '0' && str8[1] <= '9') {
    // The integer part cannot start with 0, unless it's 0.
    return errorWithChar(u"Unexpected character in number: ", str8[1]);
  }

  str8.push_back('\0');

  char *endPtr;
  double value = ::hermes_g_strtod(str8.data(), &endPtr);
  if (endPtr != str8.data() + len) {
    return errorWithChar(u"Unexpected character in number: ", *endPtr);
  }
  token_.setNumber(value);
  return ExecutionStatus::RETURNED;
}

template <typename ForKey>
ExecutionStatus JSONLexer::scanString() {
  assert(*curCharPtr_ == '"');
  ++curCharPtr_;
  bool hasEscape = false;
  // Ideally we don't have to use tmpStorage. In the case of a plain string with
  // no escapes, we construct an ArrayRef at the end of scanning that points to
  // the beginning and end of the string.
  SmallU16String<32> tmpStorage;
  curCharPtr_.beginCapture();
  // Make sure we don't somehow leave a dangling open capture.
  auto ensureCaptureClosed =
      llvh::make_scope_exit([this] { curCharPtr_.cancelCapture(); });
  bool allAscii = true;
  hermes::JenkinsHash hash = hermes::JenkinsHashInit;

  while (curCharPtr_.hasChar()) {
    if (*curCharPtr_ == '"') {
      // End of string.
      llvh::ArrayRef<char16_t> strRef =
          hasEscape ? tmpStorage.arrayRef() : curCharPtr_.endCapture();
      ++curCharPtr_;
      if constexpr (ForKey::value) {
        auto symRes = runtime_.getIdentifierTable().getSymbolHandle(
            runtime_, strRef, hash);
        if (symRes == ExecutionStatus::EXCEPTION)
          return ExecutionStatus::EXCEPTION;
        token_.setSymbol(*symRes);
        return ExecutionStatus::RETURNED;
      }
      auto strRes =
          StringPrimitive::createWithKnownEncoding(runtime_, strRef, allAscii);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      token_.setString(runtime_.makeHandle<StringPrimitive>(*strRes));
      return ExecutionStatus::RETURNED;
    } else if (*curCharPtr_ <= '\u001F') {
      return error(u"U+0000 thru U+001F is not allowed in string");
    }
    char16_t scannedChar = -1;
    if (*curCharPtr_ == u'\\') {
      if (!hasEscape) {
        // This is the first escape character encountered, so append everything
        // we've seen so far to tmpStorage.
        tmpStorage.append(curCharPtr_.endCapture());
      }
      hasEscape = true;
      ++curCharPtr_;
      if (!curCharPtr_.hasChar()) {
        return error("Unexpected end of input");
      }
      switch (*curCharPtr_) {
#define CONSUME_VAL(v)     \
  tmpStorage.push_back(v); \
  ++curCharPtr_;

        case u'"':
        case u'/':
        case u'\\':
          CONSUME_VAL(*curCharPtr_)
          break;
        case 'b':
          CONSUME_VAL(8)
          break;
        case 'f':
          CONSUME_VAL(12)
          break;
        case 'n':
          CONSUME_VAL(10)
          break;
        case 'r':
          CONSUME_VAL(13)
          break;
        case 't':
          CONSUME_VAL(9)
          break;
        case 'u': {
          ++curCharPtr_;
          CallResult<char16_t> cr = consumeUnicode();
          if (LLVM_UNLIKELY(cr == ExecutionStatus::EXCEPTION)) {
            return ExecutionStatus::EXCEPTION;
          }
          tmpStorage.push_back(*cr);
          break;
        }

        default:
          return errorWithChar(u"Invalid escape sequence: ", *curCharPtr_);
      }
      scannedChar = tmpStorage.back();
    } else {
      scannedChar = *curCharPtr_;
      if (hasEscape)
        tmpStorage.push_back(scannedChar);
      ++curCharPtr_;
    }
    if constexpr (ForKey::value) {
      hash = hermes::updateJenkinsHash(hash, scannedChar);
    } else {
      allAscii &= isASCII(scannedChar);
    }
  }
  return error("Unexpected end of input");
}

ExecutionStatus JSONLexer::scanWord(const char *word, JSONTokenKind kind) {
  while (*word && curCharPtr_.hasChar()) {
    if (*curCharPtr_ != *word) {
      return errorWithChar(u"Unexpected character: ", *curCharPtr_);
    }
    ++curCharPtr_;
    ++word;
  }
  if (*word) {
    return error(u"Unexpected end of input");
  }
  token_.setPunctuator(kind);
  return ExecutionStatus::RETURNED;
}

} // namespace vm
} // namespace hermes
