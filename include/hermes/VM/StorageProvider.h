/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STORAGEPROVIDER_H
#define HERMES_VM_STORAGEPROVIDER_H

#include "llvh/Support/ErrorOr.h"

#include <limits>
#include <memory>

namespace hermes {
namespace vm {

/// A StorageProvider creates and destroys memory space to be used for segments
/// by the GC.
class StorageProvider {
 public:
  StorageProvider() = default;
  virtual ~StorageProvider();

  /// @name Factories
  /// @{

  /// Provide storage from mmap'ed separate regions.
  static std::unique_ptr<StorageProvider> mmapProvider();

  /// Provide storage from a contiguous mmap'ed region.
  static std::unique_ptr<StorageProvider> contiguousVAProvider(size_t size);

  /// Provide storage via malloc.
  static std::unique_ptr<StorageProvider> mallocProvider();

  /// @}

  /// \return A pointer to a block of memory that has \p sz bytes, and is
  /// aligned on AlignedHeapSegment::kSegmentUnitSize. Note that \p sz must
  /// be non-zero and equals to a multiple of
  /// AlignedHeapSegment::kSegmentUnitSize.
  llvh::ErrorOr<void *> newStorage(size_t sz, const char *name = nullptr);

  /// Delete the given segment's memory space, and make it available for re-use.
  /// Note that \p sz must be the same as used to allocating \p storage.
  /// \post Nothing in the range [storage, storage + sz) is valid memory to be
  /// read or written.
  void deleteStorage(void *storage, size_t sz);

  /// The number of storages this provider has allocated in its lifetime.
  size_t numSucceededAllocs() const;

  /// The number of storages this provider has failed to allocate in its
  /// lifetime.
  size_t numFailedAllocs() const;

  /// The number of storages this provider has deleted its lifetime.
  size_t numDeletedAllocs() const;

  /// The number of storages allocated by this provider that have not been
  /// deleted yet.
  size_t numLiveAllocs() const;

 protected:
  /// \pre \p sz is non-zero and equal to a multiple of
  /// AlignedHeapSegment::kSegmentUnitSize.
  virtual llvh::ErrorOr<void *> newStorageImpl(size_t sz, const char *name) = 0;
  /// \pre \p sz is non-zero and equal to a multiple of
  /// AlignedHeapSegment::kSegmentUnitSize.
  virtual void deleteStorageImpl(void *storage, size_t sz) = 0;

 private:
  size_t numSucceededAllocs_{0};
  size_t numFailedAllocs_{0};
  size_t numDeletedAllocs_{0};
};

/// Attempts to allocate \p sz memory, aligned at \p alignment.
/// If the initial attempt fails, attempts to allocate less memory.
/// If the size falls below minSz, the last error is returned.
/// \returns An error code if all allocs failed, or if one succeeded, a pair
///   of a pointer to the new memory region, and the size of that memory region,
///   which can be less than the supplied \p sz, but >= \p minSz.
/// \pre sz % alignment == 0 && minSz % alignment == 0.
/// \post Returned size is always a multiple of the alignment.
/// NOTE: Exposed for testing purposes.
llvh::ErrorOr<std::pair<void *, size_t>>
vmAllocateAllowLess(size_t sz, size_t minSz, size_t alignment);

} // namespace vm
} // namespace hermes

#endif
