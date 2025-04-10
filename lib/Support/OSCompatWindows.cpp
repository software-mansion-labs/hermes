/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef _WINDOWS

#include "hermes/Support/Compiler.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/OSCompat.h"

#include <cassert>

// Include windows.h first because other includes from windows API need it.
// The blank line after the include is necessary to avoid lint error.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX // do not define min/max macros
#endif
#include <windows.h>

#include <intrin.h>
#include <io.h>
#include <psapi.h>

#include "llvh/ADT/Twine.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace oscompat {

#ifndef NDEBUG
static size_t testPgSz = 0;

void set_test_page_size(size_t pageSz) {
  testPgSz = pageSz;
}

void reset_test_page_size() {
  testPgSz = 0;
}
#endif

static inline size_t page_size_real() {
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwPageSize;
}

size_t page_size() {
#ifndef NDEBUG
  if (testPgSz != 0) {
    return testPgSz;
  }
#endif
  return page_size_real();
}

#ifndef NDEBUG
static constexpr size_t unsetVMAllocLimit = std::numeric_limits<size_t>::max();
static size_t totalVMAllocLimit = unsetVMAllocLimit;

void set_test_vm_allocate_limit(size_t totSz) {
  totalVMAllocLimit = totSz;
}

void unset_test_vm_allocate_limit() {
  totalVMAllocLimit = unsetVMAllocLimit;
}
#endif // !NDEBUG

static char *alignAlloc(void *p, size_t alignment) {
  return reinterpret_cast<char *>(
      llvh::alignTo(reinterpret_cast<uintptr_t>(p), alignment));
}

static llvh::ErrorOr<void *>
vm_allocate_impl(void *addr, size_t sz, DWORD flags) {
  void *result = VirtualAlloc(addr, sz, flags, PAGE_READWRITE);
  if (result == nullptr) {
    // Windows does not have POSIX error codes, but defines its own set.
    // Use system_category with GetLastError so that the codes are interpreted
    // correctly.
    return std::error_code(GetLastError(), std::system_category());
  }
  return result;
}

static std::error_code vm_free_impl(void *p, size_t sz) {
  BOOL ret = VirtualFree(p, 0, MEM_RELEASE);

  return ret ? std::error_code{}
             : std::error_code(GetLastError(), std::system_category());
}

// TODO(T40416012): Use hint on Windows.
static llvh::ErrorOr<void *> vm_allocate_aligned_impl(
    size_t sz,
    size_t alignment,
    bool commit,
    void * /* hint */) {
  /// A value of 3 means vm_allocate_aligned_impl will:
  /// 1. Opportunistic: allocate and see if it happens to be aligned
  /// 2. Regular: Try aligned allocation 3 times (see below for details)
  /// 3. Fallback: Allocate more than needed, and waste the excess
  constexpr int aligned_allocation_attempts = 3;

#ifndef NDEBUG
  assert(sz > 0 && sz % page_size() == 0);
  assert(alignment > 0 && alignment % page_size() == 0);
  if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
    return make_error_code(OOMError::TestVMLimitReached);
  } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
    totalVMAllocLimit -= sz;
  }
#endif // !NDEBUG

  DWORD commitFlag = commit ? MEM_COMMIT : 0;

  // Opportunistically allocate without alignment constraint,
  // and see if the memory happens to be aligned.
  // While this may be unlikely on the first allocation request,
  // subsequent allocation requests have a good chance.
  llvh::ErrorOr<void *> result =
      vm_allocate_impl(nullptr, sz, MEM_RESERVE | commitFlag);
  if (!result) {
    // Don't attempt to do anything further if the allocation failed.
    return result;
  }
  void *addr = *result;
  if (LLVM_LIKELY(addr == alignAlloc(addr, alignment))) {
    return addr;
  }
  // Free the oppotunistic allocation.
  if (std::error_code err = vm_free_impl(addr, sz)) {
    hermes_fatal(
        "Failed to free memory region in vm_allocate_aligned_impl", err);
  }

  for (int attempts = 0; attempts < aligned_allocation_attempts; attempts++) {
    // Allocate a larger section to ensure that it contains
    // a subsection that satisfies the request.
    result = vm_allocate_impl(
        nullptr, sz + alignment - page_size_real(), MEM_RESERVE);
    if (!result) {
      return result;
    }
    addr = *result;
    // Find the desired subsection
    char *aligned = alignAlloc(addr, alignment);

    // Free the larger allocation (including the desired subsection)
    if (std::error_code err = vm_free_impl(addr, sz)) {
      hermes_fatal(
          "Failed to free memory region in vm_allocate_aligned_impl", err);
    }

    // Request allocation at the desired subsection
    result = vm_allocate_impl(aligned, sz, MEM_RESERVE | commitFlag);
    if (result) {
      assert(result.get() == aligned);
      return result.get();
    }
  }

  // Similar to the regular mechanism, but simply return the desired
  // subsection (instead of free and re-allocate). This has two downsides:
  // 1. Wasted virtual address space.
  // 2. vm_free_aligned is now required to call VirtualQuery, which has
  //    a non-trivial cost.
  result =
      vm_allocate_impl(nullptr, sz + alignment - page_size_real(), MEM_RESERVE);
  if (!result) {
    return result;
  }
  addr = *result;
  addr = alignAlloc(addr, alignment);
  if (commit) {
    result = vm_allocate_impl(addr, alignment, MEM_COMMIT);
    if (!result) {
      hermes_fatal(
          "Failed to commit subsection of reserved memory in vm_allocate_aligned_impl",
          result.getError());
    }
    return result;
  } else {
    return addr;
  }
}

llvh::ErrorOr<void *> vm_allocate(size_t sz, void * /* hint */) {
#ifndef NDEBUG
  assert(sz % page_size() == 0);
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    return vm_allocate_aligned(sz, testPgSz);
  }
  if (LLVM_UNLIKELY(sz > totalVMAllocLimit)) {
    return make_error_code(OOMError::TestVMLimitReached);
  } else if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit)) {
    totalVMAllocLimit -= sz;
  }
#endif // !NDEBUG
  return vm_allocate_impl(nullptr, sz, MEM_RESERVE | MEM_COMMIT);
}

llvh::ErrorOr<void *>
vm_allocate_aligned(size_t sz, size_t alignment, void *hint) {
  constexpr bool commit = true;
  return vm_allocate_aligned_impl(sz, alignment, commit, hint);
}

void vm_free(void *p, size_t sz) {
#ifndef NDEBUG
  if (testPgSz != 0 && testPgSz > static_cast<size_t>(page_size_real())) {
    vm_free_aligned(p, sz);
    return;
  }
#endif // !NDEBUG

  if (std::error_code err = vm_free_impl(p, sz)) {
    hermes_fatal("Failed to free virtual memory region", err);
  }

#ifndef NDEBUG
  if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit) && p) {
    totalVMAllocLimit += sz;
  }
#endif
}

void vm_free_aligned(void *p, size_t sz) {
  // VirtualQuery is necessary because p may not be the base location
  // of the allocation (due to possible fallback in vm_allocate_aligned_impl).
  MEMORY_BASIC_INFORMATION mbi;
  SIZE_T query_ret = VirtualQuery(p, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
  assert(query_ret != 0 && "Failed to invoke VirtualQuery in vm_free_aligned");

  BOOL ret = VirtualFree(mbi.AllocationBase, 0, MEM_RELEASE);
  assert(ret && "Failed to invoke VirtualFree in vm_free_aligned.");
  (void)ret;

#ifndef NDEBUG
  if (LLVM_UNLIKELY(totalVMAllocLimit != unsetVMAllocLimit) && p) {
    totalVMAllocLimit += sz;
  }
#endif
}

// TODO(T40416012): Implement these functions for Windows.
llvh::ErrorOr<void *>
vm_reserve_aligned(size_t sz, size_t alignment, void *hint) {
  constexpr bool commit = false;
  return vm_allocate_aligned_impl(sz, alignment, commit, hint);
}

void vm_release_aligned(void *p, size_t sz) {
  vm_free_aligned(p, sz);
}

llvh::ErrorOr<void *> vm_commit(void *p, size_t sz) {
  // n.b. the nullptr check here is important; Windows "helpfully" assumes you
  // forgot to set MEM_RESERVE if you send in a null pointer with only
  // MEM_COMMIT. If we didn't check p, mistakenly calling vm_commit with a
  // nullptr p will succeed and *reserve and commit* the memory. see:
  // https://devblogs.microsoft.com/oldnewthing/20151008-00/?p=91411
  if (p == nullptr) {
    // These are a bit "special case semantics" but for all intents and
    // purposes we can treat this as an invalid operation
    return std::error_code(ERROR_INVALID_OPERATION, std::system_category());
  }
  void *result = VirtualAlloc(p, sz, MEM_COMMIT, PAGE_READWRITE);
  if (result == nullptr) {
    // Windows does not have POSIX error codes, but defines its own set.
    // Use system_category with GetLastError so that the codes are interpreted
    // correctly.
    return std::error_code(GetLastError(), std::system_category());
  }
  return result;
}

void vm_uncommit(void *p, size_t sz) {
  VirtualFree(p, sz, MEM_DECOMMIT);
}

void vm_hugepage(void *p, size_t sz) {
  assert(
      reinterpret_cast<uintptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");
}

void vm_unused(void *p, size_t sz) {
#ifndef NDEBUG
  const size_t PS = page_size();
  assert(
      reinterpret_cast<intptr_t>(p) % PS == 0 &&
      "Precondition: pointer is page-aligned.");
  assert(sz % PS == 0 && "Precondition: size is page-aligned.");
#endif

  // TODO(T40416012) introduce explicit "commit" in OSCompat abstraction of
  // virtual memory

  // Do nothing.

  // In POSIX, a mem page implicitly transitions from "reserved" state to
  // "committed" state on access. However, on Windows, accessing
  // "reserved" but not "committed" page results in an access violation.
  // There is no explicit call to transition to "committed" state
  // in Hermes' virtual memory abstraction.
  // As a result, even though Windows has an API to transition a page from
  // "committed" state back to "reserved" state, we can not invoke it here.
}

void vm_prefetch(void *p, size_t sz) {
  assert(
      reinterpret_cast<intptr_t>(p) % page_size() == 0 &&
      "Precondition: pointer is page-aligned.");

  // TODO(T40415796) provide actual "prefetch" implementation

  // Do nothing
}

void vm_name(void *p, size_t sz, const char *name) {
  (void)p;
  (void)sz;
  (void)name;
}

bool vm_protect(void *p, size_t sz, ProtectMode mode) {
  DWORD oldProtect;
  DWORD newProtect = PAGE_NOACCESS;
  if (mode == ProtectMode::ReadWrite) {
    newProtect = PAGE_READWRITE;
  }
  BOOL err = VirtualProtect(p, sz, newProtect, &oldProtect);
  return err != 0;
}

bool vm_madvise(void *p, size_t sz, MAdvice advice) {
  // Not implemented.
  return false;
}

llvh::ErrorOr<size_t> vm_footprint(char *start, char *end) {
  return std::error_code(errno, std::generic_category());
}

int pages_in_ram(const void *p, size_t sz, llvh::SmallVectorImpl<int> *runs) {
  // Not yet supported.
  return -1;
}

uint64_t peak_rss() {
  PROCESS_MEMORY_COUNTERS pmc;
  BOOL ret = GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
  if (!ret) {
    // failed
    return 0;
  }
  return pmc.PeakWorkingSetSize;
}

uint64_t current_rss() {
  PROCESS_MEMORY_COUNTERS pmc;
  BOOL ret = GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
  if (!ret) {
    // failed
    return 0;
  }
  return pmc.WorkingSetSize;
}

uint64_t current_private_dirty() {
  // Not yet supported.
  return 0;
}

std::vector<std::string> get_vm_protect_modes(const void *p, size_t sz) {
  // Not yet supported.
  return std::vector<std::string>{};
}

bool num_context_switches(long &voluntary, long &involuntary) {
  // Not yet supported.
  voluntary = involuntary = -1;
  return false;
}

uint64_t process_id() {
  return GetCurrentProcessId();
}

uint64_t global_thread_id() {
  return GetCurrentThreadId();
}

namespace detail {

std::pair<const void *, size_t> thread_stack_bounds_impl() {
  // The stack is allocated in a single contiguous allocation, with different
  // protections for the three different regions:
  //
  // high +--------------------+
  //      |  committed memory  |
  //      |                    |
  //      +--------------------+
  //      |  guard region      |
  //      +--------------------+
  //      |  reserved memory   |
  //      |                    |
  // low  +--------------------+
  //
  // This structure is maintained even when the stack is fully exhausted.
  // Empirically, there is always at least one reserved page at the lowest
  // address, followed by a guard region.
  // So to determine the usable stack area, we need to determine the size of the
  // overall allocation, and then subtract the size of the guard region and the
  // permanently reserved page.
  // https://learn.microsoft.com/en-us/windows/win32/procthread/thread-stack-size

  // Create a dummy variable and initialize it so we know it must be in
  // committed memory.
  volatile char x = 0;

  // Obtain the stack bounds by querying somewhere in the committed region.
  MEMORY_BASIC_INFORMATION info;
  VirtualQuery((void *)&x, &info, sizeof(info));

  // The low bound of the stack is the base of the overall stack allocation.
  // This is given by AllocationBase, which corresponds to the base address of
  // the region allocated by VirtualAlloc that contains &x.
  const void *stackLow = info.AllocationBase;
  // The high bound of the stack is the high bound of the commmitted region.
  const void *stackHigh = (char *)info.BaseAddress + info.RegionSize;

  size_t sz = (const char *)stackHigh - (const char *)stackLow;

  const size_t pageSz = page_size_real();

  // If we fail to determine the actual size of the guard region, fall back to
  // 12KB or one page, whichever is larger.
  size_t guardSz = std::max<size_t>(12 * 1024, pageSz);

  // Try up to two times to get the size of the guard region. Note that we have
  // to try multiple times, because a call to VirtualQuery may use additional
  // stack space, which in turn moves the stack.
  for (size_t i = 0; i < 2; ++i) {
    // First obtain the reserved memory region below the guard region.
    VirtualQuery(stackLow, &info, sizeof(info));
    assert(info.State == MEM_RESERVE && "Reserved memory not found");
    auto reservedSz = info.RegionSize;

    // The guard page is the region immediately above the reserved memory
    // region, separating it from already committed stack memory.
    VirtualQuery((const char *)stackLow + reservedSz, &info, sizeof(info));
    // The region above the reserved region must always be committed, since it
    // is either the guard region or committed memory (if VirtualQuery caused
    // the stack to grow).
    assert(info.State == MEM_COMMIT);

    // Before we record the guard size, check that the reserved memory size has
    // not changed due to stack growth.
    auto estGuardSz = info.RegionSize;
    [[maybe_unused]] auto guardProt = info.Protect;

    VirtualQuery(stackLow, &info, sizeof(info));
    assert(info.State == MEM_RESERVE && "Reserved memory not found");

    // If the reserved region has not moved, then we know we had the right base
    // for the guard region, so we are done.
    if (info.RegionSize == reservedSz) {
      assert(guardProt & PAGE_GUARD && "Guard region must be guard");
      guardSz = estGuardSz;
      break;
    }
  }

  // Exclude the guard region from the overall size.
  sz -= guardSz;

  // We need to also exclude space that has been reserved for exception
  // handling.
  // https://devblogs.microsoft.com/oldnewthing/20200610-00/?p=103855
  ULONG guarantee = 0;
  SetThreadStackGuarantee(&guarantee);
  sz -= guarantee;

  // We know that there will always be at least one reserved page even when the
  // stack is exhausted, subtract it.
  sz -= pageSz;

  return {stackHigh, sz};
}

} // namespace detail

void set_thread_name(const char *name) {
  // Set the thread name for TSAN. It doesn't share the same name mapping as the
  // OS does. This macro expands to nothing if TSAN isn't on.
  TsanThreadName(name);
  // SetThreadDescription is too new (since Windows 10 version 1607).
  // Prior to that, the concept of thread names only exists when
  // a Visual Studio debugger is attached.
}

static std::chrono::microseconds::rep fromFileTimeToMicros(
    const FILETIME &fileTime) {
  ULARGE_INTEGER uli;
  uli.LowPart = fileTime.dwLowDateTime;
  uli.HighPart = fileTime.dwHighDateTime;
  // convert from 100-nanosecond to microsecond
  return uli.QuadPart / 10;
}

std::chrono::microseconds thread_cpu_time() {
  FILETIME creationTime;
  FILETIME exitTime;
  FILETIME kernelTime;
  FILETIME userTime;
  GetThreadTimes(
      GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime);
  return std::chrono::microseconds(
      fromFileTimeToMicros(kernelTime) + fromFileTimeToMicros(userTime));
}

bool thread_page_fault_count(int64_t *outMinorFaults, int64_t *outMajorFaults) {
  // Windows provides GetProcessMemoryInfo. There is no counterpart of this
  // API call for threads. In addition, it measures soft page faults.
  // It may be possible to get per-thread information by using ETW (Event
  // Tracing for Windows), which is probably overkill for the use case here.

  // not supported on Windows
  return false;
}

std::string thread_name() {
  // SetThreadDescription/GetThreadDescription is too new (since
  // Windows 10 version 1607).
  // Prior to that, the concept of thread names only exists when
  // a Visual Studio debugger is attached.
  return "";
}

std::vector<bool> sched_getaffinity() {
  // Not yet supported.
  return std::vector<bool>();
}

int sched_getcpu() {
  // Not yet supported.
  return -1;
}

uint64_t cpu_cycle_counter() {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
  return __rdtsc();
#elif __has_builtin(__builtin_readcyclecounter)
  return __builtin_readcyclecounter();
#else
  LARGE_INTEGER cnt;
  QueryPerformanceCounter(&cnt);
  return static_cast<uint64_t>(cnt.QuadPart);
#endif
}

bool set_env(const char *name, const char *value) {
  // Setting an env var to empty requires a lot of hacks on Windows
  assert(*value != '\0' && "value cannot be empty string");
  return _putenv_s(name, value) == 0;
}

bool unset_env(const char *name) {
  return _putenv_s(name, "") == 0;
}

/// Windows does not have the concept of alternate signal stacks, so nothing to
/// do.
/*static*/
void *SigAltStackLeakSuppressor::stackRoot_{nullptr};

SigAltStackLeakSuppressor::~SigAltStackLeakSuppressor() {}

} // namespace oscompat
} // namespace hermes

#endif // _WINDOWS
