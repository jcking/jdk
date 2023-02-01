/*
 * Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.
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

#ifndef OS_CPU_LINUX_ARM_ORDERACCESS_LINUX_ARM_HPP
#define OS_CPU_LINUX_ARM_ORDERACCESS_LINUX_ARM_HPP

// Included in orderAccess.hpp header file.

#include "runtime/os.hpp"
#include "runtime/vm_version.hpp"

// Implementation of class OrderAccess.
// - we define the high level barriers below and use the general
//   implementation in orderAccess.hpp.

// Memory Ordering on ARM is weak.
//
// Implement all 4 memory ordering barriers by DMB, since it is a
// lighter version of DSB.
// dmb_sy implies full system shareability domain. RD/WR access type.
// dmb_st implies full system shareability domain. WR only access type.
//
// NOP on < ARMv6 (MP not supported)
//
// Non mcr instructions can be used if we build for armv7 or higher arch
//    __asm__ __volatile__ ("dmb" : : : "memory");
//    __asm__ __volatile__ ("dsb" : : : "memory");
//
// inline void _OrderAccess_dsb() {
//    volatile intptr_t dummy = 0;
//    __asm__ volatile (
//      "mcr p15, 0, %0, c7, c10, 4"
//      : : "r" (dummy) : "memory");
// }

inline void OrderAccess::cross_modify_fence_impl()   { }

#endif // OS_CPU_LINUX_ARM_ORDERACCESS_LINUX_ARM_HPP
