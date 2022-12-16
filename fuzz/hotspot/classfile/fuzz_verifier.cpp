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
#include "jvm.h"

#include <limits>

#include <stdio.h>
#include <inttypes.h>

class VerifierFuzzer final : public Fuzzer {
 public:
  ~VerifierFuzzer() override {
    env()->DeleteGlobalRef(_class_loader);
  }

  jint Initialize() override {
    {
      jclass class_loader_class = env()->FindClass("java/lang/ClassLoader");
      if (env()->ExceptionCheck()) {
        env()->ExceptionDescribe();
        return JNI_ERR;
      }
      jmethodID get_system_class_loader_method = env()->GetStaticMethodID(class_loader_class,
                                                                          "getSystemClassLoader",
                                                                          "()Ljava/lang/ClassLoader;");
      if (env()->ExceptionCheck()) {
        env()->ExceptionDescribe();
        return JNI_ERR;
      }
      jobject class_loader = env()->CallStaticObjectMethod(class_loader_class,
                                                           get_system_class_loader_method);
      if (env()->ExceptionCheck()) {
        env()->ExceptionDescribe();
        return JNI_ERR;
      }
      _class_loader = env()->NewGlobalRef(class_loader);
      if (env()->ExceptionCheck()) {
        env()->ExceptionDescribe();
        return JNI_ERR;
      }
      env()->DeleteLocalRef(class_loader);
      env()->DeleteLocalRef(class_loader_class);
    }
    return JNI_OK;
  }

  jint TestOneInput(const uint8_t* data, size_t size) override {
    Attempt();
    if (size > std::numeric_limits<jsize>::max()) {
      size = std::numeric_limits<jsize>::max();
    }
    jclass clazz = env()->DefineClass(_class_name, _class_loader,
                                      reinterpret_cast<const jbyte*>(data),
                                      static_cast<jsize>(size));
    if (env()->ExceptionCheck()) {
      env()->ExceptionClear();
    } else {
      env()->DeleteLocalRef(clazz);
    }
    return JNI_OK;
  }

 private:
  void Attempt() {
    int result = snprintf(_class_name, sizeof(_class_name) - 1, "jdk.fuzz.Fuzzer%" PRIu64,
                          _attempt++);
    if (result >= 0) {
      // Should never be greater than sizeof(_class_name).
      _class_name[result] = '\0';
    }
  }

  uint64_t _attempt = 0;
  jobject _class_loader = nullptr;
  char _class_name[128];
};

FUZZ(VerifierFuzzer);
