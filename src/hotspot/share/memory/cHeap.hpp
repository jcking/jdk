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

#ifndef SHARE_MEMORY_CHEAP_HPP
#define SHARE_MEMORY_CHEAP_HPP

#include "memory/allocation.hpp"
#include "memory/allStatic.hpp"
#include "services/nmtCommon.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"

#include <cstddef>
#include <type_traits>

class PreprocessedArgument;

class CHeap final : public AllStatic {
 public:
  static jint initialize(const char* native_memory_tracking_flag, const char* malloc_limit_flag);

  static bool is_initialized();

  // Normal allocations.

  static void* allocate(size_t size, MEMFLAGS flag,
                        AllocFailType alloc_failmodes,
                        size_t* actual_size = nullptr);

  static void* allocate(size_t size, MEMFLAGS flag,
                        AllocFailType alloc_failmodes,
                        const NativeCallStack& stack,
                        size_t* actual_size = nullptr);

  static void* allocate_zeroed(size_t size, MEMFLAGS flags,
                               AllocFailType alloc_failmode,
                               size_t* actual_size = nullptr);

  static void* allocate_zeroed(size_t size, MEMFLAGS flags,
                               AllocFailType alloc_failmode,
                               const NativeCallStack& stack,
                               size_t* actual_size = nullptr);

  static void* allocate_array(size_t count, size_t size, MEMFLAGS flags,
                              AllocFailType alloc_failmode,
                              size_t* actual_size = nullptr);

  static void* allocate_array(size_t count, size_t size, MEMFLAGS flags,
                              AllocFailType alloc_failmode,
                              const NativeCallStack& stack,
                              size_t* actual_size = nullptr);

  static void* allocate_array_zeroed(size_t count, size_t size, MEMFLAGS flags,
                                     AllocFailType alloc_failmode,
                                     size_t* actual_size = nullptr);

  static void* allocate_array_zeroed(size_t count, size_t size, MEMFLAGS flags,
                                     AllocFailType alloc_failmode,
                                     const NativeCallStack& stack,
                                     size_t* actual_size = nullptr);

  static void* reallocate(void* old_ptr, size_t new_size, MEMFLAGS flags,
                          AllocFailType alloc_failmode,
                          size_t* actual_size = nullptr);

  static void* reallocate(void* old_ptr, size_t new_size, MEMFLAGS flags,
                          AllocFailType alloc_failmode,
                          const NativeCallStack& stack,
                          size_t* actual_size = nullptr);

  static void* reallocate_array(void* old_ptr, size_t new_count,
                                size_t new_size, MEMFLAGS flags,
                                AllocFailType alloc_failmode,
                                size_t* actual_size = nullptr);

  static void* reallocate_array(void* old_ptr, size_t new_count,
                                size_t new_size, MEMFLAGS flags,
                                AllocFailType alloc_failmode,
                                const NativeCallStack& stack,
                                size_t* actual_size = nullptr);

  static void deallocate(void* ptr);

  static void deallocate_sized(void* ptr, size_t size);

  static size_t good_size(size_t size);

  // Over-aligned allocations.

  static void* allocate_aligned(size_t alignment, size_t size, MEMFLAGS flags,
                                AllocFailType alloc_failmode,
                                size_t* actual_size = nullptr);

  static void* allocate_aligned(size_t alignment, size_t size, MEMFLAGS flags,
                                AllocFailType alloc_failmode,
                                const NativeCallStack& stack,
                                size_t* actual_size = nullptr);

  static void deallocate_aligned_sized(void* ptr, size_t alignment, size_t size);

  static size_t good_size_aligned(size_t alignment, size_t size);

  // Miscellaneous.

  static bool trim();

  static bool mark_thread_idle();

  static void mark_thread_busy();

  static constexpr size_t min_alignment() { return alignof(std::max_align_t); }

  ATTRIBUTE_PURE static size_t max_alignment();

  static size_t page_size() { return max_alignment(); }

  ATTRIBUTE_PURE static void* guard_page();

 private:
  union alignas(std::max_align_t) MallocHeader final {
    MallocHeader(size_t size, uint32_t marker, MEMFLAGS flags)
        : size(size),
          marker(marker),
          flags(flags) {}

    struct {
      const size_t size;
      const uint32_t marker;
      const MEMFLAGS flags;
    };
    uint8_t padding[alignof(std::max_align_t)];
  };

  struct MallocFooter final {
    MallocFooter(size_t size, uint32_t marker, MEMFLAGS flags)
        : size(size),
          marker(marker),
          flags(flags) {}

    const size_t size;
    const uint32_t marker;
    const MEMFLAGS flags;
  };

  STATIC_ASSERT(std::is_trivially_destructible<MallocHeader>::value);

  STATIC_ASSERT(std::is_trivially_destructible<MallocFooter>::value);

  static void* do_allocate(size_t size, MEMFLAGS flags, AllocFailType alloc_failmode,
                           NMT_TrackingLevel level, const NativeCallStack& stack,
                           size_t* actual_size);

  static void* do_allocate_zeroed(size_t size, MEMFLAGS flags, AllocFailType alloc_failmode,
                                  NMT_TrackingLevel level, const NativeCallStack& stack,
                                  size_t* actual_size);

  static void* do_allocate_array(size_t count, size_t size, MEMFLAGS flags,
                                 AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                 const NativeCallStack& stack, size_t* actual_size);

  static void* do_allocate_array_zeroed(size_t count, size_t size, MEMFLAGS flags,
                                        AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                        const NativeCallStack& stack, size_t* actual_size);

  static void* do_reallocate(void* old_ptr, size_t new_size, MEMFLAGS flags,
                             AllocFailType alloc_failmode, NMT_TrackingLevel level,
                             const NativeCallStack& stack, size_t* actual_size);

  static void* do_reallocate_array(void* old_ptr, size_t new_count, size_t new_size, MEMFLAGS flags,
                                   AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                   const NativeCallStack& stack, size_t* actual_size);

  static void* do_allocate_aligned(size_t alignment, size_t size, MEMFLAGS flags,
                                   AllocFailType alloc_failmode, NMT_TrackingLevel level,
                                   const NativeCallStack& stack, size_t* actual_size);

  static void do_deallocate(void* ptr, NMT_TrackingLevel level);
};

#endif  // SHARE_MEMORY_CHEAP_HPP
