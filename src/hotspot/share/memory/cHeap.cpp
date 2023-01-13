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

#include "memory/cHeap.hpp"

#include "jvm.h"
#include "memory/malloc.hpp"
#include "runtime/arguments.hpp"
#include "runtime/globals.hpp"
#include "services/memTracker.hpp"
#include "services/mallocTracker.hpp"
#include "services/mallocSiteTable.hpp"
#include "services/nmtCommon.hpp"
#include "utilities/align.hpp"
#include "utilities/debug.hpp"
#include "utilities/defaultStream.hpp"
#include "utilities/population_count.hpp"

#include <type_traits>

#if defined(__has_builtin)
#define HAS_BUILTIN(x) __has_builtin(x)
#else
#define HAS_BUILTIN(x) 0
#endif

static bool checked_add(size_t x, size_t y, size_t* out) {
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_add_overflow)
  return __builtin_add_overflow(x, y, out);
#else
  if (y != 0 && x > std::numeric_limits<size_t>::max() - y) {
    return true;
  }
  *out = x + y;
  return false;
#endif
}

static bool checked_multiply(size_t x, size_t y, size_t* out) {
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_mul_overflow)
  return __builtin_mul_overflow(x, y, out);
#else
  if (y != 0 && x > std::numeric_limits<size_t>::max() / y) {
    return true;
  }
  *out = x * y;
  return false;
#endif
}

static bool checked_align_up(size_t x, size_t y, size_t* out) {
  size_t z;
  if (checked_add(x, y, &z)) {
    return true;
  }
  return (z - size_t{1}) & ~(y - size_t{1});
}

void* CHeap::do_allocate(size_t size, MEMFLAGS flags, AllocFailType alloc_failmode,
                         NMT_TrackingLevel level, const NativeCallStack& stack,
                         size_t* actual_size) {
  switch (level) {
  case NMT_off:
    return Malloc::allocate(size, alloc_failmode, actual_size);
  case NMT_summary:
  case NMT_detail: {
    size_t outer_size;
    if (PREDICT_FALSE(checked_add(size, sizeof(MallocHeader), &outer_size))) {
      if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
        vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "");
      }
      return nullptr;
    }
    size_t actual_outer_size = outer_size;
    void* ptr = Malloc::allocate(outer_size, alloc_failmode, actual_size != nullptr ? &actual_outer_size : nullptr);
    if (PREDICT_FALSE(ptr == nullptr)) {
      return nullptr;
    }
    MallocMemorySummary::record_malloc(actual_outer_size, flags);
    uint32_t marker = 0;
    if (level == NMT_detail) {
      MallocSiteTable::allocation_at(stack, actual_outer_size, &marker, flags);
    }
    MallocHeader* header = ::new (ptr) MallocHeader(actual_outer_size, marker, flags);
    if (actual_size != nullptr) {
      *actual_size = actual_outer_size - sizeof(MallocHeader);
    }
    return header + 1;
  }
  default:
    ShouldNotReachHere();
    return nullptr;
  }
}

void* CHeap::do_allocate_zeroed(size_t size, MEMFLAGS flags, AllocFailType alloc_failmode,
                                NMT_TrackingLevel level, const NativeCallStack& stack,
                                size_t* actual_size) {
  switch (level) {
  case NMT_off:
    return Malloc::allocate_zeroed(size, alloc_failmode, actual_size);
  case NMT_summary:
  case NMT_detail: {
    size_t outer_size;
    if (PREDICT_FALSE(checked_add(size, sizeof(MallocHeader), &outer_size))) {
      if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
        vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "calloc");
      }
      return nullptr;
    }
    size_t actual_outer_size = outer_size;
    void* ptr = Malloc::allocate_zeroed(outer_size, alloc_failmode,
                                      actual_size != nullptr ? &actual_outer_size : nullptr);
    if (PREDICT_FALSE(ptr == nullptr)) {
      return nullptr;
    }
    MallocMemorySummary::record_malloc(actual_outer_size, flags);
    uint32_t marker = 0;
    if (level == NMT_detail) {
      MallocSiteTable::allocation_at(stack, actual_outer_size, &marker, flags);
    }
    MallocHeader* header = ::new (ptr) MallocHeader(actual_outer_size, marker, flags);
    if (actual_size != nullptr) {
      *actual_size = actual_outer_size - sizeof(MallocHeader);
    }
    return header + 1;
  }
  default:
    ShouldNotReachHere();
    return nullptr;
  }
}

void* CHeap::do_allocate_array(size_t count, size_t size, MEMFLAGS flags,
                               AllocFailType alloc_failmode, NMT_TrackingLevel level,
                               const NativeCallStack& stack, size_t* actual_size) {
  size_t total;
  if (PREDICT_FALSE(checked_multiply(count, size, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "malloc");
    }
    return nullptr;
  }
  return do_allocate(total, flags, alloc_failmode, level, stack, actual_size);
}

void* CHeap::do_allocate_array_zeroed(size_t count, size_t size, MEMFLAGS flags,
                                      AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                      const NativeCallStack& stack, size_t* actual_size) {
  size_t total;
  if (PREDICT_FALSE(checked_multiply(count, size, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "calloc");
    }
    return nullptr;
  }
  return do_allocate_zeroed(total, flags, alloc_failmode, level, stack, actual_size);
}

void* CHeap::do_reallocate(void* old_ptr, size_t new_size, MEMFLAGS flags,
                           AllocFailType alloc_failmode, NMT_TrackingLevel level,
                           const NativeCallStack& stack, size_t* actual_size) {
  switch (level) {
  case NMT_off:
    return Malloc::reallocate(old_ptr, new_size, alloc_failmode, actual_size);
  case NMT_summary:
  case NMT_detail: {
    if (old_ptr == nullptr || old_ptr == Malloc::guard_page()) {
      return do_allocate(new_size, flags, alloc_failmode, level, stack, actual_size);
    }
    if (new_size == 0) {
      do_deallocate(old_ptr, level);
      return nullptr;
    }
    MallocHeader* old_header = static_cast<MallocHeader*>(old_ptr) - 1;
    MallocHeader old_header_copy = *old_header;
    size_t outer_size;
    if (PREDICT_FALSE(checked_add(new_size, sizeof(MallocHeader), &outer_size))) {
      if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
        vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "realloc");
      }
      return nullptr;
    }
    size_t actual_outer_size = outer_size;
    void* new_ptr = Malloc::reallocate(old_header, outer_size, alloc_failmode,
                                       actual_size != nullptr ? &actual_outer_size : nullptr);
    if (PREDICT_FALSE(new_ptr == nullptr)) {
      return nullptr;
    }
    MallocMemorySummary::record_malloc(actual_outer_size, flags);
    uint32_t marker = 0;
    if (level == NMT_detail) {
      MallocSiteTable::allocation_at(stack, actual_outer_size, &marker, flags);
    }
    MallocMemorySummary::record_free(old_header_copy.size, old_header_copy.flags);
    if (level == NMT_detail) {
      MallocSiteTable::deallocation_at(old_header_copy.size, old_header_copy.marker);
    }
    MallocHeader* new_header = ::new (new_ptr) MallocHeader(actual_outer_size, marker, flags);
    if (actual_size != nullptr) {
      *actual_size = actual_outer_size - sizeof(MallocHeader);
    }
    return new_header + 1;
  }
  default:
    ShouldNotReachHere();
    return nullptr;
  }
}

void* CHeap::do_reallocate_array(void* old_ptr, size_t new_count, size_t new_size, MEMFLAGS flags,
                                 AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                 const NativeCallStack& stack, size_t* actual_size) {
  size_t total;
  if (PREDICT_FALSE(checked_multiply(new_count, new_size, &total))) {
    if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
      vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "realloc");
    }
    return nullptr;
  }
  return do_reallocate(old_ptr, total, flags, alloc_failmode, level, stack, actual_size);
}

void* CHeap::do_allocate_aligned(size_t alignment, size_t size, MEMFLAGS flags,
                                 AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                 const NativeCallStack& stack, size_t* actual_size) {
  switch (level) {
  case NMT_off:
    return Malloc::allocate_aligned(alignment, size, alloc_failmode, actual_size);
  case NMT_summary:
  case NMT_detail: {
    if (alignment <= Malloc::min_alignment()) {
      assert(population_count(alignment) == 1, "alignment must be a power of 2");
      return do_allocate(size, flags, alloc_failmode, level, stack, actual_size);
    }
    if (size == 0) {
      if (actual_size != nullptr) {
        *actual_size = 0;
      }
      return Malloc::guard_page();
    }
    size_t outer_size;
    if (PREDICT_FALSE(checked_align_up(size, alignof(MallocFooter), &outer_size) ||
                      checked_add(outer_size, sizeof(MallocFooter), &outer_size))) {
      if (alloc_failmode == AllocFailStrategy::EXIT_OOM) {
        vm_exit_out_of_memory(std::numeric_limits<size_t>::max(), OOM_MALLOC_ERROR, "aligned_alloc");
      }
      return nullptr;
    }
    size_t actual_outer_size = outer_size;
    void* ptr = Malloc::allocate_aligned(alignment, outer_size, alloc_failmode,
                                         actual_size != nullptr ? &actual_outer_size : nullptr);
    if (PREDICT_FALSE(ptr == nullptr)) {
      return nullptr;
    }
    MallocMemorySummary::record_malloc(actual_outer_size, flags);
    uint32_t marker = 0;
    if (level == NMT_detail) {
      MallocSiteTable::allocation_at(stack, actual_outer_size, &marker, flags);
    }
    size_t inner_size = align_down((actual_outer_size - sizeof(MallocFooter)),
                                   alignof(MallocFooter));
    ::new (static_cast<void*>(static_cast<uint8_t*>(ptr) + inner_size))
            MallocFooter(actual_outer_size, marker, flags);
    if (actual_size != nullptr) {
      *actual_size = inner_size;
    } else {
      assert(inner_size == align_up(size, alignof(MallocFooter)), "invariant");
    }
    return ptr;
  }
  default:
    ShouldNotReachHere();
    return nullptr;
  }
}

void CHeap::do_deallocate(void* ptr, NMT_TrackingLevel level) {
  switch (level) {
  case NMT_off:
    Malloc::deallocate(ptr);
    return;
  case NMT_summary:
  case NMT_detail: {
    if (ptr == nullptr || ptr == Malloc::guard_page()) {
      return;
    }
    MallocHeader* header = static_cast<MallocHeader*>(ptr) - 1;
    MallocMemorySummary::record_free(header->size, header->flags);
    if (level == NMT_detail) {
      MallocSiteTable::deallocation_at(header->size, header->marker);
    }
    Malloc::deallocate_sized(header, header->size);
    return;
  }
  default:
    ShouldNotReachHere();
  }
}

static bool _cheap_initialized = false;
static NMT_TrackingLevel _cheap_native_memory_tracking = NMT_unknown;
static MallocLimits _cheap_malloc_limit = {0, {0}};

jint CHeap::initialize(const char* native_memory_tracking_flag, const char* malloc_limit_flag) {
  NMT_TrackingLevel native_memory_tracking;
  MallocLimits malloc_limit;
  native_memory_tracking = NMTUtil::parse_tracking_level(native_memory_tracking_flag);
  if (native_memory_tracking == NMT_unknown) {
    jio_fprintf(defaultStream::error_stream(),
                "Syntax error, expecting -XX:NativeMemoryTracking=[off|summary|detail]");
    return JNI_ERR;
  }
  if (!Arguments::parse_malloc_limits(malloc_limit_flag, &malloc_limit)) {
    jio_fprintf(defaultStream::error_stream(),
                "Syntax error, expecting -XX:MallocLimit=[<size>|<category>:<size>...]");
    return JNI_ERR;
  }

  if (_cheap_initialized) {
    // We were already successfully initialized once. It is not possible to change the tracking
    // level or limits after the fact, so we just ensure they match the existing ones or return an
    // error.
    if (native_memory_tracking != _cheap_native_memory_tracking ||
        malloc_limit != _cheap_malloc_limit) {
      jio_fprintf(defaultStream::error_stream(),
                  "Precondition error, attempting to initialize multiple times with different "
                  "effective -XX:NativeMemoryTracking or -XX:MallocLimit");
      return JNI_ERR;
    }
    return JNI_OK;
  }

  _cheap_native_memory_tracking = native_memory_tracking;
  _cheap_malloc_limit = malloc_limit;
  MemTracker::initialize(native_memory_tracking, malloc_limit);
  _cheap_initialized = true;
  return JNI_OK;
}

bool CHeap::is_initialized() {
  return _cheap_initialized;
}

void* CHeap::allocate(size_t size, MEMFLAGS flags, AllocFailType alloc_failmode,
                      size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_allocate(size, flags, alloc_failmode, level,
                     PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                     actual_size);
}

void* CHeap::allocate(size_t size, MEMFLAGS flags, AllocFailType alloc_failmode,
                      const NativeCallStack& stack, size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_allocate(size, flags, alloc_failmode, MemTracker::tracking_level(),
                     stack, actual_size);
}

void* CHeap::allocate_zeroed(size_t size, MEMFLAGS flags,
                             AllocFailType alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_allocate_zeroed(size, flags, alloc_failmode, level,
                            PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                            actual_size);
}

void* CHeap::allocate_zeroed(size_t size, MEMFLAGS flags,
                           AllocFailType alloc_failmode,
                           const NativeCallStack& stack,
                           size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_allocate_zeroed(size, flags, alloc_failmode, MemTracker::tracking_level(),
                            stack, actual_size);
}

void* CHeap::allocate_array(size_t count, size_t size, MEMFLAGS flags,
                            AllocFailType alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_allocate_array(count, size, flags, alloc_failmode, level,
                           PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                           actual_size);
}

void* CHeap::allocate_array(size_t count, size_t size, MEMFLAGS flags,
                            AllocFailType alloc_failmode,
                            const NativeCallStack& stack,
                            size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_allocate_array(count, size, flags, alloc_failmode, MemTracker::tracking_level(),
                           stack, actual_size);
}

void* CHeap::allocate_array_zeroed(size_t count, size_t size, MEMFLAGS flags,
                                   AllocFailType alloc_failmode,
                                   size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_allocate_array_zeroed(count, size, flags, alloc_failmode, level,
                                  PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                                  actual_size);
}

void* CHeap::allocate_array_zeroed(size_t count, size_t size, MEMFLAGS flags,
                                   AllocFailType alloc_failmode,
                                   const NativeCallStack& stack,
                                   size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_allocate_array_zeroed(count, size, flags, alloc_failmode, MemTracker::tracking_level(),
                                  stack, actual_size);
}

void* CHeap::reallocate(void* old_ptr, size_t new_size, MEMFLAGS flags,
                        AllocFailType alloc_failmode, size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_reallocate(old_ptr, new_size, flags, alloc_failmode, level,
                       PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                       actual_size);
}

void* CHeap::reallocate(void* old_ptr, size_t new_size, MEMFLAGS flags,
                        AllocFailType alloc_failmode, const NativeCallStack& stack,
                        size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_reallocate(old_ptr, new_size, flags, alloc_failmode, MemTracker::tracking_level(),
                       stack, actual_size);
}

void* CHeap::reallocate_array(void* old_ptr, size_t new_count, size_t new_size,
                              MEMFLAGS flags, AllocFailType alloc_failmode,
                              size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_reallocate_array(old_ptr, new_count, new_size, flags, alloc_failmode, level,
                             PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                             actual_size);
}

void* CHeap::reallocate_array(void* old_ptr, size_t new_count, size_t new_size,
                              MEMFLAGS flags, AllocFailType alloc_failmode,
                              const NativeCallStack& stack,
                              size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_reallocate_array(old_ptr, new_count, new_size, flags, alloc_failmode,
                             MemTracker::tracking_level(), stack, actual_size);
}

void CHeap::deallocate(void* ptr) {
  assert(is_initialized(), "NMT must already be initialized");
  do_deallocate(ptr, MemTracker::tracking_level());
}

void CHeap::deallocate_sized(void* ptr, size_t size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  switch (level) {
  case NMT_off:
    Malloc::deallocate_sized(ptr, size);
    return;
  case NMT_summary:
  case NMT_detail: {
    if (ptr == nullptr || ptr == Malloc::guard_page()) {
      assert(size == 0, "size mismatch");
      return;
    }
    assert(size != 0, "size mismatch");
    MallocHeader* header = static_cast<MallocHeader*>(ptr) - 1;
    assert(header->size - sizeof(MallocHeader) == size, "size mismatch");
    MallocMemorySummary::record_free(header->size, header->flags);
    if (level == NMT_detail) {
      MallocSiteTable::deallocation_at(header->size, header->marker);
    }
    Malloc::deallocate_sized(header, header->size);
    return;
  }
  default:
    ShouldNotReachHere();
  }
}

void* CHeap::allocate_aligned(size_t alignment, size_t size, MEMFLAGS flags,
                              AllocFailType alloc_failmode,
                              size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  return do_allocate_aligned(alignment, size, flags, alloc_failmode, level,
                             PREDICT_FALSE(level == NMT_detail) ? NativeCallStack(1) : FAKE_CALLSTACK,
                             actual_size);
}

void* CHeap::allocate_aligned(size_t alignment, size_t size, MEMFLAGS flags,
                              AllocFailType alloc_failmode,
                              const NativeCallStack& stack,
                              size_t* actual_size) {
  assert(is_initialized(), "NMT must already be initialized");
  return do_allocate_aligned(alignment, size, flags, alloc_failmode, MemTracker::tracking_level(),
                             stack, actual_size);
}

void CHeap::deallocate_aligned_sized(void* ptr, size_t alignment, size_t size) {
  assert(is_initialized(), "NMT must already be initialized");
  NMT_TrackingLevel level = MemTracker::tracking_level();
  switch (level) {
  case NMT_off:
    Malloc::deallocate_aligned_sized(ptr, alignment, size);
    return;
  case NMT_summary:
  case NMT_detail: {
    assert(population_count(alignment) == 1, "alignment must be a power of 2");
    if (alignment <= Malloc::min_alignment()) {
      Malloc::deallocate_sized(ptr, size);
      return;
    }
    if (ptr == nullptr || ptr == Malloc::guard_page()) {
      assert(size == 0, "size mismatch");
      return;
    }
    assert(size != 0, "size mismatch");
    size_t inner_size = align_up(size, alignof(MallocFooter));
    MallocFooter* footer = static_cast<MallocFooter*>(static_cast<void*>(static_cast<uint8_t*>(ptr) +
                                                                         inner_size));
    assert(footer->size - sizeof(MallocFooter) == inner_size, "size mismatch");
    MallocMemorySummary::record_free(footer->size, footer->flags);
    if (level == NMT_detail) {
      MallocSiteTable::deallocation_at(footer->size, footer->marker);
    }
    Malloc::deallocate_aligned_sized(ptr, alignment, footer->size);
    return;
  }
  default:
    ShouldNotReachHere();
  }
}

size_t CHeap::good_size_aligned(size_t alignment, size_t size) {
  assert(is_initialized(), "NMT must already be initialized");
  if (PREDICT_FALSE(MemTracker::enabled())) {
    assert(population_count(alignment) == 1, "alignment must be a power of 2");
    if (alignment <= Malloc::min_alignment()) {
      size_t total;
      if (PREDICT_FALSE(checked_add(size, sizeof(MallocHeader), &total))) {
        return size;
      }
      return Malloc::good_size(total) - sizeof(MallocHeader);
    }
    size_t total;
    if (PREDICT_FALSE(checked_align_up(size, alignof(MallocFooter), &total) ||
                      checked_add(total, sizeof(MallocFooter), &total))) {
      return size;
    }
    return align_down(Malloc::good_size_aligned(alignment, total) - sizeof(MallocFooter),
                      alignof(MallocFooter));
  }
  return Malloc::good_size_aligned(alignment, size);
}

size_t CHeap::good_size(size_t size) {
  assert(is_initialized(), "NMT must already be initialized");
  if (PREDICT_FALSE(MemTracker::enabled())) {
    size_t total;
    if (PREDICT_FALSE(checked_add(size, sizeof(MallocHeader), &total))) {
      return size;
    }
    return Malloc::good_size(total) - sizeof(MallocHeader);
  }
  return Malloc::good_size(size);
}

bool CHeap::trim() { return Malloc::trim(); }

bool CHeap::mark_thread_idle() { return Malloc::mark_thread_idle(); }

void CHeap::mark_thread_busy() { Malloc::mark_thread_busy(); }

size_t CHeap::max_alignment() {
  STATIC_ASSERT(CHeap::min_alignment() == Malloc::min_alignment());
  return Malloc::max_alignment();
}

void* CHeap::guard_page() { return Malloc::guard_page(); }
