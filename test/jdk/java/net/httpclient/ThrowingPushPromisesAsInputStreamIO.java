/*
 * Copyright (c) 2018, 2023, Oracle and/or its affiliates. All rights reserved.
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

/*
 * @test
 * @bug 8229822
 * @summary Tests what happens when push promise handlers and their
 *          response body handlers and subscribers throw unexpected exceptions.
 * @library /test/lib /test/jdk/java/net/httpclient/lib
 * @build jdk.test.lib.net.SimpleSSLContext
 *        ReferenceTracker AbstractThrowingPushPromises ThrowingPushPromisesAsInputStreamIO
 *        jdk.httpclient.test.lib.common.HttpServerAdapters
 * @modules java.base/sun.net.www.http
 *          java.net.http/jdk.internal.net.http.common
 *          java.net.http/jdk.internal.net.http.frame
 *          java.net.http/jdk.internal.net.http.hpack
 * @run testng/othervm -Djdk.internal.httpclient.debug=true ThrowingPushPromisesAsInputStreamIO
 */

import org.testng.annotations.Test;

public class ThrowingPushPromisesAsInputStreamIO extends AbstractThrowingPushPromises {

    @Test(dataProvider = "ioVariants")
    public void testThrowingAsInputStream(String uri, boolean sameClient, Thrower thrower)
            throws Exception {
        super.testThrowingAsInputStreamImpl(uri, sameClient, thrower);
    }
}
