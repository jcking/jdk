/*
 * Copyright (c) 1997, 2022, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#include "memory/malloc.hpp"

#include "utilities/align.hpp"
#include "utilities/compilerWarnings.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/macros.hpp"

#if defined(TARGET_MALLOC_tcmalloc)
#include <tcmalloc/malloc_extension.h>
#elif defined(TARGET_MALLOC_jemalloc)
#include <jemalloc/jemalloc.h>
#elif defined(TARGET_MALLOC_mimalloc)
#include <mimalloc.h>
#else // defined(TARGET_MALLOC_system)
#if defined(__APPLE__)
#include <malloc/malloc.h>
#endif

#if defined(__GLIBC__) || defined(_WIN32)
#include <malloc.h>
#endif
#endif

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>

#if defined(__has_attribute)
#define HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAS_ATTRIBUTE(x) 0
#endif

#if defined(__has_builtin)
#define HAS_BUILTIN(x) __has_builtin(x)
#else
#define HAS_BUILTIN(x) 0
#endif

#if (defined(ADDRESS_SANITIZER) || defined(LEAK_SANITIZER)) && !defined(TARGET_MALLOC_system)
#error Custom malloc implementations are not compatible with ASan/LSan.
#endif

static ALWAYSINLINE size_t usable_size(const void* ptr, size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  return MAX2(::malloc_usable_size(const_cast<void*>(ptr)), size);
#elif defined(TARGET_MALLOC_jemalloc)
  return MAX2(::je_malloc_usable_size(const_cast<void*>(ptr)), size);
#elif defined(TARGET_MALLOC_mimalloc)
  return MAX2(::mi_malloc_usable_size(size), size);
#else // defined(TARGET_MALLOC_system)
#if (defined(__GLIBC__) || defined(__FreeBSD__))
  return MAX2(::malloc_usable_size(const_cast<void*>(ptr)), size);
#else
  static_cast<void>(ptr);
  return size;
#endif
#endif
}

static ALWAYSINLINE void* do_malloc(size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  ALLOW_C_FUNCTION(::malloc, return ::malloc(size);)
#elif defined(TARGET_MALLOC_jemalloc)
  return ::je_malloc(size);
#elif defined(TARGET_MALLOC_mimalloc)
  return ::mi_malloc(size);
#else // defined(TARGET_MALLOC_system)
  ALLOW_C_FUNCTION(::malloc, return ::malloc(size);)
#endif
}

static ALWAYSINLINE void* do_realloc(void* old_ptr, size_t new_size) {
#if defined(TARGET_MALLOC_tcmalloc)
  ALLOW_C_FUNCTION(::realloc, return ::realloc(old_ptr, new_size);)
#elif defined(TARGET_MALLOC_jemalloc)
  return ::je_realloc(old_ptr, new_size);
#elif defined(TARGET_MALLOC_mimalloc)
  return ::mi_realloc(old_ptr, new_size);
#else // defined(TARGET_MALLOC_system)
  ALLOW_C_FUNCTION(::realloc, return ::realloc(old_ptr, new_size);)
#endif
}

static ALWAYSINLINE void* do_calloc(size_t count, size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  ALLOW_C_FUNCTION(::calloc, return ::calloc(count, size);)
#elif defined(TARGET_MALLOC_jemalloc)
  return ::je_calloc(count, size);
#elif defined(TARGET_MALLOC_mimalloc)
  return ::mi_calloc(count, size);
#else // defined(TARGET_MALLOC_system)
  ALLOW_C_FUNCTION(::calloc, return ::calloc(count, size);)
#endif
}

static ALWAYSINLINE void* do_aligned_alloc(size_t alignment, size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  void* ptr;
  int result;
  ALLOW_C_FUNCTION(::posix_memalign, result = ::posix_memalign(&ptr, alignment, size);)
  if (PREDICT_FALSE(result)) {
    ptr = nullptr;
  }
  return ptr;
#elif defined(TARGET_MALLOC_jemalloc)
  void* ptr;
  if (PREDICT_FALSE(::je_posix_memalign(&ptr, alignment, size))) {
    ptr = nullptr;
  }
  return ptr;
#elif defined(TARGET_MALLOC_mimalloc)
  void* ptr;
  if (PREDICT_FALSE(::mi_posix_memalign(&ptr, alignment, size))) {
    ptr = nullptr;
  }
  return ptr;
#else // defined(TARGET_MALLOC_system)
#if defined(_WIN32)
  return ::_aligned_malloc(size, alignment);
#else
  void* ptr;
  int result;
  ALLOW_C_FUNCTION(::posix_memalign, result = ::posix_memalign(&ptr, alignment, size);)
  if (PREDICT_FALSE(result)) {
    ptr = nullptr;
  }
  return ptr;
#endif
#endif
}

static ALWAYSINLINE void do_free(void* ptr) {
#if defined(TARGET_MALLOC_tcmalloc)
  ALLOW_C_FUNCTION(::free, ::free(ptr);)
#elif defined(TARGET_MALLOC_jemalloc)
  ::je_free(ptr);
#elif defined(TARGET_MALLOC_mimalloc)
  ::mi_free(ptr);
#else // defined(TARGET_MALLOC_system)
  ALLOW_C_FUNCTION(::free, ::free(ptr);)
#endif
}

static ALWAYSINLINE void do_free_sized(void* ptr, size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  ALLOW_C_FUNCTION(::free, ::free(ptr);)
#elif defined(TARGET_MALLOC_jemalloc)
  ::je_sdallocx(ptr, size, MALLOCX_LG_ALIGN(log2i_exact(Malloc::min_alignment())));
#elif defined(TARGET_MALLOC_mimalloc)
  ::mi_free_size(ptr, size);
#else // defined(TARGET_MALLOC_system)
  // C23 introduces free_sized but no libc implementations support it yet.
  static_cast<void>(size);
  ALLOW_C_FUNCTION(::free, ::free(ptr);)
#endif
}

static ALWAYSINLINE void do_free_aligned_sized(void* ptr, size_t alignment, size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  ALLOW_C_FUNCTION(::free, ::free(ptr);)
#elif defined(TARGET_MALLOC_jemalloc)
  ::je_sdallocx(ptr, size, MALLOCX_LG_ALIGN(log2i_exact(alignment)));
#elif defined(TARGET_MALLOC_mimalloc)
  ::mi_free_size_aligned(ptr, size, alignment);
#else // defined(TARGET_MALLOC_system)
#if defined(_WIN32)
  static_cast<void>(alignment);
  static_cast<void>(size);
  ::_aligned_free(ptr);
#else
  // C23 introduces free_aligned_sized but no libc implementations support it yet.
  static_cast<void>(size);
  ALLOW_C_FUNCTION(::free, ::free(ptr);)
#endif
#endif
}

static ALWAYSINLINE size_t do_good_size(size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  return ::nallocx(size, MALLOCX_LG_ALIGN(log2i_exact(Malloc::min_alignment())));
#elif defined(TARGET_MALLOC_jemalloc)
  return ::je_nallocx(size, MALLOCX_LG_ALIGN(log2i_exact(Malloc::min_alignment())));
#elif defined(TARGET_MALLOC_mimalloc)
  return ::mi_good_size(size);
#else // defined(TARGET_MALLOC_system)
#if defined(__APPLE__)
  return ::malloc_good_size(size);
#else
  return size;
#endif
#endif
}

static ALWAYSINLINE size_t do_good_size_aligned(size_t alignment, size_t size) {
#if defined(TARGET_MALLOC_tcmalloc)
  return ::nallocx(size, MALLOCX_LG_ALIGN(log2i_exact(alignment)));
#elif defined(TARGET_MALLOC_jemalloc)
  return ::je_nallocx(size, MALLOCX_LG_ALIGN(log2i_exact(alignment)));
#else // defined(TARGET_MALLOC_system)
  static_cast<void>(alignment);
  return size;
#endif
}

static ALWAYSINLINE bool checked_add(size_t x, size_t y, size_t* out) {
#if defined(TARGET_COMPILER_gcc)
  return __builtin_add_overflow(x, y, out);
#else
  if (y != 0 && x > std::numeric_limits<size_t>::max() - y) {
    return true;
  }
  *out = x + y;
  return false;
#endif
}

static ALWAYSINLINE bool checked_multiply(size_t x, size_t y, size_t* out) {
#if defined(TARGET_COMPILER_gcc)
  return __builtin_mul_overflow(x, y, out);
#else
  if (y != 0 && x > std::numeric_limits<size_t>::max() / y) {
    return true;
  }
  *out = x * y;
  return false;
#endif
}

#define set_actual_size_if_not_nullptr(actual_size, size) \
  do {                                                    \
    if ((actual_size) != nullptr) {                       \
      *(actual_size) = (size);                            \
    }                                                     \
  } while (false)

static bool _malloc_initialized = false;
static size_t _malloc_max_alignment = 0;
static void* _malloc_guard_page = nullptr;

void Malloc::initialize() {
  if (_malloc_initialized) {
    return;
  }
  _malloc_max_alignment = []() -> size_t {
#if defined(_WIN32)
    ::SYSTEM_INFO system_info;
    std::memset(&system_info, '\0', sizeof(system_info));
    ::GetSystemInfo(&system_info);
    guarantee(system_info.dwPageSize >= Malloc::min_alignment() &&
              is_power_of_2(system_info.dwPageSize),
              "Invalid page size");
    return system_info.dwPageSize;
#else
    long size = ::sysconf(_SC_PAGESIZE);
    guarantee(size >= 0 &&
              static_cast<unsigned long>(size) >= Malloc::min_alignment()
              && is_power_of_2(size), "Invalid page size");
    return static_cast<unsigned long>(size);
#endif
  }();
  _malloc_guard_page = []() -> void* {
#if defined(_WIN32)
    void* addr = ::VirtualAlloc(nullptr, _malloc_max_alignment, MEM_RESERVE, PAGE_NOACCESS);
    guarantee(addr != nullptr, "Failed to allocate heap guard page");
    return addr;
#else
    int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#if defined(__linux__)
    flags |= MAP_NORESERVE;
#elif defined(__FreeBSD__)
    flags |= MAP_GUARD;
#elif defined(__OpenBSD__)
    flags |= MAP_CONCEAL;
#endif
    void* addr = ::mmap(nullptr, _malloc_max_alignment, PROT_NONE, flags, -1, 0);
    guarantee(addr != MAP_FAILED, "Failed to allocate heap guard page");
#if defined(__DragonFly__)
    ::madvise(addr, _malloc_max_alignment, MADV_FREE | MADV_NOCORE);
#elif defined(__APPLE__) || defined(__NetBSD__)
    ::madvise(addr, _malloc_max_alignment, MADV_FREE);
#endif
    return addr;
#endif
  }();
  _malloc_initialized = true;
}

bool Malloc::is_initialized() {
  return _malloc_initialized;
}

void* Malloc::allocate(size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  if (PREDICT_FALSE(size == 0)) {
    set_actual_size_if_not_nullptr(actual_size, 0);
    return guard_page();
  }
  void* ptr = do_malloc(size);
  if (PREDICT_FALSE(ptr == nullptr)) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(size, OOM_MALLOC_ERROR, "malloc");
    }
    return nullptr;
  }
  assert(is_aligned(ptr, min_alignment()), "under aligned");
  set_actual_size_if_not_nullptr(actual_size, usable_size(ptr, size));
  return ptr;
}

void* Malloc::allocate_zeroed(size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  if (PREDICT_FALSE(size == 0)) {
    set_actual_size_if_not_nullptr(actual_size, 0);
    return guard_page();
  }
  void* ptr = do_calloc(size, 1);
  if (PREDICT_FALSE(ptr == nullptr)) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(size, OOM_MALLOC_ERROR, "calloc");
    }
    return nullptr;
  }
  assert(is_aligned(ptr, min_alignment()), "under aligned");
  set_actual_size_if_not_nullptr(actual_size, usable_size(ptr, size));
  return ptr;
}

void* Malloc::allocate_array(size_t count, size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  size_t total;
  if (PREDICT_FALSE(checked_multiply(count, size, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "malloc");
    }
    return nullptr;
  }
  return allocate(total, alloc_failmode, actual_size);
}

void* Malloc::allocate_array_zeroed(size_t count, size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  size_t total;
  if (PREDICT_FALSE(checked_multiply(count, size, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "calloc");
    }
    return nullptr;
  }
  return allocate_zeroed(total, alloc_failmode, actual_size);
}

void* Malloc::allocate_aligned(size_t alignment, size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  assert(is_power_of_2(alignment), "alignment must be a power of 2");
  assert(alignment <= max_alignment(), "alignment too large");
  if (alignment <= min_alignment()) {
    return allocate(size, actual_size);
  }
  if (PREDICT_FALSE(size == 0)) {
    set_actual_size_if_not_nullptr(actual_size, 0);
    return guard_page();
  }
  void* ptr = do_aligned_alloc(alignment, size);
  if (PREDICT_FALSE(ptr == nullptr)) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(size, OOM_MALLOC_ERROR, "aligned_alloc");
    }
    return nullptr;
  }
  assert(is_aligned(ptr, min_alignment()), "under aligned");
  set_actual_size_if_not_nullptr(actual_size, usable_size(ptr, size));
  return ptr;
}

void* Malloc::reallocate(void* old_ptr, size_t new_size, AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  if (old_ptr == nullptr || old_ptr == guard_page()) {
    return allocate(new_size, alloc_failmode, actual_size);
  }
  if (new_size == 0) {
    deallocate(old_ptr);
    return nullptr;
  }
  void* new_ptr = do_realloc(old_ptr, new_size);
  if (PREDICT_FALSE(new_ptr == nullptr)) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(new_size, OOM_MALLOC_ERROR, "realloc");
    }
    return nullptr;
  }
  assert(is_aligned(new_ptr, min_alignment()), "under aligned");
  set_actual_size_if_not_nullptr(actual_size, usable_size(ptr, size));
  return new_ptr;
}

void* Malloc::reallocate_array(void* old_ptr, size_t new_count, size_t new_size,
                               AllocFailStrategy alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "Malloc not initialized");
  size_t total;
  if (PREDICT_FALSE(checked_multiply(new_count, new_size, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "realloc");
    }
    return nullptr;
  }
  return reallocate(old_ptr, total, actual_size);
}

char* Malloc::duplicate(const char* str, size_t len, AllocFailStrategy alloc_failmode) {
  assert(is_initialized(), "Malloc not initialized");
  size_t total;
  if (PREDICT_FALSE(checked_add(len, 1, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "strdup");
    }
    return nullptr;
  }
  char* new_str = static_cast<char*>(allocate(total, alloc_failmode));
  if (PREDICT_FALSE(new_str == nullptr)) {
    return nullptr;
  }
  std::memcpy(new_str, str, len);
  new_str[len] = '\0';
  return new_str;
}

void Malloc::deallocate(void* ptr) {
  assert(is_initialized(), "Malloc not initialized");
  if (ptr == nullptr || ptr == guard_page()) {
    return;
  }
  assert(is_aligned(ptr, min_alignment()), "under aligned");
  do_free(ptr);
}

void Malloc::deallocate_sized(void* ptr, size_t size) {
  assert(is_initialized(), "Malloc not initialized");
  if (ptr == nullptr || ptr == guard_page()) {
    assert(size == 0, "size must be 0");
    return;
  }
  assert(is_aligned(ptr, min_alignment()), "under aligned");
  do_free_sized(ptr, size);
}

void Malloc::deallocate_aligned_sized(void* ptr, size_t alignment, size_t size) {
  assert(is_initialized(), "Malloc not initialized");
  assert(is_power_of_2(alignment) == 1, "alignment must be a power of 2");
  assert(alignment <= max_alignment(), "alignment too large");
  if (ptr == nullptr || ptr == guard_page()) {
    assert(size == 0, "size must be 0");
    return;
  }
  if (alignment <= min_alignment()) {
    deallocate_sized(ptr, size);
    return;
  }
  assert(is_aligned(ptr, min_alignment()), "under aligned");
  do_free_aligned_sized(ptr, alignment, size);
}

size_t Malloc::good_size(size_t size) {
  return MAX2(do_good_size(size), size);
}

bool Malloc::trim() {
  assert(is_initialized(), "Malloc not initialized");
#if defined(TARGET_MALLOC_tcmalloc)
  ::tcmalloc::MallocExtension::ReleaseMemoryToSystem(std::numeric_limits<size_t>::max());
  return true;
#elif defined(TARGET_MALLOC_mimalloc)
  ::mi_collect(/*force*/ false);
  return true;
#else // defined(TARGET_MALLOC_system)
#if defined(__GLIBC__)
  return ::malloc_trim(0) != 0;
#elif defined(_WIN32)
  return ::_heapmin() == 0;
#else
  // Not supported.
  return false;
#endif
#endif
}

bool Malloc::mark_thread_idle() {
  assert(is_initialized(), "Malloc not initialized");
#if defined(TARGET_MALLOC_tcmalloc)
  ::tcmalloc::MallocExtension::MarkThreadIdle();
  return true;
#elif defined(TARGET_MALLOC_jemalloc)
  ::je_mallctl("thread.idle", nullptr, nullptr, nullptr, 0);
  return false;
#elif defined(__FreeBSD__)
  ::mallctl("thread.idle", nullptr, nullptr, nullptr, 0);
  return false;
#else
  // Unsupported.
  return false;
#endif
}

void Malloc::mark_thread_busy() {
  assert(is_initialized(), "Malloc not initialized");
#if defined(TARGET_MALLOC_tcmalloc)
  ::tcmalloc::MallocExtension::MarkThreadBusy();
#else
  // Unsupported.
#endif
}

size_t Malloc::good_size_aligned(size_t alignment, size_t size) {
  assert(is_initialized(), "Malloc not initialized");
  assert(is_power_of_2(alignment) == 1, "alignment must be a power of 2");
  assert(alignment <= max_alignment(), "alignment too large");
  if (alignment <= min_alignment()) {
    return good_size(size);
  }
  return MAX2(do_good_size_aligned(alignment, size), size);
}

size_t Malloc::max_alignment() {
  assert(is_initialized(), "Malloc not initialized");
  return _malloc_max_alignment;
}

void* Malloc::guard_page() {
  assert(is_initialized(), "Malloc not initialized");
  return _malloc_guard_page;
}
