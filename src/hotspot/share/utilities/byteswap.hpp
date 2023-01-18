/*
 * Copyright (c) 2023, Google and/or its affiliates. All rights reserved.
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

#ifndef SHARE_UTILITIES_BYTESWAP_HPP
#define SHARE_UTILITIES_BYTESWAP_HPP

// Byte swapping for 16-bit, 32-bit, and 64-bit integers.

// byteswap<T>()
//
// Reverses the bytes for the value of the integer type T. Partially compatible with std::byteswap
// introduced in C++23

#include "metaprogramming/enableIf.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/macros.hpp"

#if defined(__linux__)
#include <byteswap.h>
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
#endif

#if defined(__OpenBSD__)
#include <endian.h>
#endif

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#endif

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

#if defined(_MSC_VER)
#pragma intrinsic(_byteswap_ushort)
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_uint64)
#endif

template <typename T>
struct CanByteswapImpl final
    : public std::integral_constant<bool, (std::is_integral<T>::value &&
                                           (sizeof(T) == 1 || sizeof(T) == 2 ||
                                            sizeof(T) == 4 || sizeof(T) == 8))> {};

struct Byteswap16Impl final {
  ALWAYSINLINE uint16_t operator()(uint16_t x) const {
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_bswap16)
    return __builtin_bswap16(x);
#elif defined(_MSC_VER)
    return _byteswap_ushort(x);
#elif defined(__linux__)
    return bswap_16(x);
#elif defined(__APPLE__)
    return OSSwapInt16(x);
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    return bswap16(x);
#else
    return (((x & UINT16_C(0x00ff)) << 8) | ((x & UINT16_C(0xff00)) >> 8));
#endif
  }
};

struct Byteswap32Impl final {
  ALWAYSINLINE uint32_t operator()(uint32_t x) const {
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_bswap32)
    return __builtin_bswap32(x);
#elif defined(_MSC_VER)
    return _byteswap_ulong(x);
#elif defined(__linux__)
    return bswap_32(x);
#elif defined(__APPLE__)
    return OSSwapInt32(x);
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    return bswap32(x);
#else
    return (((x & UINT32_C(0x000000ff)) << 24) | ((x & UINT32_C(0x0000ff00)) << 8) |
            ((x & UINT32_C(0x00ff0000)) >> 8) | ((x & UINT32_C(0xff000000)) >> 24));
#endif
  }
};

struct Byteswap64Impl final {
  ALWAYSINLINE uint64_t operator()(uint64_t x) const {
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_bswap64)
    return __builtin_bswap64(x);
#elif defined(_MSC_VER)
    return _byteswap_uint64(x);
#elif defined(__linux__)
    return bswap_64(x);
#elif defined(__APPLE__)
    return OSSwapInt64(x);
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    return bswap64(x);
#else
    return (((x & UINT64_C(0x00000000000000ff)) << 56) | ((x & UINT64_C(0x000000000000ff00)) << 40) |
            ((x & UINT64_C(0x0000000000ff0000)) << 24) | ((x & UINT64_C(0x00000000ff000000)) << 8) |
            ((x & UINT64_C(0x000000ff00000000)) >> 8) | ((x & UINT64_C(0x0000ff0000000000)) >> 24) |
            ((x & UINT64_C(0x00ff000000000000)) >> 40) | ((x & UINT64_C(0xff00000000000000)) >> 56));
#endif
  }
};

template <typename T, size_t N = sizeof(T)>
struct ByteswapImpl;

// We support 8-bit integer types to be compatible with C++23's std::byteswap.
template <typename T>
struct ByteswapImpl<T, 1> final {
  ALWAYSINLINE T operator()(T x) const {
    STATIC_ASSERT(CanByteswapImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 1);
    return x;
  }
};

template <typename T>
struct ByteswapImpl<T, 2> final {
  ALWAYSINLINE T operator()(T x) const {
    STATIC_ASSERT(CanByteswapImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 2);
    return static_cast<T>(Byteswap16Impl{}(static_cast<uint16_t>(x)));
  }
};

template <typename T>
struct ByteswapImpl<T, 4> final {
  ALWAYSINLINE T operator()(T x) const {
    STATIC_ASSERT(CanByteswapImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 4);
    return static_cast<T>(Byteswap32Impl{}(static_cast<uint32_t>(x)));
  }
};

template <typename T>
struct ByteswapImpl<T, 8> final {
  ALWAYSINLINE T operator()(T x) const {
    STATIC_ASSERT(CanByteswapImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 8);
    return static_cast<T>(Byteswap64Impl{}(static_cast<uint64_t>(x)));
  }
};

template <typename T, ENABLE_IF(CanByteswapImpl<T>::value)>
ALWAYSINLINE T byteswap(T x) {
  return ByteswapImpl<T>{}(x);
}

#endif // SHARE_UTILITIES_BYTESWAP_HPP
