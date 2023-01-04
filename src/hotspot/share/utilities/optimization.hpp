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

#ifndef SHARE_UTILITIES_OPTIMIZATION_HPP
#define SHARE_UTILITIES_OPTIMIZATION_HPP

#include "utilities/debug.hpp"
#include "utilities/macros.hpp"

#if HAS_BUILTIN(__builtin_expect)
#define PREDICT_FALSE(condition) __builtin_expect(false || (condition), false)
#define PREDICT_TRUE(condition) __builtin_expect(false || (condition), true)
#else
#define PREDICT_FALSE(condition) (condition)
#define PREDICT_TRUE(condition) (condition)
#endif

#if defined(ASSERT)
#define ASSUME(condition) assert((condition), #condition)
#elif HAS_BUILTIN(__builtin_assume)
#define ASSUME(condition) __builtin_assume((condition))
#elif defined(TARGET_COMPILER_visCPP)
#define ASSUME(condition) __assume((condition))
#elif HAS_BUILTIN(__builtin_unreachable)
#define ASSUME(condition)                      \
  do {                                         \
    if (!(condition)) __builtin_unreachable(); \
  } while (false)
#else
#define ASSUME(condition)            \
  do {                               \
    ((void) (false && (condition))); \
  } while (false)
#endif

#endif  // SHARE_UTILITIES_OPTIMIZATION_HPP
