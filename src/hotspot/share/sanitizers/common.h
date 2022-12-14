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

#ifndef SHARE_SANITIZERS_COMMON_HPP
#define SHARE_SANITIZERS_COMMON_HPP

#if defined(ADDRESS_SANITIZER)
#define ANY_SANITIZER
#endif

#ifdef ANY_SANITIZER
#include <sanitizer/common_interface_defs.h>
#endif

// SANITIZER_PRINT_STACK_TRACE()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_PRINT_STACK_TRACE() __sanitizer_print_stack_trace()
#else
#define SANITIZER_PRINT_STACK_TRACE() static_cast<void>(0)
#endif

// SANITIZER_UNALIGNED_LOAD16()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_UNALIGNED_LOAD16(p) __sanitizer_unaligned_load16((p))
#endif

// SANITIZER_UNALIGNED_LOAD32()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_UNALIGNED_LOAD32(p) __sanitizer_unaligned_load32((p))
#endif

// SANITIZER_UNALIGNED_LOAD64()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_UNALIGNED_LOAD64(p) __sanitizer_unaligned_load64((p))
#endif

// SANITIZER_UNALIGNED_STORE16()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_UNALIGNED_STORE16(p, x) __sanitizer_unaligned_store16((p), (x))
#endif

// SANITIZER_UNALIGNED_STORE32()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_UNALIGNED_STORE32(p, x) __sanitizer_unaligned_store32((p), (x))
#endif

// SANITIZER_UNALIGNED_STORE64()
//
//
#ifdef ANY_SANITIZER
#define SANITIZER_UNALIGNED_STORE64(p, x) __sanitizer_unaligned_store64((p), (x))
#endif

#undef ANY_SANITIZER

#endif  // SHARE_SANITIZERS_COMMON_HPP
