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

#ifndef SHARE_UTILITIES_UNALIGNED_ACCESS_HPP
#define SHARE_UTILITIES_UNALIGNED_ACCESS_HPP

// Support for well defined potentially unaligned memory access, regardless of underlying
// architecture support.
//
// Unaligned access is undefined behavior according to the standard. Some architectures support
// aligned and unaligned memory accesses via the same instructions (i.e. x86, AArch64) while some do
// not (i.e. RISC-V, PowerPC). Compilers are free to assume that all memory accesses of a type T are
// done at a suitably aligned address for type T, that is an address aligned to alignof(T). This is
// not always the case, as there are use cases where we may want to access type T at a non-suitably
// aligned address. For example, when serializing scalar types to a buffer without padding.

// UnalignedAccess<T>::load()
//
// Loads the bits of the value of type T from the specified address. The address may or may not be
// suitably aligned for type T. T must be trivially copyable and must be default constructible.

// UnalignedAccess<T>::store()
//
// Stores the bits of the value of type T at the specified address. The address may or may not be
// suitably aligned for type T. T must be trivially copyable and must be default constructible.

#include "memory/allStatic.hpp"
#include "metaprogramming/enableIf.hpp"
#include "utilities/debug.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/macros.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#if defined(ADDRESS_SANITIZER)
#include <sanitizer/common_interface_defs.h>
#endif

template <typename T>
struct CanAccessUnalignedImpl final
    : public std::integral_constant<bool, (std::is_trivially_copyable<T>::value &&
                                           std::is_default_constructible<T>::value)> {};

template <typename T, size_t S = sizeof(T), size_t A = alignof(T)>
struct UnalignedLoadImpl final {
  ALWAYSINLINE T operator()(const void* p) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == S);
    STATIC_ASSERT(alignof(T) == A);
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_memcpy)
    // When available, explicitly prefer the builtin memcpy variant. This ensures GCC/Clang will
    // do its best at generating optimal machine code regardless of build options. For architectures
    // which support unaligned access, this typically results in a single instruction. For other
    // architectures, GCC/Clang will attempt to determine if the access is aligned first at compile
    // time and generate a single instruction otherwise it will fallback to a more general approach.
    T x;
    __builtin_memcpy(&x, p, S);
    return x;
#elif defined(_MSC_VER)
    return *static_cast<__unaligned const T*>(p);
#else
    // Most compilers will generate optimal machine code.
    T x;
    std::memcpy(&x, p, S);
    return x;
#endif
  }
};

template <typename T, size_t S = sizeof(T), size_t A = alignof(T)>
struct UnalignedStoreImpl final {
  ALWAYSINLINE void operator()(void* p, T x) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == S);
    STATIC_ASSERT(alignof(T) == A);
#if (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_memcpy)
    // When available, explicitly prefer the builtin memcpy variant. This ensures GCC/Clang will
    // do its best at generating optimal machine code regardless of build options. For architectures
    // which support unaligned access, this typically results in a single instruction. For other
    // architectures, GCC/Clang will attempt to determine if the access is aligned first at compile
    // time and generate a single instruction otherwise it will fallback to a more general approach.
    __builtin_memcpy(p, &x, S);
#elif defined(_MSC_VER)
    *static_cast<__unaligned T*>(p) = x;
#else
    // Most compilers will generate optimal machine code.
    std::memcpy(p, &x, S);
#endif
  }
};

// Loads for types with an alignment of 1 byte are always aligned, but for simplicity of
// metaprogramming we accept them in UnalignedAccess.
template <typename T, size_t S>
struct UnalignedLoadImpl<T, S, 1> final {
  ALWAYSINLINE T operator()(const void* p) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(alignof(T) == 1);
    return *static_cast<const T*>(p);
  }
};

// Stores for types with an alignment of 1 byte are always aligned, but for simplicity of
// metaprogramming we accept them in UnalignedAccess.
template <typename T, size_t S>
struct UnalignedStoreImpl<T, S, 1> final {
  ALWAYSINLINE void operator()(void* p, T x) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(alignof(T) == 1);
    *static_cast<T*>(p) = x;
  }
};

template <typename To>
struct UnalignedBitCastImpl final {
  template <typename From>
  ALWAYSINLINE To operator()(const From& from) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<From>::value);
    STATIC_ASSERT(CanAccessUnalignedImpl<To>::value);
    STATIC_ASSERT(sizeof(To) == sizeof(From));
#if HAS_BUILTIN(__builtin_bit_cast)
    return __builtin_bit_cast(To, from);
#elif (defined(__GNUC__) && !defined(__clang__)) || HAS_BUILTIN(__builtin_memcpy)
    To to;
    __builtin_memcpy(&to, &from, sizeof(To));
    return to;
#else
    To to;
    std::memcpy(&to, &from, sizeof(To));
    return to;
#endif
  }
};

#if defined(ADDRESS_SANITIZER)
// Intercept unaligned accesses of size 2, 4, and 8 for ASan which can miss some bugs related to
// unaligned accesses if these are not used.
//
// NOTE: these should also be enabled for MSan and TSan as well when/if we use those.

template <typename T>
struct UnalignedLoadImpl<T, 2, 2> final {
  ALWAYSINLINE T operator()(const void* p) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 2);
    STATIC_ASSERT(alignof(T) == 2);
    return UnalignedBitCastImpl<T>{}(__sanitizer_unaligned_load16(p));
  }
};

template <typename T>
struct UnalignedStoreImpl<T, 2, 2> final {
  ALWAYSINLINE void operator()(void* p, T x) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 2);
    STATIC_ASSERT(alignof(T) == 2);
    __sanitizer_unaligned_store16(p, UnalignedBitCastImpl<uint16_t>{}(x));
  }
};

template <typename T>
struct UnalignedLoadImpl<T, 4, 4> final {
  ALWAYSINLINE T operator()(const void* p) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 4);
    STATIC_ASSERT(alignof(T) == 4);
    return UnalignedBitCastImpl<T>{}(__sanitizer_unaligned_load32(p));
  }
};

template <typename T>
struct UnalignedStoreImpl<T, 4, 4> final {
  ALWAYSINLINE void operator()(void* p, T x) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 4);
    STATIC_ASSERT(alignof(T) == 4);
    __sanitizer_unaligned_store32(p, UnalignedBitCastImpl<uint32_t>{}(x));
  }
};

template <typename T>
struct UnalignedLoadImpl<T, 8, 8> final {
  ALWAYSINLINE T operator()(const void* p) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 8);
    STATIC_ASSERT(alignof(T) == 8);
    return UnalignedBitCastImpl<T>{}(__sanitizer_unaligned_load64(p));
  }
};

template <typename T>
struct UnalignedStoreImpl<T, 8, 8> final {
  ALWAYSINLINE void operator()(void* p, T x) const {
    STATIC_ASSERT(CanAccessUnalignedImpl<T>::value);
    STATIC_ASSERT(sizeof(T) == 8);
    STATIC_ASSERT(alignof(T) == 8);
    __sanitizer_unaligned_store64(p, UnalignedBitCastImpl<uint64_t>{}(x));
  }
};
#endif

class UnalignedAccess final : public AllStatic {
 public:
  // Load the bits of the value x of type T from the specified address p.
  template <typename T, ENABLE_IF(CanAccessUnalignedImpl<T>::value)>
  static ALWAYSINLINE T load(const void* p) {
    return UnalignedLoadImpl<T>{}(p);
  }

  // Store the bits of the value x of type U, which must be implicitly convertible to type T, at the
  // specified address p. This approach requires explicitly specifying type T for readability,
  // rather than deriving type T from the argument.
  template <typename T, typename U = T, ENABLE_IF(CanAccessUnalignedImpl<T>::value &&
                                                  std::is_convertible<U, T>::value)>
  static ALWAYSINLINE void store(void* p, U x) {
    UnalignedStoreImpl<T>{}(p, x);
  }
};

#endif // SHARE_UTILITIES_UNALIGNED_ACCESS_HPP
