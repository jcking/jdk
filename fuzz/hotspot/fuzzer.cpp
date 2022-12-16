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

#include "fuzzer.h"
#include "jni.h"

#include <string.h>

static void RemoveArgument(int* argc, char** argv, int index) {
  memmove(&argv[index], &argv[index + 1], *argc - (index + 1));
  argv[--(*argc)] = nullptr;
}

static bool StartsWith(const char* subject, const char* prefix) {
  size_t subject_length = strlen(subject);
  size_t prefix_length = strlen(prefix);
  if (prefix_length > subject_length) {
    return false;
  }
  return strncmp(subject, prefix, prefix_length) == 0;
}

static bool IsStandardOption(const char* option) {
  return StartsWith(option, "-X") || StartsWith(option, "-D") || strcmp(option, "-verbose") == 0
         || StartsWith(option, "-verbose:");
}

static void ProcessArguments(int* argc, char** argv, JavaVMInitArgs* vm_args) {
  for (int index = 1; index < *argc; index++) {
    char* arg = argv[index];
    if (IsStandardOption(arg)) {
      vm_args->options[vm_args->nOptions++].optionString = arg;
      RemoveArgument(argc, argv, index);
    }
  }
}

static void FreeOptions(JavaVMInitArgs* vm_args) {
  delete[] vm_args->options;
  vm_args->options = nullptr;
  vm_args->nOptions = 0;
}

static Fuzzer* fuzzer = nullptr;

extern "C" Fuzzer* FuzzerNew();

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
  JavaVM* jvm = nullptr;
  JNIEnv* env = nullptr;
  JavaVMInitArgs vm_args;
  memset(&vm_args, '\0', sizeof(vm_args));
  vm_args.version = JNI_VERSION_1_8;
  vm_args.ignoreUnrecognized = JNI_FALSE;
  vm_args.options = new JavaVMOption[*argc];
  memset(vm_args.options, '\0', (*argc) * sizeof(JavaVMOption));
  ProcessArguments(argc, *argv, &vm_args);
  jint result = JNI_CreateJavaVM(&jvm, reinterpret_cast<void**>(&env), &vm_args);
  FreeOptions(&vm_args);
  if (result != JNI_OK) {
    jvm = nullptr;
    env = nullptr;
    return result;
  }
  fuzzer = FuzzerNew();
  result = fuzzer->Initialize(jvm, env);
  if (result != JNI_OK) {
    delete fuzzer;
    fuzzer = nullptr;
    jvm->DestroyJavaVM();
    return result;
  }
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  jint result = fuzzer->TestOneInput(data, size);
  return result == JNI_OK ? 0 : -1;
}
