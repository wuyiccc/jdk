/*
 * Copyright (c) 2014, 2023, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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

package jdk.internal.jimage;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Objects;

/**
 * @implNote This class needs to maintain JDK 8 source compatibility.
 *
 * It is used internally in the JDK to implement jimage/jrtfs access,
 * but also compiled and delivered as part of the jrtfs.jar to support access
 * to the jimage file provided by the shipped JDK by tools running on JDK 8.
 */
public final class ImageHeader {
    public static final int MAGIC = 0xCAFEDADA;
    public static final int MAJOR_VERSION = 1;
    public static final int MINOR_VERSION = 0;
    private static final int HEADER_SLOTS = 7;

    private final int magic;
    private final int majorVersion;
    private final int minorVersion;
    private final int flags;
    private final int resourceCount;
    private final int tableLength;
    private final int locationsSize;
    private final int stringsSize;

    public ImageHeader(int resourceCount, int tableCount,
            int locationsSize, int stringsSize) {
        this(MAGIC, MAJOR_VERSION, MINOR_VERSION, 0, resourceCount,
                tableCount, locationsSize, stringsSize);
    }

    public ImageHeader(int magic, int majorVersion, int minorVersion,
                int flags, int resourceCount,
                int tableLength, int locationsSize, int stringsSize)
    {
        this.magic = magic;
        this.majorVersion = majorVersion;
        this.minorVersion = minorVersion;
        this.flags = flags;
        this.resourceCount = resourceCount;
        this.tableLength = tableLength;
        this.locationsSize = locationsSize;
        this.stringsSize = stringsSize;
    }

    public static int getHeaderSize() {
       return HEADER_SLOTS * 4;
    }

    static ImageHeader readFrom(IntBuffer buffer) {
        Objects.requireNonNull(buffer);

        if (buffer.capacity() != HEADER_SLOTS) {
            throw new InternalError(
                "jimage header not the correct size: " + buffer.capacity());
        }

        int magic = buffer.get(0);
        int version = buffer.get(1);
        int majorVersion = version >>> 16;
        int minorVersion = version & 0xFFFF;
        int flags = buffer.get(2);
        int resourceCount = buffer.get(3);
        int tableLength = buffer.get(4);
        int locationsSize = buffer.get(5);
        int stringsSize = buffer.get(6);

        return new ImageHeader(magic, majorVersion, minorVersion, flags,
            resourceCount, tableLength, locationsSize, stringsSize);
    }

    public void writeTo(ImageStream stream) {
        Objects.requireNonNull(stream);
        stream.ensure(getHeaderSize());
        writeTo(stream.getBuffer());
    }

    public void writeTo(ByteBuffer buffer) {
        Objects.requireNonNull(buffer);
        buffer.putInt(magic);
        buffer.putInt(majorVersion << 16 | minorVersion);
        buffer.putInt(flags);
        buffer.putInt(resourceCount);
        buffer.putInt(tableLength);
        buffer.putInt(locationsSize);
        buffer.putInt(stringsSize);
    }

    public int getMagic() {
        return magic;
    }

    public int getMajorVersion() {
        return majorVersion;
    }

    public int getMinorVersion() {
        return minorVersion;
    }

    public int getFlags() {
        return flags;
    }

    public int getResourceCount() {
        return resourceCount;
    }

    public int getTableLength() {
        return tableLength;
    }

    public int getRedirectSize() {
        return tableLength * 4;
    }

    public int getOffsetsSize() {
        return tableLength * 4;
    }

    public int getLocationsSize() {
        return locationsSize;
    }

    public int getStringsSize() {
        return stringsSize;
    }

    public int getIndexSize() {
        return getHeaderSize() +
               getRedirectSize() +
               getOffsetsSize() +
               getLocationsSize() +
               getStringsSize();
    }

    int getRedirectOffset() {
        return getHeaderSize();
    }

    int getOffsetsOffset() {
        return getRedirectOffset() +
               getRedirectSize();
    }

    int getLocationsOffset() {
        return getOffsetsOffset() +
               getOffsetsSize();
    }

    int getStringsOffset() {
        return getLocationsOffset() +
               getLocationsSize();
    }

    /**
     * Tries to determine the {@link ByteOrder} of a image file present at {@code imageFile}.
     * The implementation reads the first few relevant bytes of the image file and compares it to
     * the expected magic bytes of a jimage. If those bytes correspond to the magic bytes, then
     * this method returns the ByteOrder of the image file, else it returns null.
     *
     * @param imageFile The path to the image file
     * @return The ByteOrder if one could be determined. Else null.
     * @throws IOException If any error occurred reading the contents of the image file.
     */
    static ByteOrder tryDetectByteOrder(final Path imageFile) throws IOException {
        final byte[] bytes = new byte[Integer.SIZE]; // read only as much as the size of magic bytes
        try (final InputStream is = Files.newInputStream(imageFile)) {
            final int numRead = is.read(bytes);
            if (numRead != bytes.length) {
                // not enough bytes to ascertain if this is a jimage
                return null;
            }
        }
        final ByteBuffer bb = ByteBuffer.wrap(bytes);
        if (bb.getInt() == MAGIC) {
            return bb.order();
        }
        // try with the other byte order
        final ByteOrder altByteOrder = bb.order() == ByteOrder.BIG_ENDIAN
                ? ByteOrder.LITTLE_ENDIAN : ByteOrder.BIG_ENDIAN;
        bb.flip();
        bb.order(altByteOrder);
        if (bb.getInt() == MAGIC) {
            return bb.order();
        }
        // doesn't match the magic bytes
        return null;
    }
}
