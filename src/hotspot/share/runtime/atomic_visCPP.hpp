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

#ifndef SHARE_RUNTIME_ATOMIC_VISCPP_HPP
#define SHARE_RUNTIME_ATOMIC_VISCPP_HPP

#ifndef SHARE_RUNTIME_ATOMIC_HPP
#error atomic_visCPP.hpp cannot not be included directly, use atomic.hpp
#endif

#include "metaprogramming/primitiveConversions.hpp"
#include "utilities/debug.hpp"

#include <intrin.h>

#include <type_traits>

template <>
template <typename T>
inline T Atomic::PlatformLoad<1>::operator()(T const volatile* src) const {
  STATIC_ASSERT(1 == sizeof(T));
  return static_cast<T>(_InterlockedCompareExchange8(reinterpret_cast<char volatile*>(const_cast<T volatile*>(src)), 0, 0));
}

template <>
template <typename T>
inline T Atomic::PlatformLoad<2>::operator()(T const volatile* src) const {
  STATIC_ASSERT(2 == sizeof(T));
  return static_cast<T>(_InterlockedCompareExchange16(reinterpret_cast<short volatile*>(const_cast<T volatile*>(src)), 0, 0));
}

template <>
template <typename T>
inline T Atomic::PlatformLoad<4>::operator()(T const volatile* src) const {
  STATIC_ASSERT(4 == sizeof(T));
  return static_cast<T>(_InterlockedCompareExchange(reinterpret_cast<long volatile*>(const_cast<T volatile*>(src)), 0, 0));
}

template <>
template <typename T>
inline T Atomic::PlatformLoad<8>::operator()(T const volatile* src) const {
  STATIC_ASSERT(8 == sizeof(T));
  return static_cast<T>(_InterlockedCompareExchange64(reinterpret_cast<__int64 volatile*>(const_cast<T volatile*>(src)), 0, 0));
}

template <>
template <typename T>
inline void Atomic::PlatformStore<1>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(1 == sizeof(T));
  static_cast<void>(_InterlockedExchange8(reinterpret_cast<char volatile*>(dest), static_cast<char>(store_value)));
}

template <>
template <typename T>
inline void Atomic::PlatformStore<2>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(2 == sizeof(T));
  static_cast<void>(_InterlockedExchange16(reinterpret_cast<short volatile*>(dest), static_cast<short>(store_value)));
}

template <>
template <typename T>
inline void Atomic::PlatformStore<4>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(4 == sizeof(T));
  static_cast<void>(_InterlockedExchange(reinterpret_cast<long volatile*>(dest), static_cast<long>(store_value)));
}

template <>
template <typename T>
inline void Atomic::PlatformStore<8>::operator()(T volatile* dest,
                                                 T store_value) const {
  STATIC_ASSERT(8 == sizeof(T));
  static_cast<void>(_InterlockedExchange64(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(store_value)));
}

template <>
struct Atomic::PlatformOrderedLoad<1, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(1 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    return static_cast<T>(_InterlockedCompareExchange8_acq(reinterpret_cast<char volatile*>(const_cast<T volatile*>(p)), 0, 0));
#else
    return Atomic::PlatformLoad<1>{}(p);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedLoad<2, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(2 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    return static_cast<T>(_InterlockedCompareExchange16_acq(reinterpret_cast<short volatile*>(const_cast<T volatile*>(p)), 0, 0));
#else
    return Atomic::PlatformLoad<2>{}(p);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedLoad<4, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(4 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    return static_cast<T>(_InterlockedCompareExchange_acq(reinterpret_cast<long volatile*>(const_cast<T volatile*>(p)), 0, 0));
#else
    return Atomic::PlatformLoad<4>{}(p);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedLoad<8, X_ACQUIRE> {
  template <typename T>
  inline T operator()(const volatile T* p) const {
    STATIC_ASSERT(8 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    return static_cast<T>(_InterlockedCompareExchange64_acq(reinterpret_cast<__int64 volatile*>(const_cast<T volatile*>(p)), 0, 0));
#else
    return Atomic::PlatformLoad<8>{}(p);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedStore<1, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(1 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    static_cast<void>(_InterlockedExchange8_rel(reinterpret_cast<char volatile*>(p), static_cast<char>(v)));
#else
    Atomic::PlatformStore<1>{}(p, v);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedStore<2, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(2 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    static_cast<void>(_InterlockedExchange16_rel(reinterpret_cast<short volatile*>(p), static_cast<short>(v)));
#else
    Atomic::PlatformStore<2>{}(p, v);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedStore<4, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(4 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    static_cast<void>(_InterlockedExchange_rel(reinterpret_cast<long volatile*>(p), static_cast<long>(v)));
#else
    Atomic::PlatformStore<4>{}(p, v);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedStore<8, RELEASE_X> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(8 == sizeof(T));
#if defined(_M_ARM) || defined(_M_ARM64)
    static_cast<void>(_InterlockedExchange64_rel(reinterpret_cast<__int64 volatile*>(p), static_cast<__int64>(v)));
#else
    Atomic::PlatformStore<8>{}(p, v);
#endif
  }
};

template <>
struct Atomic::PlatformOrderedStore<1, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(1 == sizeof(T));
    Atomic::PlatformOrderedStore<1, RELEASE_X>{}(p, v);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformOrderedStore<2, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(2 == sizeof(T));
    Atomic::PlatformOrderedStore<2, RELEASE_X>{}(p, v);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformOrderedStore<4, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(4 == sizeof(T));
    Atomic::PlatformOrderedStore<4, RELEASE_X>{}(p, v);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformOrderedStore<8, RELEASE_X_FENCE> {
  template <typename T>
  inline void operator()(volatile T* p, T v) const {
    STATIC_ASSERT(8 == sizeof(T));
    Atomic::PlatformOrderedStore<8, RELEASE_X>{}(p, v);
    OrderAccess::fence();
  }
};

template <>
struct Atomic::PlatformAdd<1> {
  template<typename D, typename I>
  inline D fetch_and_add(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 1);
    switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
      case memory_order_relaxed:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd8_nf(reinterpret_cast<char volatile*>(dest), PrimitiveConversions::cast<char>(add_value)));
      case memory_order_acquire:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd8_acq(reinterpret_cast<char volatile*>(dest), PrimitiveConversions::cast<char>(add_value)));
      case memory_order_release:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd8_rel(reinterpret_cast<char volatile*>(dest), PrimitiveConversions::cast<char>(add_value)));
#endif
      default:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd8(reinterpret_cast<char volatile*>(dest), PrimitiveConversions::cast<char>(add_value)));
    }
  }

  template<typename D, typename I>
  inline D add_and_fetch(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 1);
    return fetch_and_add(dest, add_value, order) + add_value;
  }
};

template <>
struct Atomic::PlatformAdd<2> {
  template<typename D, typename I>
  inline D fetch_and_add(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 2);
    switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
      case memory_order_relaxed:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd16_nf(reinterpret_cast<short volatile*>(dest), PrimitiveConversions::cast<short>(add_value)));
      case memory_order_acquire:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd16_acq(reinterpret_cast<short volatile*>(dest), PrimitiveConversions::cast<short>(add_value)));
      case memory_order_release:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd16_rel(reinterpret_cast<short volatile*>(dest), PrimitiveConversions::cast<short>(add_value)));
#endif
      default:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd16(reinterpret_cast<short volatile*>(dest), PrimitiveConversions::cast<short>(add_value)));
    }
  }

  template<typename D, typename I>
  inline D add_and_fetch(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 2);
    return fetch_and_add(dest, add_value, order) + add_value;
  }
};

template <>
struct Atomic::PlatformAdd<4> {
  template<typename D, typename I>
  inline D fetch_and_add(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 4);
    switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
      case memory_order_relaxed:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd_nf(reinterpret_cast<long volatile*>(dest), PrimitiveConversions::cast<long>(add_value)));
      case memory_order_acquire:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd_acq(reinterpret_cast<long volatile*>(dest), PrimitiveConversions::cast<long>(add_value)));
      case memory_order_release:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd_rel(reinterpret_cast<long volatile*>(dest), PrimitiveConversions::cast<long>(add_value)));
#endif
      default:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd(reinterpret_cast<long volatile*>(dest), PrimitiveConversions::cast<long>(add_value)));
    }
  }

  template<typename D, typename I>
  inline D add_and_fetch(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 4);
    return fetch_and_add(dest, add_value, order) + add_value;
  }
};

template <>
struct Atomic::PlatformAdd<8> {
  template<typename D, typename I>
  inline D fetch_and_add(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 8);
    switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
      case memory_order_relaxed:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd64_nf(reinterpret_cast<__int64 volatile*>(dest), PrimitiveConversions::cast<__int64>(add_value)));
      case memory_order_acquire:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd64_acq(reinterpret_cast<__int64 volatile*>(dest), PrimitiveConversions::cast<__int64>(add_value)));
      case memory_order_release:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd64_rel(reinterpret_cast<__int64 volatile*>(dest), PrimitiveConversions::cast<__int64>(add_value)));
#endif
      default:
        return PrimitiveConversions::cast<D>(_InterlockedExchangeAdd64(reinterpret_cast<__int64 volatile*>(dest), PrimitiveConversions::cast<__int64>(add_value)));
    }
  }

  template<typename D, typename I>
  inline D add_and_fetch(D volatile* dest, I add_value, atomic_memory_order order) const {
    STATIC_ASSERT(sizeof(D) == 8);
    return fetch_and_add(dest, add_value, order) + add_value;
  }
};

template <>
template <typename T>
inline T Atomic::PlatformXchg<1>::operator()(T volatile* dest,
                                             T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(1 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedExchange8_nf(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedExchange8_acq(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedExchange8_rel(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value)));
#endif
    default:
      return static_cast<T>(_InterlockedExchange8(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformXchg<2>::operator()(T volatile* dest,
                                             T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(2 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedExchange16_nf(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedExchange16_acq(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedExchange16_rel(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value)));
#endif
    default:
      return static_cast<T>(_InterlockedExchange16(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformXchg<4>::operator()(T volatile* dest,
                                             T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(4 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedExchange_nf(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedExchange_acq(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedExchange_rel(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value)));
#endif
    default:
      return static_cast<T>(_InterlockedExchange(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformXchg<8>::operator()(T volatile* dest,
                                             T exchange_value,
                                             atomic_memory_order order) const {
  STATIC_ASSERT(8 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedExchange64_nf(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedExchange64_acq(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedExchange64_rel(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value)));
#endif
    default:
      return static_cast<T>(_InterlockedExchange64(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<1>::operator()(T volatile* dest,
                                                T compare_value,
                                                T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(1 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedCompareExchange8_nf(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value), static_cast<char>(compare_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedCompareExchange8_acq(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value), static_cast<char>(compare_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedCompareExchange8_rel(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value), static_cast<char>(compare_value)));
#endif
    default:
      return static_cast<T>(_InterlockedCompareExchange8(reinterpret_cast<char volatile*>(dest), static_cast<char>(exchange_value), static_cast<char>(compare_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<2>::operator()(T volatile* dest,
                                                T compare_value,
                                                T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(2 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedCompareExchange16_nf(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value), static_cast<short>(compare_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedCompareExchange16_acq(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value), static_cast<short>(compare_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedCompareExchange16_rel(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value), static_cast<short>(compare_value)));
#endif
    default:
      return static_cast<T>(_InterlockedCompareExchange16(reinterpret_cast<short volatile*>(dest), static_cast<short>(exchange_value), static_cast<short>(compare_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<4>::operator()(T volatile* dest,
                                                T compare_value,
                                                T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(4 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedCompareExchange_nf(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value), static_cast<long>(compare_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedCompareExchange_acq(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value), static_cast<long>(compare_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedCompareExchange_rel(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value), static_cast<long>(compare_value)));
#endif
    default:
      return static_cast<T>(_InterlockedCompareExchange(reinterpret_cast<long volatile*>(dest), static_cast<long>(exchange_value), static_cast<long>(compare_value)));
  }
}

template <>
template <typename T>
inline T Atomic::PlatformCmpxchg<8>::operator()(T volatile* dest,
                                                T compare_value,
                                                T exchange_value,
                                                atomic_memory_order order) const {
  STATIC_ASSERT(8 == sizeof(T));
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      return static_cast<T>(_InterlockedCompareExchange64_nf(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value), static_cast<__int64>(compare_value)));
    case memory_order_acquire:
      return static_cast<T>(_InterlockedCompareExchange64_acq(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value), static_cast<__int64>(compare_value)));
    case memory_order_release:
      return static_cast<T>(_InterlockedCompareExchange64_rel(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value), static_cast<__int64>(compare_value)));
#endif
    default:
      return static_cast<T>(_InterlockedCompareExchange64(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(exchange_value), static_cast<__int64>(compare_value)));
  }
}

template <>
template <typename D>
inline bool Atomic::PlatformBitSet<1>::operator()(D volatile* dest, int bit,
                                                        atomic_memory_order order) const {
  STATIC_ASSERT(1 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedOr8_nf(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedOr8_acq(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedOr8_rel(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedOr8(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
  }
  return (fetched & mask) == 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitSet<2>::operator()(D volatile* dest, int bit,
                                                        atomic_memory_order order) const {
  STATIC_ASSERT(2 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedOr16_nf(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedOr16_acq(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedOr16_rel(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedOr16(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
  }
  return (fetched & mask) == 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitSet<4>::operator()(D volatile* dest, int bit,
                                                        atomic_memory_order order) const {
  STATIC_ASSERT(4 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedOr_nf(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedOr_acq(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedOr_rel(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedOr(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
  }
  return (fetched & mask) == 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitSet<8>::operator()(D volatile* dest, int bit,
                                                        atomic_memory_order order) const {
  STATIC_ASSERT(8 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedOr64_nf(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedOr64_acq(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedOr64_rel(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedOr64(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
  }
  return (fetched & mask) == 0;
}

template <size_t N>
template <typename D>
inline bool Atomic::PlatformBitTest<N>::operator()(const D volatile* dest, int bit,
                                                        atomic_memory_order order) const {
  D fetched;
  switch (order) {
    case memory_order_acquire:
      fetched = Atomic::load_acquire(dest);
      break;
    default:
      fetched = Atomic::load(dest);
      break;
  }
  using U = std::make_unsigned_t<D>;
  return (fetched & static_cast<D>(U{1} << bit)) != 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitClear<1>::operator()(D volatile* dest, int bit,
                                                          atomic_memory_order order) const {
  STATIC_ASSERT(1 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedXor8_nf(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedXor8_acq(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedXor8_rel(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedXor8(reinterpret_cast<char volatile*>(dest), static_cast<char>(mask)));
      break;
  }
  return (fetched & mask) != 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitClear<2>::operator()(D volatile* dest, int bit,
                                                          atomic_memory_order order) const {
  STATIC_ASSERT(2 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedXor16_nf(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedXor16_acq(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedXor16_rel(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedXor16(reinterpret_cast<short volatile*>(dest), static_cast<short>(mask)));
      break;
  }
  return (fetched & mask) != 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitClear<4>::operator()(D volatile* dest, int bit,
                                                          atomic_memory_order order) const {
  STATIC_ASSERT(4 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedXor_nf(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedXor_acq(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedXor_rel(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedXor(reinterpret_cast<long volatile*>(dest), static_cast<long>(mask)));
      break;
  }
  return (fetched & mask) != 0;
}

template <>
template <typename D>
inline bool Atomic::PlatformBitClear<8>::operator()(D volatile* dest, int bit,
                                                          atomic_memory_order order) const {
  STATIC_ASSERT(8 == sizeof(D));
  using U = std::make_unsigned_t<D>;
  constexpr D mask = static_cast<D>(U{1} << bit);
  D fetched;
  switch (order) {
#if defined(_M_ARM) || defined(_M_ARM64)
    case memory_order_relaxed:
      fetched = static_cast<D>(_InterlockedXor64_nf(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
    case memory_order_acquire:
      fetched = static_cast<D>(_InterlockedXor64_acq(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
    case memory_order_release:
      fetched = static_cast<D>(_InterlockedXor64_rel(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
#endif
    default:
      fetched = static_cast<D>(_InterlockedXor64(reinterpret_cast<__int64 volatile*>(dest), static_cast<__int64>(mask)));
      break;
  }
  return (fetched & mask) != 0;
}

#endif // SHARE_RUNTIME_ATOMIC_VISCPP_HPP
