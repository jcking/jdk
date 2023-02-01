/*
 * Copyright (c) 1999, 2023, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_RUNTIME_ATOMIC_GCC_HPP
#define SHARE_RUNTIME_ATOMIC_GCC_HPP

#ifndef SHARE_RUNTIME_ATOMIC_HPP
#error atomic_gcc.hpp cannot not be included directly, use atomic.hpp
#endif

#include "metaprogramming/primitiveConversions.hpp"
#include "utilities/debug.hpp"

#include <type_traits>

template <>
template <typename T>
inline T Atomic::PlatformLoad<1>::operator()(T const volatile* src) const {
  STATIC_ASSERT(1 == sizeof(T));
  return __atomic_load_n(src, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline T Atomic::PlatformLoad<2>::operator()(T const volatile* src) const {
  STATIC_ASSERT(2 == sizeof(T));
  return __atomic_load_n(src, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline T Atomic::PlatformLoad<4>::operator()(T const volatile* src) const {
  STATIC_ASSERT(4 == sizeof(T));
  return __atomic_load_n(src, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline T Atomic::PlatformLoad<8>::operator()(T const volatile* src) const {
  STATIC_ASSERT(8 == sizeof(T));
  return __atomic_load_n(src, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline void Atomic::PlatformStore<1>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(1 == sizeof(T));
  __atomic_store_n(dest, store_value, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline void Atomic::PlatformStore<2>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(2 == sizeof(T));
  __atomic_store_n(dest, store_value, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline void Atomic::PlatformStore<4>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(4 == sizeof(T));
  __atomic_store_n(dest, store_value, __ATOMIC_RELAXED);
}

template <>
template <typename T>
inline void Atomic::PlatformStore<8>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(8 == sizeof(T));
  __atomic_store_n(dest, store_value, __ATOMIC_RELAXED);
}

template <>
struct Atomic::PlatformOrderedLoad<1, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(1 == sizeof(T));
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
  }
};

template <>
struct Atomic::PlatformOrderedLoad<2, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(2 == sizeof(T));
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
  }
};

template <>
struct Atomic::PlatformOrderedLoad<4, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(4 == sizeof(T));
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
  }
};

template <>
struct Atomic::PlatformOrderedLoad<8, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(8 == sizeof(T));
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
  }
};

template <>
struct Atomic::PlatformOrderedStore<1, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(1 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
  }
};

template <>
struct Atomic::PlatformOrderedStore<2, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(2 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
  }
};

template <>
struct Atomic::PlatformOrderedStore<4, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(4 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
  }
};

template <>
struct Atomic::PlatformOrderedStore<8, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(8 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
  }
};

template <>
struct Atomic::PlatformOrderedStore<1, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(1 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformOrderedStore<2, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(2 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformOrderedStore<4, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(4 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformOrderedStore<8, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(8 == sizeof(T));
    __atomic_store_n(p, v, __ATOMIC_RELEASE);
    OrderAccess::fence();
  }
};

template <size_t N>
struct Atomic::PlatformAdd {
  template<typename D, typename I>
  inline D fetch_and_add(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == N);
    return __atomic_fetch_add(dest, add_value, static_cast<int>(Atomic::liberalize(order)));
  }

  template<typename D, typename I>
  inline D add_and_fetch(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == N);
    return __atomic_add_fetch(dest, add_value, static_cast<int>(Atomic::liberalize(order)));
  }
};

template <>
template <typename T>
inline T Atomic::PlatformXchg<1>::operator()(T volatile* dest, T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(1 == sizeof(T));
  return __atomic_exchange_n(dest, exchange_value, static_cast<int>(Atomic::liberalize(order)));
}

template <>
template <typename T>
inline T Atomic::PlatformXchg<2>::operator()(T volatile* dest, T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(2 == sizeof(T));
  return __atomic_exchange_n(dest, exchange_value, static_cast<int>(Atomic::liberalize(order)));
}

template <>
template <typename T>
inline T Atomic::PlatformXchg<4>::operator()(T volatile* dest, T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(4 == sizeof(T));
  return __atomic_exchange_n(dest, exchange_value, static_cast<int>(Atomic::liberalize(order)));
}

template <>
template <typename T>
inline T Atomic::PlatformXchg<8>::operator()(T volatile* dest, T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(8 == sizeof(T));
  return __atomic_exchange_n(dest, exchange_value, static_cast<int>(Atomic::liberalize(order)));
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<1>::operator()(T volatile* dest, T compare_value, T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(1 == sizeof(T));
  T value = compare_value;
  __atomic_compare_exchange_n(dest, &value, exchange_value, false,
                              static_cast<int>(Atomic::liberalize_for_success(order)),
                              static_cast<int>(Atomic::liberalize_for_failure(order)));
  return value;
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<2>::operator()(T volatile* dest, T compare_value, T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(2 == sizeof(T));
  T value = compare_value;
  __atomic_compare_exchange_n(dest, &value, exchange_value, false,
                              static_cast<int>(Atomic::liberalize_for_success(order)),
                              static_cast<int>(Atomic::liberalize_for_failure(order)));
  return value;
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<4>::operator()(T volatile* dest, T compare_value, T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(4 == sizeof(T));
  T value = compare_value;
  __atomic_compare_exchange_n(dest, &value, exchange_value, false,
                              static_cast<int>(Atomic::liberalize_for_success(order)),
                              static_cast<int>(Atomic::liberalize_for_failure(order)));
  return value;
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<8>::operator()(T volatile* dest, T compare_value, T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(8 == sizeof(T));
  T value = compare_value;
  __atomic_compare_exchange_n(dest, &value, exchange_value, false,
                              static_cast<int>(Atomic::liberalize_for_success(order)),
                              static_cast<int>(Atomic::liberalize_for_failure(order)));
  return value;
}

template <size_t N>
template <typename D>
inline bool Atomic::PlatformBitSet<N>::operator()(D volatile* dest, int bit,
                                                  atomic_memory_order order) const {
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  return (__atomic_fetch_or(dest, mask, static_cast<int>(Atomic::liberalize(order))) & mask) == 0;
}
template <size_t N>
template <typename D>
inline bool Atomic::PlatformBitTest<N>::operator()(const D volatile* dest, int bit,
                                                   atomic_memory_order order) const {
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  return (__atomic_load_n(dest, static_cast<int>(Atomic::liberalize(order))) & mask) != 0;
}
template <size_t N>
template <typename D>
inline bool Atomic::PlatformBitClear<N>::operator()(D volatile* dest, int bit,
                                                    atomic_memory_order order) const {
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  return (__atomic_fetch_xor(dest, mask, static_cast<int>(Atomic::liberalize(order))) & mask) == 0;
}

#endif // SHARE_RUNTIME_ATOMIC_GCC_HPP
