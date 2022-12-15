/*
 * Copyright (c) 2022, BELLSOFT. All rights reserved.
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
 */

import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;

/**
 * @test
 * @library /test/lib
 * @requires vm.hasSA
 * @modules jdk.hotspot.agent/sun.jvm.hotspot.debugger
 *          jdk.hotspot.agent/sun.jvm.hotspot.code
 * @run main/othervm -Xbootclasspath/a:. TestCompressedSparseDataReadStream
 */
public class TestCompressedSparseDataReadStream {

    public static void testReadInt() {
        byte data[] = { (byte)0x08, (byte)0x1f, (byte)0xf0, (byte)0x00 };
        CompressedSparseDataReadStream in = new CompressedSparseDataReadStream(new Addr(data), 0);
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 1);  // 0x81     -> 1
        assertEquals(in.readInt(), 63); // 0xff00   -> 63
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        assertEquals(in.readInt(), 0);  // zero bit -> 0
        in.setPosition(2);
        assertEquals(in.readInt(), 48); // 0xf000   -> 48
    }
    public static void testReadB() {
        byte data[] = { (byte)0x08, (byte)0x18, (byte)0x18, (byte)0x10 };
        CompressedSparseDataReadStream in = new CompressedSparseDataReadStream(new Addr(data), 0);
        assertEquals(in.readByte(), 0);         // zero bit -> 0
        assertEquals(in.readByte(), 0);         // zero bit -> 0
        assertEquals(in.readBoolean(), false);  // zero bit -> false
        assertEquals(in.readBoolean(), false);  // zero bit -> false
        assertEquals(in.readByte(), 1);         // 0x81     -> 1
        assertEquals(in.readBoolean(), true);   // 0x81     -> true
    }
    public static void main(String[] args) {
        testReadInt();
        testReadB();
    }
    private static void assertEquals(int a, int b) {
        if (a != b) throw new RuntimeException("assert failed: " + a + " != " + b);
    }
    private static void assertEquals(boolean a, boolean b) {
        if (a != b) throw new RuntimeException("assert failed: " + a + " != " + b);
    }
}

class DummyAddr implements sun.jvm.hotspot.debugger.Address {
    public boolean    equals(Object arg)                { return false; }
    public int        hashCode()                        { return 0; }
    public long       getCIntegerAt      (long offset, long numBytes, boolean isUnsigned) { return 0; }
    public Address    getAddressAt       (long offset)  { return null; }
    public Address    getCompOopAddressAt (long offset) { return null; }
    public Address    getCompKlassAddressAt (long offset) { return null; }
    public boolean    getJBooleanAt      (long offset)  { return false; }
    public byte       getJByteAt         (long offset)  { return 0; }
    public char       getJCharAt         (long offset)  { return 0; }
    public double     getJDoubleAt       (long offset)  { return 0; }
    public float      getJFloatAt        (long offset)  { return 0; }
    public int        getJIntAt          (long offset)  { return 0; }
    public long       getJLongAt         (long offset)  { return 0; }
    public short      getJShortAt        (long offset)  { return 0; }
    public OopHandle  getOopHandleAt     (long offset)  { return null; }
    public OopHandle  getCompOopHandleAt (long offset)  { return null; }
    public void       setCIntegerAt      (long offset, long numBytes, long value) {}
    public void       setAddressAt       (long offset, Address value) {}
    public void       setJBooleanAt      (long offset, boolean value) {}
    public void       setJByteAt         (long offset, byte value)    {}
    public void       setJCharAt         (long offset, char value)    {}
    public void       setJDoubleAt       (long offset, double value)  {}
    public void       setJFloatAt        (long offset, float value)   {}
    public void       setJIntAt          (long offset, int value)     {}
    public void       setJLongAt         (long offset, long value)    {}
    public void       setJShortAt        (long offset, short value)   {}
    public void       setOopHandleAt     (long offset, OopHandle value) {}
    public Address    addOffsetTo        (long offset)  { return null; }
    public OopHandle  addOffsetToAsOopHandle(long offset)  { return null; }
    public long       minus              (Address arg)  { return 0; }
    public boolean    lessThan           (Address arg)  { return false; }
    public boolean    lessThanOrEqual    (Address arg)  { return false; }
    public boolean    greaterThan        (Address arg)  { return false; }
    public boolean    greaterThanOrEqual (Address arg)  { return false; }
    public Address    andWithMask        (long mask)    { return null; }
    public Address    orWithMask         (long mask)    { return null; }
    public Address    xorWithMask        (long mask)    { return null; }
    public long       asLongValue        ()             { return 0; }
}

class Addr extends DummyAddr {
    byte data[];
    public Addr(byte data[]) {
        this.data = data;
    }
    public long getCIntegerAt(long offset, long numBytes, boolean isUnsigned) {
        return data[(int)offset];
    }
}
