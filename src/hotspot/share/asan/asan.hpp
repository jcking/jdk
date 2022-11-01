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

#ifndef SHARE_ASAN_ASAN_HPP
#define SHARE_ASAN_ASAN_HPP

#include <cstddef>
#include <cstdint>

#include "memory/allStatic.hpp"
#include "utilities/globalDefinitions.hpp"

#ifdef ADDRESS_SANITIZER
#include <sanitizer/asan_interface.h>

#define NO_SANITIZE_ADDRESS __attribute__((no_sanitize("address")))
#else
#define NO_SANITIZE_ADDRESS
#endif

class Asan final : AllStatic {
 public:
  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void
  poison_memory_region(const volatile void* ptr, size_t n) {
#ifdef ADDRESS_SANITIZER
    __asan_poison_memory_region(ptr, n);
#else
    static_cast<void>(ptr);
    static_cast<void>(n);
#endif
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void
  unpoison_memory_region(const volatile void* ptr, size_t n) {
#ifdef ADDRESS_SANITIZER
    __asan_unpoison_memory_region(ptr, n);
#else
    static_cast<void>(ptr);
    static_cast<void>(n);
#endif
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void store1(
      void* ptr) {
    storeN(ptr, 1);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void store2(
      void* ptr) {
    storeN(ptr, 2);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void store4(
      void* ptr) {
    storeN(ptr, 4);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void store8(
      void* ptr) {
    storeN(ptr, 8);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void storeN(
      void* ptr, size_t n) {
#ifdef ADDRESS_SANITIZER
    void* bad = is_region_poisoned(ptr, n);
    if (UNLIKELY(bad != nullptr)) {
      report_error(__builtin_extract_return_addr(__builtin_return_address(0)),
                   __builtin_frame_address(0), reinterpret_cast<void*>(&bad), bad, n);
    }
#else
    static_cast<void>(ptr);
    static_cast<void>(n);
#endif
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void load1(
      const void* ptr) {
    loadN(ptr, 1);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void load2(
      const void* ptr) {
    loadN(ptr, 2);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void load4(
      const void* ptr) {
    loadN(ptr, 4);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void load8(
      const void* ptr) {
    loadN(ptr, 8);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void loadN(
      const void* ptr, size_t n) {
#ifdef ADDRESS_SANITIZER
    const void* bad = is_region_poisoned(ptr, n);
    if (UNLIKELY(bad != nullptr)) {
      report_error(__builtin_extract_return_addr(__builtin_return_address(0)),
                   __builtin_frame_address(0), reinterpret_cast<void*>(&bad), bad, n);
    }
#else
    static_cast<void>(ptr);
    static_cast<void>(n);
#endif
  }

 private:
#ifdef ADDRESS_SANITIZER
  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void*
  is_region_poisoned(void* ptr, size_t n) {
    return __asan_region_is_poisoned(ptr, n);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE const void*
  is_region_poisoned(const void* ptr, size_t n) {
    return __asan_region_is_poisoned(const_cast<void*>(ptr), n);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void
  report_error(void* pc, void* bp, void* sp, void* addr, size_t size) {
    __asan_report_error(pc, bp, sp, addr, 1, size);
  }

  static NO_SANITIZE_ADDRESS ATTRIBUTE_ARTIFICIAL ALWAYSINLINE void
  report_error(void* pc, void* bp, void* sp, const void* addr, size_t size) {
    __asan_report_error(pc, bp, sp, const_cast<void*>(addr), 0, size);
  }
#endif
};

#endif  // SHARE_ASAN_ASAN_HPP
