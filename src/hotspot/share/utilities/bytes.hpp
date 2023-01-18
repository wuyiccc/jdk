/*
 * Copyright (c) 2014, 2019, 2023, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_UTILITIES_BYTES_HPP
#define SHARE_UTILITIES_BYTES_HPP

#include "memory/allStatic.hpp"
#include "utilities/byteswap.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/macros.hpp"
#include "utilities/unalignedAccess.hpp"

class Endian final : public AllStatic {
 public:
  enum Order {
    LITTLE,
    BIG,
    JAVA = BIG,
    NATIVE =
#ifdef VM_LITTLE_ENDIAN
    LITTLE
#else
    BIG
#endif
  };

  // Returns true, if the byte ordering used by Java is different from
  // the native byte ordering of the underlying machine.
  static constexpr bool is_Java_byte_ordering_different() {
    return NATIVE != JAVA;
  }
};

template <typename T, bool D = Endian::is_Java_byte_ordering_different()>
struct BytesSwapImpl;

template <typename T>
struct BytesSwapImpl<T, false> final {
  ALWAYSINLINE T operator()(T x) const {
    return x;
  }
};

template <typename T>
struct BytesSwapImpl<T, true> final {
  ALWAYSINLINE T operator()(T x) const {
    return byteswap<T>(x);
  }
};

class Bytes final : public AllStatic {
 public:
  static ALWAYSINLINE u2 get_native_u2(const void* p) { return UnalignedAccess::load<u2>(p); }
  static ALWAYSINLINE u4 get_native_u4(const void* p) { return UnalignedAccess::load<u4>(p); }
  static ALWAYSINLINE u8 get_native_u8(const void* p) { return UnalignedAccess::load<u8>(p); }

  static ALWAYSINLINE void put_native_u2(void* p, u2 x) { UnalignedAccess::store<u2>(p, x); }
  static ALWAYSINLINE void put_native_u4(void* p, u4 x) { UnalignedAccess::store<u4>(p, x); }
  static ALWAYSINLINE void put_native_u8(void* p, u8 x) { UnalignedAccess::store<u8>(p, x); }

  static ALWAYSINLINE u2 get_Java_u2(const void* p) { return BytesSwapImpl<u2>{}(get_native_u2(p)); }
  static ALWAYSINLINE u4 get_Java_u4(const void* p) { return BytesSwapImpl<u4>{}(get_native_u4(p)); }
  static ALWAYSINLINE u8 get_Java_u8(const void* p) { return BytesSwapImpl<u8>{}(get_native_u8(p)); }

  static ALWAYSINLINE void put_Java_u2(void* p, u2 x) { put_native_u2(p, BytesSwapImpl<u2>{}(x)); }
  static ALWAYSINLINE void put_Java_u4(void* p, u4 x) { put_native_u4(p, BytesSwapImpl<u4>{}(x)); }
  static ALWAYSINLINE void put_Java_u8(void* p, u8 x) { put_native_u8(p, BytesSwapImpl<u8>{}(x)); }

  static ALWAYSINLINE u2 swap_u2(u2 x) { return byteswap<u2>(x); }
  static ALWAYSINLINE u4 swap_u4(u4 x) { return byteswap<u4>(x); }
  static ALWAYSINLINE u8 swap_u8(u8 x) { return byteswap<u8>(x); }
};

#endif // SHARE_UTILITIES_BYTES_HPP
