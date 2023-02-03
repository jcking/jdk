/*
 * Copyright (c) 2023, Google and/or its affiliates. All rights reserved.
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

#ifndef SHARE_UTILITIES_OPTIMIZATION_HPP
#define SHARE_UTILITIES_OPTIMIZATION_HPP

#include <cstddef>

// UNLIKELY(condition)
//
// Hint to the compiler that the condition is expected to be false the majority of the time. The
// compiler may use this information to optimize code layout. Incorrect use of this macro may result
// in suboptimal performance, so use with caution. When in doubt, do not use it.
//
// if (UNLIKELY(foo || bar)) {
//   /* Code unlikely to be executed. */
// }

// LIKELY(condition)
//
// Hint to the compiler that the condition is expected to be true the majority of the time. The
// compiler may use this information to optimize code layout. Incorrect use of this macro may result
// in suboptimal performance, so use with caution. When in doubt, do not use it.
//
// if (LIKELY(foo || bar)) {
//   /* Code likely to be executed. */
// }

// ASSUME(expression)
//
// Hint to the compiler that the given expression evaluates to true. The compiler can use this
// information to make assumptions about following statements and peform optimizations based on it.

// T assume_aligned<T, A>(T)
//
// Hint to the compiler that the pointer returned can be assumed to be aligned to at least the
// specified alignment.
//
// T* foo = /* ... */;
// T* bar = assume_aligned<32>(foo);
// /* Compiler can assume `bar` is aligned to at least 32 bytes even if it cannot prove it. */

// unreachable()
//
// Inform the compiler that this statement is unreachable. The compiler may use this information to
// optimize generated code, for example by omitting call return logic.

// prefetch_for_read(const void*)
//
// Moves data into the L1 cache before it is read. Incorrect use of this function may result in
// suboptimal performance, so use with caution. When in doubt, do not use it.

// prefetch_for_read_nta(const void*)
//
// Moves data into the L1 cache before it is read, but with non-temporal locality: the data is not
// left in any of the cache tiers. Generally useful when the data is used only once. Incorrect use
// of this function may result in suboptimal performance, so use with caution. When in doubt, do not
// use it.

// prefetch_for_write(const void*)
//
// Moves data into the L1 cache before it is modified. Incorrect use of this function may result in
// suboptimal performance, so use with caution. When in doubt, do not use it.

#if defined(TARGET_COMPILER_gcc)

#define UNLIKELY(condition) __builtin_expect(false || (condition), false)
#define LIKELY(condition) __builtin_expect(false || (condition), true)

[[noreturn]] inline void unreachable() {
  __builtin_unreachable();
}

#ifdef __has_builtin
#if __has_builtin(__builtin_assume)
#define ASSUME(expression) __builtin_assume(expression)
#endif
#endif

template <typename T, size_t A>
inline T* assume_aligned(T* ptr) {
  static_assert(A != 0 && (A & (A - 1)) == 0, "alignment must be power of 2");
#ifdef __has_builtin
#if __has_builtin(__builtin_assume_aligned)
  return static_cast<T*>(__builtin_assume_aligned(ptr, A));
#else
  return ptr;
#endif
#else
  return ptr;
#endif
}

inline void prefetch_for_read(const void* addr) {
  __builtin_prefetch(addr, 0, 3);
}

inline void prefetch_for_read_nta(const void* addr) {
  __builtin_prefetch(addr, 0, 0);
}

inline void prefetch_for_write(const void* addr) {
  __builtin_prefetch(addr, 1, 3);
}

#elif defined(TARGET_COMPILER_visCPP)

#define ASSUME(expression) __assume(expression)

template <typename T, size_t A>
inline T* assume_aligned(T* ptr) {
  static_assert(A != 0 && (A & (A - 1)) == 0, "alignment must be power of 2");
  return ptr;
}

[[noreturn]] inline void unreachable() {
  __assume(0);
}

#if defined(_M_X64)

#include <intrin.h>

#pragma intrinsic(_mm_prefetch)
#pragma intrinsic(_m_prefetchw)

inline void prefetch_for_read(const void* addr) {
  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0);
}

inline void prefetch_for_read_nta(const void* addr) {
  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_NTA);
}

inline void prefetch_for_write(const void* addr) {
#ifdef _MM_HINT_ET0
  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_ET0);
#else
  _m_prefetchw(const_cast<void*>(addr));
#endif
}

#else

inline void prefetch_for_read(const void* addr) {
  static_cast<void>(addr);
}

inline void prefetch_for_read_nta(const void* addr) {
  static_cast<void>(addr);
}

inline void prefetch_for_write(const void* addr) {
  static_cast<void>(addr);
}

#endif

#elif defined(TARGET_COMPILER_xlc)

#include <builtins.h>

#define UNLIKELY(condition) __builtin_expect(false || (condition), false)
#define LIKELY(condition) __builtin_expect(false || (condition), true)

template <typename T, size_t A>
inline T* assume_aligned(T* ptr) {
  static_assert(A != 0 && (A & (A - 1)) == 0, "alignment must be power of 2");
  __alignx(static_cast<int>(A), ptr);
  return ptr;
}

[[noreturn]] inline void unreachable() {
  /* returning from noreturn is undefined behavior */
}

inline void prefetch_for_read(const void* addr) {
  __dcbt(const_cast<void*>(addr));
}

inline void prefetch_for_read_nta(const void* addr) {
#if defined(_ARCH_PWR7)
  __dcbtt(const_cast<void*>(addr));
#else
  static_cast<void>(addr);
#endif
}

inline void prefetch_for_write(const void* addr) {
  __dcbtst(const_cast<void*>(addr));
}

#else

#error Unknown toolchain.

#endif

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(likely)
#define ATTRIBUTE_LIKELY [[likely]]
#endif
#if __has_cpp_attribute(unlikely)
#define ATTRIBUTE_UNLIKELY [[unlikely]]
#endif
#endif

#ifndef ATTRIBUTE_LIKELY
#define ATTRIBUTE_LIKELY
#endif

#ifndef ATTRIBUTE_UNLIKELY
#define ATTRIBUTE_UNLIKELY
#endif

#ifndef UNLIKELY
#define UNLIKELY(condition) (condition)
#endif

#ifndef LIKELY
#define LIKELY(condition) (condition)
#endif

#ifndef ASSUME
#define ASSUME(expression)   \
  do {                       \
    if (false) {             \
      ((void) (expression)); \
    }                        \
  } while (false)
#endif

#endif  // SHARE_UTILITIES_OPTIMIZATION_HPP
