/*
 * Copyright (c) 2003, 2020, Oracle and/or its affiliates. All rights reserved.
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

#ifndef OS_CPU_WINDOWS_X86_ORDERACCESS_WINDOWS_X86_HPP
#define OS_CPU_WINDOWS_X86_ORDERACCESS_WINDOWS_X86_HPP

// Included in orderAccess.hpp header file.

#include "runtime/vm_version.hpp"

#include <intrin.h>

#if _MSC_VER >= 1928
#pragma intrinsic(_serialize)
#endif

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(__cpuid)

// Compiler version last used for testing: Microsoft Visual Studio 2010
// Please update this information when this file changes

// Implementation of class OrderAccess.

#pragma warning(push)
#pragma warning(disable : 6001)
#pragma warning(disable : 28113)

inline void OrderAccess::fence() {
  _ReadWriteBarrier();
  volatile long guard;
  (void) _InterlockedIncrement(&guard);
  _ReadWriteBarrier();
}

#pragma warning(pop)

inline void OrderAccess::cross_modify_fence_impl() {
#if _MSC_VER >= 1928
  // _serialize() intrinsic is supported starting from VS2019-16.7.2
  if (VM_Version::supports_serialize()) {
    _serialize();
    return;
  }
#endif
  int regs[4];
  __cpuid(regs, 0);
}

#endif // OS_CPU_WINDOWS_X86_ORDERACCESS_WINDOWS_X86_HPP
