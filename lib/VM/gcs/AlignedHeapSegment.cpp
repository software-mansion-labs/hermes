/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/AlignedHeapSegment.h"

#include "hermes/Support/OSCompat.h"
#include "hermes/VM/GCBase-inline.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HiddenClass.h"

#include "llvh/Support/MathExtras.h"

#include <algorithm>
#include <cassert>
#include <cstring>

namespace hermes {
namespace vm {

void AlignedHeapSegment::Contents::protectGuardPage(
    oscompat::ProtectMode mode) {
  char *begin = &paddedGuardPage_[kGuardPagePadding];
  size_t size = sizeof(paddedGuardPage_) - kGuardPagePadding;
  size_t PS = oscompat::page_size();
  // Only protect if the actual system page size matches expectations.
  if (reinterpret_cast<uintptr_t>(begin) % PS == 0 && PS <= size) {
    oscompat::vm_protect(begin, PS, mode);
  }
}

llvh::ErrorOr<AlignedHeapSegment> AlignedHeapSegment::create(
    StorageProvider *provider) {
  return create(provider, nullptr);
}

llvh::ErrorOr<AlignedHeapSegment> AlignedHeapSegment::create(
    StorageProvider *provider,
    const char *name) {
  auto result = provider->newStorage(name);
  if (!result) {
    return result.getError();
  }
  return AlignedHeapSegment{provider, *result};
}

AlignedHeapSegment::AlignedHeapSegment(StorageProvider *provider, void *lowLim)
    : lowLim_(static_cast<char *>(lowLim)), provider_(provider) {
  assert(
      storageStart(lowLim_) == lowLim_ &&
      "The lower limit of this storage must be aligned");
  // Storage end must be page-aligned so that markUnused below stays in
  // segment.
  assert(
      reinterpret_cast<uintptr_t>(hiLim()) % oscompat::page_size() == 0 &&
      "The higher limit must be page aligned");
  if (*this) {
    new (contents()) Contents();
    contents()->protectGuardPage(oscompat::ProtectMode::None);
#ifndef NDEBUG
    clear();
#endif
  }
}

void swap(AlignedHeapSegment &a, AlignedHeapSegment &b) {
  // Field lowLim_ and provider_ need to be swapped to make sure the storage of
  // a is not deleted when b is destroyed.
  std::swap(a.lowLim_, b.lowLim_);
  std::swap(a.provider_, b.provider_);
  std::swap(a.level_, b.level_);
  std::swap(a.effectiveEnd_, b.effectiveEnd_);
}

AlignedHeapSegment::AlignedHeapSegment(AlignedHeapSegment &&other)
    : AlignedHeapSegment() {
  swap(*this, other);
}

AlignedHeapSegment &AlignedHeapSegment::operator=(AlignedHeapSegment &&other) {
  swap(*this, other);
  return *this;
}

AlignedHeapSegment::~AlignedHeapSegment() {
  if (lowLim() == nullptr) {
    return;
  }
  contents()->protectGuardPage(oscompat::ProtectMode::ReadWrite);
  contents()->~Contents();
  __asan_unpoison_memory_region(start(), end() - start());

  if (provider_) {
    provider_->deleteStorage(lowLim_);
  }
}

void AlignedHeapSegment::markUnused(char *start, char *end) {
  assert(
      !llvh::alignmentAdjustment(start, oscompat::page_size()) &&
      !llvh::alignmentAdjustment(end, oscompat::page_size()));
  assert(start <= end && "Unused region boundaries inverted");
  assert(lowLim() <= start && end <= hiLim() && "Unused region out-of-bounds");
  // Some kernels seems to require all pages in the mapping to have the same
  // permissions for the advise to "take", so suspend guard page protection
  // temporarily.
  contents()->protectGuardPage(oscompat::ProtectMode::ReadWrite);
#ifndef HERMES_ALLOW_HUGE_PAGES
  oscompat::vm_unused(start, end - start);
#endif
  contents()->protectGuardPage(oscompat::ProtectMode::None);
}

template <AdviseUnused MU>
void AlignedHeapSegment::setLevel(char *lvl) {
  assert(dbgContainsLevel(lvl));
  if (lvl < level_) {
#ifndef NDEBUG
    clear(lvl, level_);
#else
    if (MU == AdviseUnused::Yes) {
      const size_t PS = oscompat::page_size();
      auto nextPageAfter = reinterpret_cast<char *>(
          llvh::alignTo(reinterpret_cast<uintptr_t>(lvl), PS));
      auto nextPageBefore = reinterpret_cast<char *>(
          llvh::alignTo(reinterpret_cast<uintptr_t>(level_), PS));

      markUnused(nextPageAfter, nextPageBefore);
    }
#endif
  }
  level_ = lvl;
}

/// Explicit template instantiations for setLevel
template void AlignedHeapSegment::setLevel<AdviseUnused::Yes>(char *lvl);
template void AlignedHeapSegment::setLevel<AdviseUnused::No>(char *lvl);

template <AdviseUnused MU>
void AlignedHeapSegment::resetLevel() {
  setLevel<MU>(start());
}

/// Explicit template instantiations for resetLevel
template void AlignedHeapSegment::resetLevel<AdviseUnused::Yes>();
template void AlignedHeapSegment::resetLevel<AdviseUnused::No>();

void AlignedHeapSegment::setEffectiveEnd(char *effectiveEnd) {
  assert(
      start() <= effectiveEnd && effectiveEnd <= end() &&
      "Must be valid end for segment.");
  assert(level() <= effectiveEnd && "Must not set effective end below level");
  effectiveEnd_ = effectiveEnd;
}

void AlignedHeapSegment::clearExternalMemoryCharge() {
  setEffectiveEnd(end());
}

#ifndef NDEBUG
bool AlignedHeapSegment::dbgContainsLevel(const void *lvl) const {
  return contains(lvl) || lvl == hiLim();
}

bool AlignedHeapSegment::validPointer(const void *p) const {
  return start() <= p && p < level() &&
      static_cast<const GCCell *>(p)->isValid();
}

void AlignedHeapSegment::clear() {
  clear(start(), end());
}

/* static */ void AlignedHeapSegment::clear(char *start, char *end) {
#if LLVM_ADDRESS_SANITIZER_BUILD
  __asan_poison_memory_region(start, end - start);
#else
  std::memset(start, kInvalidHeapValue, end - start);
#endif
}

/* static */ void AlignedHeapSegment::checkUnwritten(char *start, char *end) {
#if !LLVM_ADDRESS_SANITIZER_BUILD && defined(HERMES_SLOW_DEBUG)
  // Check that the space was not written into.
  std::for_each(
      start, end, [](char value) { assert(value == kInvalidHeapValue); });
#endif
}
#endif // !NDEBUG

} // namespace vm
} // namespace hermes
