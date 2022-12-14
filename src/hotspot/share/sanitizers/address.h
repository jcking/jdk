/*
 * Copyright (c) 2022, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_SANITIZERS_ADDRESS_HPP
#define SHARE_SANITIZERS_ADDRESS_HPP

#include <stdio.h>

#ifdef ADDRESS_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

// NO_SANITIZE_ADDRESS
//
// Function attribute that can be applied to disable ASan instrumentation for
// the function.
#ifdef ADDRESS_SANITIZER
#define NO_SANITIE_ADDRESS __attribute__((no_sanitize("address")))
#else
#define NO_SANITIE_ADDRESS
#endif

// ASAN_POISON_MEMORY_REGION()
//
// Poisons the specified memory region. Subsequent reads and writes to the
// memory region will result in a fatal error.
#ifdef ADDRESS_SANITIZER
#undef ASAN_POISON_MEMORY_REGION
#define ASAN_POISON_MEMORY_REGION(addr, size)                               \
  __asan_poison_memory_region((addr), (size));                              \
  if (!__asan_report_present()) {                                           \
    fprintf(stderr,                                                         \
            "%s:%d: poisoning memory region [0x%" PRIxPTR "..0x%" PRIxPTR   \
            ")\n",                                                          \
            __FILE__, __LINE__, reinterpret_cast<uintptr_t>(addr),          \
            reinterpret_cast<uintptr_t>(addr) + static_cast<size_t>(size)); \
    fflush(stderr);                                                         \
  }
#else
#define ASAN_POISON_MEMORY_REGION(addr, size) \
  do {                                        \
    static_cast<void>(addr);                  \
    static_cast<void>(size);                  \
  } while (false)
#endif

// ASAN_UNPOISON_MEMORY_REGION()
//
// Unpoisons the specified memory region. Subsequent reads and writes to the
// memory region are valid.
#ifdef ADDRESS_SANITIZER
#undef ASAN_UNPOISON_MEMORY_REGION
#define ASAN_UNPOISON_MEMORY_REGION(addr, size)                             \
  __asan_unpoison_memory_region((addr), (size));                            \
  if (!__asan_report_present()) {                                           \
    fprintf(stderr,                                                         \
            "%s:%d: unpoisoning memory region [0x%" PRIxPTR "..0x%" PRIxPTR \
            ")\n",                                                          \
            __FILE__, __LINE__, reinterpret_cast<uintptr_t>(addr),          \
            reinterpret_cast<uintptr_t>(addr) + static_cast<size_t>(size)); \
    fflush(stderr);                                                         \
  }
#else
#define ASAN_UNPOISON_MEMORY_REGION(addr, size) \
  do {                                          \
    static_cast<void>(addr);                    \
    static_cast<void>(size);                    \
  } while (false)
#endif

#define ASAN_VERIFY_REGION_IS_UNPOISONED(addr, size)                          \
  do {                                                                        \
    void* _asan_addr = const_cast<void*>(addr);                               \
    size_t _asan_size = (size);                                               \
    void* _asan_bad_addr = __asan_region_is_poisoned(_asan_addr, _asan_size); \
    if (_asan_bad_addr != nullptr) {                                          \
      __asan_report_error(                                                    \
          __builtin_extract_return_addr(__builtin_return_address(0)),         \
          __builtin_frame_address(0),                                         \
          reinterpret_cast<void*>(&_asan_bad_addr), _asan_bad_addr, 0,        \
          _asan_size - (((char*)_asan_bad_addr) - ((char*)_asan_addr)));      \
    }                                                                         \
  } while (false)

#define ASAN_VERIFY_REGION_IS_POISONED(addr, size)                            \
  do {                                                                        \
    void* _asan_addr = const_cast<void*>(addr);                               \
    size_t _asan_size = (size);                                               \
    void* _asan_bad_addr = __asan_region_is_poisoned(_asan_addr, _asan_size); \
    if (_asan_bad_addr == nullptr && _asan_size != 0) {                       \
      __asan_report_error(                                                    \
          __builtin_extract_return_addr(__builtin_return_address(0)),         \
          __builtin_frame_address(0),                                         \
          reinterpret_cast<void*>(&_asan_bad_addr), _asan_addr, 0,            \
          _asan_size);                                                        \
    }                                                                         \
  } while (false)

#endif  // SHARE_SANITIZERS_ADDRESS_HPP
