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

#ifndef SHARE_MEMORY_MALLOC_HPP
#define SHARE_MEMORY_MALLOC_HPP

#include "memory/allocation.hpp"
#include "memory/allStatic.hpp"
#include "utilities/globalDefinitions.hpp"

#include <cstddef>
#include <cstring>

class Arguments;
class CHeap;
class PreprocessedArguments;
class ClassLoader;

class Malloc final : public AllStatic {
 private:
  // Direct access to low-level memory management routines is restricted to support abstractions
  // such as NMT. Only a small subset of Hotspot actually needs access to these abstractions
  // directly.
  friend class PreprocessedArguments;
  friend class Arguments;
  friend class CHeap;
  friend class ClassLoader;

  // Normal allocations.
  //
  // All allocations are aligned to at least min_alignment().

  static void* allocate(size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* allocate(size_t size, AllocFailStrategy alloc_failmode) {
    return allocate(size, alloc_failmode, nullptr);
  }
  static inline void* allocate(size_t size, size_t* actual_size) {
    return allocate(size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* allocate(size_t size) {
    return allocate(size, AllocFailStrategy::RETURN_NULL, nullptr);
  }

  static void* allocate_zeroed(size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* allocate_zeroed(size_t size, AllocFailStrategy alloc_failmode) {
    return allocate_zeroed(size, alloc_failmode, nullptr);
  }
  static inline void* allocate_zeroed(size_t size, size_t* actual_size) {
    return allocate_zeroed(size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* allocate_zeroed(size_t size) {
    return allocate_zeroed(size, AllocFailStrategy::RETURN_NULL, nullptr);
  }


  static void* allocate_array(size_t count, size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* allocate_array(size_t count, size_t size, AllocFailStrategy alloc_failmode) {
    return allocate_array(count, size, alloc_failmode, nullptr);
  }
  static inline void* allocate_array(size_t count, size_t size, size_t* actual_size) {
    return allocate_array(count, size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* allocate_array(size_t count, size_t size) {
    return allocate_array(count, size, AllocFailStrategy::RETURN_NULL, nullptr);
  }

  static void* allocate_array_zeroed(size_t count, size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* allocate_array_zeroed(size_t count, size_t size, AllocFailStrategy alloc_failmode) {
    return allocate_array_zeroed(count, size, alloc_failmode, nullptr);
  }
  static inline void* allocate_array_zeroed(size_t count, size_t size, size_t* actual_size) {
    return allocate_array_zeroed(count, size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* allocate_array_zeroed(size_t count, size_t size) {
    return allocate_array_zeroed(count, size, AllocFailStrategy::RETURN_NULL, nullptr);
  }

  static void* reallocate(void* old_ptr, size_t new_size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* reallocate(void* old_ptr, size_t new_size, AllocFailStrategy alloc_failmode) {
    return reallocate(old_ptr, new_size, alloc_failmode, nullptr);
  }
  static inline void* reallocate(void* old_ptr, size_t new_size, size_t* actual_size) {
    return reallocate(old_ptr, new_size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* reallocate(void* old_ptr, size_t new_size) {
    return reallocate(old_ptr, new_size, AllocFailStrategy::RETURN_NULL, nullptr);
  }

  static void* reallocate_array(void* old_ptr, size_t new_count, size_t new_size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* reallocate_array(void* old_ptr, size_t new_count, size_t new_size, AllocFailStrategy alloc_failmode) {
    return reallocate_array(old_ptr, new_count, new_size, alloc_failmode, nullptr);
  }
  static inline void* reallocate_array(void* old_ptr, size_t new_count, size_t new_size, size_t* actual_size) {
    return reallocate_array(old_ptr, new_count, new_size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* reallocate_array(void* old_ptr, size_t new_count, size_t new_size) {
    return reallocate_array(old_ptr, new_count, new_size, AllocFailStrategy::RETURN_NULL, nullptr);
  }

  static char* duplicate(const char* str, size_t len, AllocFailStrategy alloc_failmode);
  static inline char* duplicate(const char* str, size_t len) {
    return duplicate(str, len, AllocFailStrategy::RETURN_NULL);
  }
  static inline char* duplicate(const char* str, AllocFailStrategy alloc_failmode) {
    return duplicate(str, strlen(str), alloc_failmode);
  }
  static inline char* duplicate(const char* str) {
    return duplicate(str, strlen(str), AllocFailStrategy::RETURN_NULL);
  }

  static void deallocate(void* ptr);

  static void deallocate_sized(void* ptr, size_t size);

  static size_t good_size(size_t size);

  // Over-aligned allocations.
  //
  // All memory allocated via allocate_aligned() MUST be deallocated using
  // deallocate_aligned_sized(), NOT deallocate or deallocate_sized. The size and alignment given to
  // deallocate_aligned_sized() MUST match allocate_aligned().

  static void* allocate_aligned(size_t alignment, size_t size, AllocFailStrategy alloc_failmode, size_t* actual_size);
  static inline void* allocate_aligned(size_t alignment, size_t size, AllocFailStrategy alloc_failmode) {
    return allocate_aligned(alignment, size, alloc_failmode, nullptr);
  }
  static inline void* allocate_aligned(size_t alignment, size_t size, size_t* actual_size) {
    return allocate_aligned(alignment, size, AllocFailStrategy::RETURN_NULL, actual_size);
  }
  static inline void* allocate_aligned(size_t alignment, size_t size) {
    return allocate_aligned(alignment, size, AllocFailStrategy::RETURN_NULL, nullptr);
  }

  static void deallocate_aligned_sized(void* ptr, size_t alignment, size_t size);

  static size_t good_size_aligned(size_t alignment, size_t size);

  // Miscellaneous.

  static bool trim();

  static bool mark_thread_idle();

  static void mark_thread_busy();

 public:
  static void initialize();

  static bool is_initialized();

  // Gets the minimum alignment returned by all malloc-based allocations.
  static constexpr size_t min_alignment() { return alignof(std::max_align_t); }

  // Gets the maximum supported alignment. Attempting to allocate malloc-based memory with
  // alignments greater than this is undefined behavior.
  ATTRIBUTE_PURE static size_t max_alignment();

  static inline size_t page_size() { return max_alignment(); }

  // When zero-sized allocations are requested, implementations are free to either return NULL or a
  // unique address. Hotspot has historically enforced the latter by bumping zero sized allocations
  // to 1 byte. To maintain backwards compatibility we instead return this address for zero sized
  // allocations. This address points to a location in memory which is not readable, writable, or
  // executable and attempting to access it will likely result in SIGSEGV.
  ATTRIBUTE_PURE static void* guard_page();
};

#endif  // SHARE_MEMORY_MALLOC_HPP
