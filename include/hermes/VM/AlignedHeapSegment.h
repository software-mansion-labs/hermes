/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SEGMENT_H
#define HERMES_VM_SEGMENT_H

#include "hermes/ADT/BitArray.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/VM/AdviseUnused.h"
#include "hermes/VM/AllocResult.h"
#include "hermes/VM/AllocSource.h"
#include "hermes/VM/CardTableNC.h"
#include "hermes/VM/GCBase.h"
#include "hermes/VM/GCCell.h"
#include "hermes/VM/HeapAlign.h"

#include "llvh/Support/MathExtras.h"

#include <cstdint>
#include <vector>

namespace hermes {
namespace vm {

class StorageProvider;

#ifndef HERMESVM_LOG_HEAP_SEGMENT_SIZE
#error Heap segment size must be defined.
#endif

// In this class:
// TODO (T25527350): Debug Dump
// TODO (T25527350): Heap Moving

/// An \c AlignedHeapSegment is a contiguous chunk of memory aligned to its own
/// storage size (which is a fixed power of two number of bytes).  The storage
/// is further split up according to the diagram below:
///
/// +----------------------------------------+
/// | (1) Card Table                         |
/// +----------------------------------------+
/// | (2) Mark Bit Array                     |
/// +----------------------------------------+
/// | (3) Allocation Space                   |
/// |                                        |
/// | ...                                    |
/// |                                        |
/// | (End)                                  |
/// +----------------------------------------+
///
/// The tables in (1), and (2) cover the contiguous allocation space (3)
/// into which GCCells are bump allocated.
class AlignedHeapSegment {
 public:
  /// @name Constants and utility functions for the aligned storage of \c
  /// AlignedHeapSegment.
  ///
  /// @{
  /// The size and the alignment of the storage, in bytes.
  static constexpr unsigned kLogSize = HERMESVM_LOG_HEAP_SEGMENT_SIZE;
  static constexpr size_t kSize{1 << kLogSize};
  /// Mask for isolating the offset into a storage for a pointer.
  static constexpr size_t kLowMask{kSize - 1};
  /// Mask for isolating the storage being pointed into by a pointer.
  static constexpr size_t kHighMask{~kLowMask};

  /// Returns the storage size, in bytes, of an \c AlignedHeapSegment.
  static constexpr size_t storageSize() {
    return kSize;
  }

  /// Returns the pointer to the beginning of the storage containing \p ptr
  /// (inclusive). Assuming such a storage exists. Note that
  ///
  ///     storageStart(seg.hiLim()) != seg.lowLim()
  ///
  /// as \c seg.hiLim() is not contained in the bounds of \c seg -- it
  /// is the first address not in the bounds.
  static void *storageStart(const void *ptr) {
    return reinterpret_cast<char *>(
        reinterpret_cast<uintptr_t>(ptr) & kHighMask);
  }

  /// Returns the pointer to the end of the storage containing \p ptr
  /// (exclusive). Assuming such a storage exists. Note that
  ///
  ///     storageEnd(seg.hiLim()) != seg.hiLim()
  ///
  /// as \c seg.hiLim() is not contained in the bounds of \c seg -- it
  /// is the first address not in the bounds.
  static void *storageEnd(const void *ptr) {
    return reinterpret_cast<char *>(storageStart(ptr)) + kSize;
  }

  /// Returns the offset in bytes to \p ptr from the start of its containing
  /// storage. Assuming such a storage exists. Note that
  ///
  ///     offset(seg.hiLim()) != seg.size()
  ///
  /// as \c seg.hiLim() is not contained in the bounds of \c seg -- it
  /// is the first address not in the bounds.
  static size_t offset(const char *ptr) {
    return reinterpret_cast<size_t>(ptr) & kLowMask;
  }
  /// @}

  /// Construct a null AlignedHeapSegment (one that does not own memory).
  AlignedHeapSegment() = default;
  /// \c AlignedHeapSegment is movable and assignable, but not copyable.
  AlignedHeapSegment(AlignedHeapSegment &&);
  AlignedHeapSegment &operator=(AlignedHeapSegment &&);
  AlignedHeapSegment(const AlignedHeapSegment &) = delete;

  ~AlignedHeapSegment();

  /// Create a AlignedHeapSegment by allocating memory with \p provider.
  static llvh::ErrorOr<AlignedHeapSegment> create(StorageProvider *provider);
  static llvh::ErrorOr<AlignedHeapSegment> create(
      StorageProvider *provider,
      const char *name);

  /// Contents of the memory region managed by this segment.
  class Contents {
   public:
    /// The number of bits representing the total number of heap-aligned
    /// addresses in the segment storage.
    static constexpr size_t kMarkBitArraySize = kSize >> LogHeapAlign;
    /// BitArray for marking allocation region of a segment.
    using MarkBitArray = BitArray<kMarkBitArraySize>;

    /// Set the protection mode of paddedGuardPage_ (if system page size allows
    /// it).
    void protectGuardPage(oscompat::ProtectMode mode);

   private:
    friend class AlignedHeapSegment;

    /// Note that because of the Contents object, the first few bytes of the
    /// card table are unused, we instead use them to store a small
    /// SHSegmentInfo struct.
    CardTable cardTable_;

    MarkBitArray markBitArray_;

    static constexpr size_t kMetadataSize =
        sizeof(cardTable_) + sizeof(MarkBitArray);
    /// Padding to ensure that the guard page is aligned to a page boundary.
    static constexpr size_t kGuardPagePadding =
        llvh::alignTo<pagesize::kExpectedPageSize>(kMetadataSize) -
        kMetadataSize;

    /// Memory made inaccessible through protectGuardPage, for security and
    /// earlier detection of corruption. Padded to contain at least one full
    /// aligned page.
    char paddedGuardPage_[pagesize::kExpectedPageSize + kGuardPagePadding];

    static constexpr size_t kMetadataAndGuardSize =
        kMetadataSize + sizeof(paddedGuardPage_);

    /// The first byte of the allocation region, which extends past the "end" of
    /// the struct, to the end of the memory region that contains it.
    char allocRegion_[1];
  };

  static_assert(
      offsetof(Contents, paddedGuardPage_) == Contents::kMetadataSize,
      "Should not need padding after metadata.");

  /// The total space at the start of the CardTable taken up by the metadata and
  /// guard page in the Contents struct.
  static constexpr size_t kCardTableUnusedPrefixBytes =
      Contents::kMetadataAndGuardSize / CardTable::kHeapBytesPerCardByte;
  static_assert(
      sizeof(SHSegmentInfo) < kCardTableUnusedPrefixBytes,
      "SHSegmentInfo does not fit in available unused CardTable space.");

  /// The offset from the beginning of a segment of the allocatable region.
  static constexpr size_t offsetOfAllocRegion{offsetof(Contents, allocRegion_)};

  static_assert(
      isSizeHeapAligned(offsetOfAllocRegion),
      "Allocation region must start at a heap aligned offset");

  static_assert(
      (offsetof(Contents, paddedGuardPage_) + Contents::kGuardPagePadding) %
              pagesize::kExpectedPageSize ==
          0,
      "Guard page must be aligned to likely page size");

  class HeapCellIterator : public llvh::iterator_facade_base<
                               HeapCellIterator,
                               std::forward_iterator_tag,
                               GCCell *> {
   public:
    HeapCellIterator(GCCell *cell) : cell_(cell) {}

    bool operator==(const HeapCellIterator &R) const {
      return cell_ == R.cell_;
    }

    GCCell *const &operator*() const {
      return cell_;
    }

    HeapCellIterator &operator++() {
      cell_ = cell_->nextCell();
      return *this;
    }

   private:
    GCCell *cell_{nullptr};
  };

  /// Returns the index of the segment containing \p lowLim, which is required
  /// to be the start of its containing segment.  (This can allow extra
  /// efficiency, in cases where the segment start has already been computed.)
  static unsigned getSegmentIndexFromStart(const void *lowLim) {
    assert(lowLim == storageStart(lowLim) && "Precondition.");
    auto *segInfo = reinterpret_cast<const SHSegmentInfo *>(lowLim);
    return segInfo->index;
  }

  /// Requires that \p lowLim is the start address of a segment, and sets
  /// that segment's index to \p index.
  static void setSegmentIndexFromStart(void *lowLim, unsigned index) {
    assert(lowLim == storageStart(lowLim) && "Precondition.");
    auto *segInfo = reinterpret_cast<SHSegmentInfo *>(lowLim);
    segInfo->index = index;
  }

  /// Attempt an allocation of the given size in the segment.  If there is
  /// sufficent space, cast the space as a GCCell, and returns an uninitialized
  /// pointer to that cell (with success = true).  If there is not sufficient
  /// space, returns {nullptr, false}.
  inline AllocResult alloc(uint32_t size);

  /// Given the \p lowLim of some valid segment's memory region, returns a
  /// pointer to the AlignedHeapSegment::Contents laid out in that storage,
  /// assuming it exists.
  inline static Contents *contents(void *lowLim);
  inline static const Contents *contents(const void *lowLim);

  /// Given a \p ptr into the memory region of some valid segment \c s, returns
  /// a pointer to the CardTable covering the segment containing the pointer.
  ///
  /// \pre There exists a currently alive heap that claims to contain \c ptr.
  inline static CardTable *cardTableCovering(const void *ptr);

  /// Given a \p ptr into the memory region of some valid segment \c s, returns
  /// a pointer to the MarkBitArray covering the segment containing the
  /// pointer.
  ///
  /// \pre There exists a currently alive heap that claims to contain \c ptr.
  inline static Contents::MarkBitArray *markBitArrayCovering(const void *ptr);

  /// Translate the given address to a 0-based index in the MarkBitArray of its
  /// segment. The base address is the start of the storage of this segment.
  static size_t addressToMarkBitArrayIndex(const void *ptr) {
    auto *cp = reinterpret_cast<const char *>(ptr);
    auto *base = reinterpret_cast<const char *>(storageStart(cp));
    return (cp - base) >> LogHeapAlign;
  }

  /// Mark the given \p cell.  Assumes the given address is a valid heap object.
  inline static void setCellMarkBit(const GCCell *cell);

  /// Return whether the given \p cell is marked.  Assumes the given address is
  /// a valid heap object.
  inline static bool getCellMarkBit(const GCCell *cell);

  /// Find the head of the first cell that extends into the card at index
  /// \p cardIdx.
  /// \return A cell such that
  /// cell <= indexToAddress(cardIdx) < cell->nextCell().
  inline GCCell *getFirstCellHead(size_t cardIdx);

  /// Record the head of this cell so it can be found by the card scanner.
  static inline void setCellHead(const GCCell *start, const size_t sz);

  /// The largest size the allocation region of an aligned heap segment could
  /// be.
  inline static constexpr size_t maxSize();

  /// The size of the allocation region in this aligned heap segment.
  inline size_t size() const;

  /// The number of bytes in the segment that are currently allocated.
  inline size_t used() const;

  /// The number of bytes in the segment that are available for allocation.
  inline size_t available() const;

  /// Returns the address that is the lower bound of the segment.
  /// \post The returned pointer is guaranteed to be aligned to a segment
  ///   boundary.
  char *lowLim() const {
    return lowLim_;
  }

  /// Returns the address that is the upper bound of the segment.
  char *hiLim() const {
    return lowLim() + storageSize();
  }

  /// Returns the address at which the first allocation in this segment would
  /// occur.
  /// Disable UB sanitization because 'this' may be null during the tests.
  inline char *start() const LLVM_NO_SANITIZE("undefined");

  /// Returns the first address after the region in which allocations can occur,
  /// taking external memory credits into a account (they decrease the effective
  /// end).
  inline char *effectiveEnd() const;

  /// The external memory charge of the generation owning this segment may have
  /// changed; set the effective end of the segment to the given \p
  /// effectiveEnd, which is required to be a valid effective end for the
  /// segment.
  void setEffectiveEnd(char *effectiveEnd);

  /// Clear any external memory charge for this segment.  This has the effect of
  /// equating the effective end to the real end.
  void clearExternalMemoryCharge();

  /// Returns the first address after the region in which allocations may occur,
  /// ignoring external memory credits.
  inline char *end() const;

  /// Returns the address at which the next allocation, if any, will occur.
  inline char *level() const;

  /// Returns an iterator range corresponding to the cells in this segment.
  inline llvh::iterator_range<HeapCellIterator> cells();

  /// Returns whether \p a and \p b are contained in the same
  /// AlignedHeapSegment.
  inline static bool containedInSame(const void *a, const void *b);

  /// Return a reference to the card table covering the memory region managed by
  /// this segment.
  /// Disable sanitization because 'this' may be null in the tests.
  inline CardTable &cardTable() const LLVM_NO_SANITIZE("null");

  /// Return a reference to the mark bit array covering the memory region
  /// managed by this segment.
  inline Contents::MarkBitArray &markBitArray() const;

  explicit operator bool() const {
    return lowLim();
  }

  /// \return \c true if and only if \p ptr is within the memory range owned by
  ///     this \c AlignedHeapSegment.
  bool contains(const void *ptr) const {
    return storageStart(ptr) == lowLim();
  }

  /// Mark a set of pages as unused.
  ///
  /// \pre start and end must be aligned to the page boundary.
  void markUnused(char *start, char *end);

  /// Clear allocations after \p level in this segment.
  ///
  /// \p MU Indicate whether the newly freed pages should be returned to the OS.
  ///
  /// \post this->level() == level.
  template <AdviseUnused MU = AdviseUnused::No>
  void setLevel(char *level);

  /// Clear allocations in this segment.
  ///
  /// \p MU Indicate whether the newly freed pages should be returned to the OS.
  ///
  /// \post this->level() == this->start();
  template <AdviseUnused MU = AdviseUnused::No>
  void resetLevel();

#ifndef NDEBUG
  /// Returns true iff \p lvl could refer to a level within this segment.
  bool dbgContainsLevel(const void *lvl) const;

  /// Returns true iff \p p is located within a valid section of the segment,
  /// and not at dead memory.
  bool validPointer(const void *p) const;

  /// Set the contents of the segment to a dead value.
  void clear();
  /// Set the given range [start, end) to a dead value.
  static void clear(char *start, char *end);
  /// Checks that dead values are present in the [start, end) range.
  static void checkUnwritten(char *start, char *end);
#endif

 protected:
  /// Return a pointer to the contents of the memory region managed by this
  /// segment.
  inline Contents *contents() const;

  /// The start of the aligned segment.
  char *lowLim_{nullptr};

  /// The provider that created this segment. It will be used to properly
  /// destroy this.
  StorageProvider *provider_{nullptr};

  char *level_{start()};

  /// The upper limit of the space that we can currently allocated into;
  /// this may be decreased when externally allocated memory is credited to
  /// the generation owning this space.
  char *effectiveEnd_{end()};

  /// Used in move constructor and move assignment operator following the copy
  /// and swap idiom.
  friend void swap(AlignedHeapSegment &a, AlignedHeapSegment &b);

 private:
  AlignedHeapSegment(StorageProvider *provider, void *lowLim);
};

AllocResult AlignedHeapSegment::alloc(uint32_t size) {
  assert(lowLim() != nullptr && "Cannot allocate in a null segment");
  assert(size >= sizeof(GCCell) && "cell must be larger than GCCell");
  assert(isSizeHeapAligned(size) && "size must be heap aligned");

  char *cellPtr; // Initialized in the if below.

  // On 64-bit systems, we know that we can't allocate a size large enough to
  // cause a pointer value to overflow. This is not true on 32-bit systems. This
  // test should be decided statically.
  // TODO: Is there a portable way of expressing this in the preprocessor?
  if (sizeof(void *) == 8) {
    // Calculate the new level_ once.
    char *newLevel = level_ + size;
    if (LLVM_UNLIKELY(newLevel > effectiveEnd())) {
      return {nullptr, false};
    }
    cellPtr = level_;
    level_ = newLevel;
  } else {
    if (LLVM_UNLIKELY(available() < size)) {
      return {nullptr, false};
    }
    cellPtr = level_;
    level_ += size;
  }

  __asan_unpoison_memory_region(cellPtr, size);
#ifndef NDEBUG
  checkUnwritten(cellPtr, cellPtr + size);
#endif

  auto *cell = reinterpret_cast<GCCell *>(cellPtr);
  return {cell, true};
}

/*static*/
AlignedHeapSegment::Contents::MarkBitArray *
AlignedHeapSegment::markBitArrayCovering(const void *ptr) {
  return &contents(storageStart(ptr))->markBitArray_;
}

/*static*/
void AlignedHeapSegment::setCellMarkBit(const GCCell *cell) {
  auto *markBits = markBitArrayCovering(cell);
  size_t ind = addressToMarkBitArrayIndex(cell);
  markBits->set(ind, true);
}

/*static*/
bool AlignedHeapSegment::getCellMarkBit(const GCCell *cell) {
  auto *markBits = markBitArrayCovering(cell);
  size_t ind = addressToMarkBitArrayIndex(cell);
  return markBits->at(ind);
}

GCCell *AlignedHeapSegment::getFirstCellHead(size_t cardIdx) {
  CardTable &cards = cardTable();
  GCCell *cell = cards.firstObjForCard(cardIdx);
  assert(cell->isValid() && "Object head doesn't point to a valid object");
  return cell;
}

/* static */
void AlignedHeapSegment::setCellHead(const GCCell *cellStart, const size_t sz) {
  const char *start = reinterpret_cast<const char *>(cellStart);
  const char *end = start + sz;
  CardTable *cards = cardTableCovering(start);
  auto boundary = cards->nextBoundary(start);
  // If this object crosses a card boundary, then update boundaries
  // appropriately.
  if (boundary.address() < end) {
    cards->updateBoundaries(&boundary, start, end);
  }
}

/* static */ AlignedHeapSegment::Contents *AlignedHeapSegment::contents(
    void *lowLim) {
  return reinterpret_cast<Contents *>(lowLim);
}

/* static */ const AlignedHeapSegment::Contents *AlignedHeapSegment::contents(
    const void *lowLim) {
  return reinterpret_cast<const Contents *>(lowLim);
}

/* static */ CardTable *AlignedHeapSegment::cardTableCovering(const void *ptr) {
  return &AlignedHeapSegment::contents(storageStart(ptr))->cardTable_;
}

/* static */ constexpr size_t AlignedHeapSegment::maxSize() {
  return storageSize() - offsetof(Contents, allocRegion_);
}

size_t AlignedHeapSegment::size() const {
  return end() - start();
}

size_t AlignedHeapSegment::used() const {
  return level() - start();
}

size_t AlignedHeapSegment::available() const {
  return effectiveEnd() - level();
}

char *AlignedHeapSegment::start() const {
  return contents()->allocRegion_;
}

char *AlignedHeapSegment::effectiveEnd() const {
  return effectiveEnd_;
}

char *AlignedHeapSegment::end() const {
  return start() + maxSize();
}

char *AlignedHeapSegment::level() const {
  return level_;
}

llvh::iterator_range<AlignedHeapSegment::HeapCellIterator>
AlignedHeapSegment::cells() {
  return {
      HeapCellIterator(reinterpret_cast<GCCell *>(start())),
      HeapCellIterator(reinterpret_cast<GCCell *>(level()))};
}

/* static */
bool AlignedHeapSegment::containedInSame(const void *a, const void *b) {
  return (reinterpret_cast<uintptr_t>(a) ^ reinterpret_cast<uintptr_t>(b)) <
      storageSize();
}

CardTable &AlignedHeapSegment::cardTable() const {
  return contents()->cardTable_;
}

AlignedHeapSegment::Contents::MarkBitArray &AlignedHeapSegment::markBitArray()
    const {
  return contents()->markBitArray_;
}

AlignedHeapSegment::Contents *AlignedHeapSegment::contents() const {
  return contents(lowLim());
}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SEGMENT_H
