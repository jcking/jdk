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

#ifndef FUZZER_HPP
#define FUZZER_HPP

#include "jni.h"

#include <stddef.h>
#include <stdint.h>

extern "C" int LLVMFuzzerInitialize(int*, char***);

class Fuzzer {
 public:
  virtual ~Fuzzer() = default;

  virtual jint Initialize() { return JNI_OK; }

  virtual jint TestOneInput(const uint8_t* data, size_t size) = 0;

 protected:
  inline JavaVM* jvm() const { return _jvm; }

  inline JNIEnv* env() const { return _env; }

 private:
  friend int LLVMFuzzerInitialize(int*, char***);

   jint Initialize(JavaVM* jvm, JNIEnv* env) {
    _jvm = jvm;
    _env = env;
    return Initialize();
   }

   JavaVM* _jvm = nullptr;
   JNIEnv* _env = nullptr;
};

#define FUZZ(type) extern "C" Fuzzer* FuzzerNew() { return new type(); }

#endif  // FUZZER_HPP
