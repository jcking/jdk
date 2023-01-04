/*
 * Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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
 */

#include "precompiled.hpp"
#include "utilities/overflow.hpp"
#include "unittest.hpp"

#include <limits>

template <typename T>
void test_add_overflow_signed() {
  T result;

  EXPECT_FALSE(add_overflow(T{0}, T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::max(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_FALSE(add_overflow(T{0}, std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::min(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_FALSE(add_overflow(T{0}, std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_TRUE(add_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), &result));

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{-1});

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, T{-1});

  EXPECT_TRUE(add_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), &result));
}

template <typename T>
void test_add_overflow_unsigned() {
  T result;

  EXPECT_FALSE(add_overflow(T{0}, T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::max(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_FALSE(add_overflow(T{0}, std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::min(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_FALSE(add_overflow(T{0}, std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_FALSE(add_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_TRUE(add_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), &result));
}

template <typename T>
void test_add_overflow() {
  test_add_overflow_signed<std::make_signed_t<T>>();
  test_add_overflow_unsigned<std::make_unsigned_t<T>>();
}

TEST(Overflow, Add) {
  test_add_overflow<char>();
  test_add_overflow<short>();
  test_add_overflow<int>();
  test_add_overflow<long>();
  test_add_overflow<long long>();
}

template <typename T>
void test_subtract_overflow_signed() {
  T result;

  EXPECT_FALSE(subtract_overflow(T{0}, T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::max(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_FALSE(subtract_overflow(T{0}, std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, -std::numeric_limits<T>::max());

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::min(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_TRUE(subtract_overflow(T{0}, std::numeric_limits<T>::min(), &result));

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_TRUE(subtract_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), &result));

  EXPECT_TRUE(subtract_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), &result));

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, T{0});
}

template <typename T>
void test_subtract_overflow_unsigned() {
  T result;

  EXPECT_FALSE(subtract_overflow(T{0}, T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::max(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_TRUE(subtract_overflow(T{0}, std::numeric_limits<T>::max(), &result));

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::min(), T{0}, &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_FALSE(subtract_overflow(T{0}, std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::min());

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, std::numeric_limits<T>::max());

  EXPECT_TRUE(subtract_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), &result));

  EXPECT_FALSE(subtract_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, T{0});
}

template <typename T>
void test_subtract_overflow() {
  test_subtract_overflow_signed<std::make_signed_t<T>>();
  test_subtract_overflow_unsigned<std::make_unsigned_t<T>>();
}

TEST(Overflow, Subtract) {
  test_subtract_overflow<char>();
  test_subtract_overflow<short>();
  test_subtract_overflow<int>();
  test_subtract_overflow<long>();
  test_subtract_overflow<long long>();
}

template <typename T>
void test_multiply_overflow_signed() {
  T result;

  EXPECT_FALSE(multiply_overflow(T{0}, T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::max(), T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(T{0}, std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::min(), T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(T{0}, std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_TRUE(multiply_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), &result));

  EXPECT_TRUE(multiply_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), &result));

  EXPECT_TRUE(multiply_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), &result));

  EXPECT_TRUE(multiply_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), &result));
}

template <typename T>
void test_multiply_overflow_unsigned() {
  T result;

  EXPECT_FALSE(multiply_overflow(T{0}, T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::max(), T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(T{0}, std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::min(), T{0}, &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(T{0}, std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::min(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_FALSE(multiply_overflow(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), &result));
  EXPECT_EQ(result, T{0});

  EXPECT_TRUE(multiply_overflow(std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), &result));
}

template <typename T>
void test_multiply_overflow() {
  test_multiply_overflow_signed<std::make_signed_t<T>>();
  test_multiply_overflow_unsigned<std::make_unsigned_t<T>>();
}

TEST(Overflow, Multiply) {
  test_multiply_overflow<char>();
  test_multiply_overflow<short>();
  test_multiply_overflow<int>();
  test_multiply_overflow<long>();
  test_multiply_overflow<long long>();
}
