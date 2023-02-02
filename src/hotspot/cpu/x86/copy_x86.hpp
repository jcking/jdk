/*
 * Copyright (c) 2003, 2022, Oracle and/or its affiliates. All rights reserved.
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

#ifndef CPU_X86_COPY_X86_HPP
#define CPU_X86_COPY_X86_HPP

#include OS_CPU_HEADER(copy)

static void pd_disjoint_words_atomic(const HeapWord* from, HeapWord* to, size_t count) {
#ifdef AMD64
  shared_disjoint_words_atomic(from, to, count);
#else
  // pd_disjoint_words is word-atomic in this implementation.
  pd_disjoint_words(from, to, count);
#endif // AMD64
}

// Windows has a different implementation
#ifndef _WINDOWS
static void pd_conjoint_jshorts_atomic(const jshort* from, jshort* to, size_t count) {
  _Copy_conjoint_jshorts_atomic(from, to, count);
}

static void pd_conjoint_jints_atomic(const jint* from, jint* to, size_t count) {
#ifdef AMD64
  _Copy_conjoint_jints_atomic(from, to, count);
#else
  assert(HeapWordSize == BytesPerInt, "heapwords and jints must be the same size");
  // pd_conjoint_words is word-atomic in this implementation.
  pd_conjoint_words((const HeapWord*)from, (HeapWord*)to, count);
#endif // AMD64
}

static void pd_conjoint_jlongs_atomic(const jlong* from, jlong* to, size_t count) {
#ifdef AMD64
  _Copy_conjoint_jlongs_atomic(from, to, count);
#else
  // Guarantee use of fild/fistp or xmm regs via some asm code, because compilers won't.
  if (from > to) {
    while (count-- > 0) {
      __asm__ volatile("fildll (%0); fistpll (%1)"
                       :
                       : "r" (from), "r" (to)
                       : "memory" );
      ++from;
      ++to;
    }
  } else {
    while (count-- > 0) {
      __asm__ volatile("fildll (%0,%2,8); fistpll (%1,%2,8)"
                       :
                       : "r" (from), "r" (to), "r" (count)
                       : "memory" );
    }
  }
#endif // AMD64
}

static void pd_conjoint_oops_atomic(const oop* from, oop* to, size_t count) {
#ifdef AMD64
  assert(BytesPerLong == BytesPerOop, "jlongs and oops must be the same size");
  _Copy_conjoint_jlongs_atomic((const jlong*)from, (jlong*)to, count);
#else
  assert(HeapWordSize == BytesPerOop, "heapwords and oops must be the same size");
  // pd_conjoint_words is word-atomic in this implementation.
  pd_conjoint_words((const HeapWord*)from, (HeapWord*)to, count);
#endif // AMD64
}

static void pd_arrayof_conjoint_jshorts(const HeapWord* from, HeapWord* to, size_t count) {
  _Copy_arrayof_conjoint_jshorts(from, to, count);
}

static void pd_arrayof_conjoint_jints(const HeapWord* from, HeapWord* to, size_t count) {
#ifdef AMD64
   _Copy_arrayof_conjoint_jints(from, to, count);
#else
  pd_conjoint_jints_atomic((const jint*)from, (jint*)to, count);
#endif // AMD64
}

static void pd_arrayof_conjoint_jlongs(const HeapWord* from, HeapWord* to, size_t count) {
#ifdef AMD64
  _Copy_arrayof_conjoint_jlongs(from, to, count);
#else
  pd_conjoint_jlongs_atomic((const jlong*)from, (jlong*)to, count);
#endif // AMD64
}

static void pd_arrayof_conjoint_oops(const HeapWord* from, HeapWord* to, size_t count) {
#ifdef AMD64
  assert(BytesPerLong == BytesPerOop, "jlongs and oops must be the same size");
  _Copy_arrayof_conjoint_jlongs(from, to, count);
#else
  pd_conjoint_oops_atomic((const oop*)from, (oop*)to, count);
#endif // AMD64
}

#endif // _WINDOWS

#endif // CPU_X86_COPY_X86_HPP
