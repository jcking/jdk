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

#ifndef SHARE_UTILITIES_OVERFLOW_HPP
#define SHARE_UTILITIES_OVERFLOW_HPP

#include "metaprogramming/enableIf.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/optimization.hpp"
#include "utilities/population_count.hpp"

#include <limits>
#include <type_traits>

// Arithmetic operations which can detect overflow in a well defined manner. All functions
// return true if overflow is detected, false otherwise.

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_signed<T>::value)>
ALWAYSINLINE bool add_overflow(T x, T y, T* out) {
#if HAS_BUILTIN(__builtin_add_overflow)
  return __builtin_add_overflow(x, y, out);
#else
  if (PREDICT_FALSE((y > 0 && x > std::numeric_limits<T>::max() - y) ||
                    (y < 0 && x < std::numeric_limits<T>::min() - y))) {
    return true;
  }
  *out = x + y;
  return false;
#endif
}

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_unsigned<T>::value)>
ALWAYSINLINE bool add_overflow(T x, T y, T* out) {
#if HAS_BUILTIN(__builtin_add_overflow)
  return __builtin_add_overflow(x, y, out);
#else
  if (PREDICT_FALSE(x > std::numeric_limits<T>::max() - y)) {
    return true;
  }
  *out = x + y;
  return false;
#endif
}

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_signed<T>::value)>
ALWAYSINLINE bool subtract_overflow(T x, T y, T* out) {
#if HAS_BUILTIN(__builtin_sub_overflow)
  return __builtin_sub_overflow(x, y, out);
#else
  if (PREDICT_FALSE((y < 0 && x > std::numeric_limits<T>::max() + y) ||
                    (y > 0 && x < std::numeric_limits<T>::min() + y))) {
    return true;
  }
  *out = x - y;
  return false;
#endif
}

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_unsigned<T>::value)>
ALWAYSINLINE bool subtract_overflow(T x, T y, T* out) {
#if HAS_BUILTIN(__builtin_sub_overflow)
  return __builtin_sub_overflow(x, y, out);
#else
  if (PREDICT_FALSE(x < std::numeric_limits<T>::min() + y)) {
    return true;
  }
  *out = x - y;
  return false;
#endif
}

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_signed<T>::value)>
ALWAYSINLINE bool multiply_overflow(T x, T y, T* out) {
#if HAS_BUILTIN(__builtin_mul_overflow)
  return __builtin_mul_overflow(x, y, out);
#else
  if (PREDICT_FALSE((x == -1 && y == std::numeric_limits<T>::min()) ||
                    (x == std::numeric_limits<T>::min() && y == -1) ||
                    (x > 0 && y > 0 && x > std::numeric_limits<T>::max() / y) ||
                    (x < 0 && y < 0 && x < std::numeric_limits<T>::max() / y) ||
                    (x > 0 && y < 0 && y < std::numeric_limits<T>::min() / x) ||
                    (x < 0 && y > 0 && x < std::numeric_limits<T>::min() / y))) {
    return true;
  }
  *out = x * y;
  return false;
#endif
}

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_unsigned<T>::value)>
ALWAYSINLINE bool multiply_overflow(T x, T y, T* out) {
#if HAS_BUILTIN(__builtin_mul_overflow)
  return __builtin_mul_overflow(x, y, out);
#else
  if (PREDICT_FALSE(y != 0 && x > std::numeric_limits<T>::max() / y)) {
    return true;
  }
  *out = x * y;
  return false;
#endif
}

template <typename T, ENABLE_IF(std::is_integral<T>::value && std::is_unsigned<T>::value)>
ALWAYSINLINE bool align_up_overflow(T x, T alignment, T* out) {
  assert(population_count(alignment) == 1, "alignment must be power of 2");
  T y;
  if (PREDICT_FALSE(add_overflow(x, alignment, &y))) {
    return true;
  }
  *out = (y - 1) & ~(alignment - 1);
  return false;
}

#endif // SHARE_UTILITIES_OVERFLOW_HPP
