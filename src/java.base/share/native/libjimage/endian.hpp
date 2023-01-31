/*
 * Copyright (c) 2015, 2022, Oracle and/or its affiliates. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   - Neither the name of Oracle nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIBJIMAGE_ENDIAN_HPP
#define LIBJIMAGE_ENDIAN_HPP

#include "inttypes.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_byteswap_ushort)
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_uint64)
#elif defined(__xlC__)
#include <builtins.h>
#endif

enum class ByteOrder {
    BIG,
    LITTLE,

    JAVA = BIG,
#if defined(_LITTLE_ENDIAN)
    NATIVE = LITTLE,
#endif
#if defined(_BIG_ENDIAN)
    NATIVE = BIG,
#endif
};

// Selectable endian handling. Endian handlers are used when accessing values
// that are of unknown (until runtime) endian.  The only requirement of the values
// accessed are that they are aligned to proper size boundaries (no misalignment.)
// To select an endian handler, one should call Endian::get_handler(big_endian);
// Where big_endian is true if big endian is required and false otherwise.  The
// native endian handler can be fetched with Endian::get_native_handler();
// To retrieve a value using the appropriate endian, use one of the overloaded
// calls to get. To set a value, then use one of the overloaded set calls.
// Ex.
//          s4 value; // Imported value;
//          ...
//          Endian* endian = Endian::get_handler(true);  // Use big endian
//          s4 corrected = endian->get(value);
//          endian->set(value, 1);
//
class Endian final {
public:
    static constexpr ByteOrder BIG = ByteOrder::BIG;
    static constexpr ByteOrder LITTLE = ByteOrder::LITTLE;
    static constexpr ByteOrder JAVA = ByteOrder::JAVA;
    static constexpr ByteOrder NATIVE = ByteOrder::NATIVE;

    static constexpr bool is_big() {
      return NATIVE == BIG;
    }

#if defined(__GNUC__)

    static inline u2 swap(u2 x) {
        return __builtin_bswap16(x);
    }

    static inline u4 swap(u4 x) {
        return __builtin_bswap32(x);
    }

    static inline u8 swap(u8 x) {
        return __builtin_bswap64(x);
    }

#elif defined(_MSC_VER)

    static inline u2 swap(u2 x) {
        return _byteswap_ushort(x);
    }

    static inline u4 swap(u4 x) {
        return _byteswap_ulong(x);
    }

    static inline u8 swap(u8 x) {
        return _byteswap_uint64(x);
    }

#elif defined(__xlC__)

    static inline u2 swap(u2 x) {
        unsigned short y;
        __store2r(static_cast<unsigned short>(x), &y);
        return static_cast<u2>(y);
    }

    static inline u4 swap(u4 x) {
        unsigned int y;
        __store4r(static_cast<unsigned int>(x), &y);
        return static_cast<u4>(y);
    }

    static inline u8 swap(u8 x) {
#if defined(_ARCH_PWR7) && defined(_ARCH_PPC64)
        unsigned long long y;
        __store8r(static_cast<unsigned long long>(x), &y);
        return static_cast<u8>(y);
#else
        return (((x & UINT64_C(0x00000000000000ff)) << 56) | ((x & UINT64_C(0x000000000000ff00)) << 40) |
                ((x & UINT64_C(0x0000000000ff0000)) << 24) | ((x & UINT64_C(0x00000000ff000000)) << 8) |
                ((x & UINT64_C(0x000000ff00000000)) >> 8) | ((x & UINT64_C(0x0000ff0000000000)) >> 24) |
                ((x & UINT64_C(0x00ff000000000000)) >> 40) | ((x & UINT64_C(0xff00000000000000)) >> 56));
#endif
    }

#else

    static inline u2 swap(u2 x) {
        return (((x & UINT16_C(0x00ff)) << 8) | ((x & UINT16_C(0xff00)) >> 8));
    }

    static inline u4 swap(u4 x) {
        return (((x & UINT32_C(0x000000ff)) << 24) | ((x & UINT32_C(0x0000ff00)) << 8) |
                ((x & UINT32_C(0x00ff0000)) >> 8) | ((x & UINT32_C(0xff000000)) >> 24));
    }

    static inline u8 swap(u8 x) {
        return (((x & UINT64_C(0x00000000000000ff)) << 56) | ((x & UINT64_C(0x000000000000ff00)) << 40) |
                ((x & UINT64_C(0x0000000000ff0000)) << 24) | ((x & UINT64_C(0x00000000ff000000)) << 8) |
                ((x & UINT64_C(0x000000ff00000000)) >> 8) | ((x & UINT64_C(0x0000ff0000000000)) >> 24) |
                ((x & UINT64_C(0x00ff000000000000)) >> 40) | ((x & UINT64_C(0xff00000000000000)) >> 56));
    }

#endif

    static inline s2 swap(s2 x) {
      return static_cast<s2>(swap(static_cast<u2>(x)));
    }

    static inline s4 swap(s4 x) {
      return static_cast<s4>(swap(static_cast<u4>(x)));
    }

    static inline s8 swap(s8 x) {
      return static_cast<s8>(swap(static_cast<u8>(x)));
    }

    static inline u2 get(ByteOrder order, u2 x) {
      if (order != NATIVE) {
          x = swap(x);
      }
      return x;
    }

    static inline u4 get(ByteOrder order, u4 x) {
      if (order != NATIVE) {
          x = swap(x);
      }
      return x;
    }

    static inline u8 get(ByteOrder order, u8 x) {
      if (order != NATIVE) {
          x = swap(x);
      }
      return x;
    }

    static inline s2 get(ByteOrder order, s2 x) {
      if (order != NATIVE) {
          x = swap(x);
      }
      return x;
    }

    static inline s4 get(ByteOrder order, s4 x) {
      if (order != NATIVE) {
          x = swap(x);
      }
      return x;
    }

    static inline s8 get(ByteOrder order, s8 x) {
      if (order != NATIVE) {
          x = swap(x);
      }
      return x;
    }

    static inline void set(ByteOrder order, u2& x, u2 y) {
        x = get(order, y);
    }

    static inline void set(ByteOrder order, u4& x, u4 y) {
        x = get(order, y);
    }

    static inline void set(ByteOrder order, u8& x, u8 y) {
        x = get(order, y);
    }

    static inline void set(ByteOrder order, s2& x, s2 y) {
        x = get(order, y);
    }

    static inline void set(ByteOrder order, s4& x, s4 y) {
        x = get(order, y);
    }

    static inline void set(ByteOrder order, s8& x, s8 y) {
        x = get(order, y);
    }

    // get platform u2 from Java Big endian
    static inline u2 get_java(const u1* x) {
        return (static_cast<u2>(x[0]) << 8) | static_cast<u2>(x[1]);
    }

    // set platform u2 to Java Big endian
    static inline void set_java(u1* p, u2 x) {
        p[0] = static_cast<u1>((x >> 8) & 0xff);
        p[1] = static_cast<u1>(x & 0xff);
    }
};

#endif // LIBJIMAGE_ENDIAN_HPP
