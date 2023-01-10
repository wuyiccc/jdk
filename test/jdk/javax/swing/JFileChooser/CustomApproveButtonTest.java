/*
 * Copyright (c) 2023, Oracle and/or its affiliates. All rights reserved.
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
import java.awt.Component;
import java.awt.Container;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;

/*
 * @test
 * @bug 8299522
 * @key headful
 * @summary JFileChooser Test to check for FileChooser's Approve Button Text
 *          when CUSTOM_DIALOG type is set.
 * @run main CustomApproveButtonTest
 */

public class CustomApproveButtonTest {
    private final String customApproveText;
    private final String lookAndFeel;
    private JFrame frame;

    public static void main(String[] args) throws Exception {
        SwingUtilities.invokeAndWait(new Runnable() {
            public void run() {
                UIManager.LookAndFeelInfo[] lookAndFeel = UIManager.getInstalledLookAndFeels();
                for (UIManager.LookAndFeelInfo look : lookAndFeel) {
                    if (look.getName().equals("Aqua")) {
                        continue;
                    }
                    new CustomApproveButtonTest(look.getClassName());
                }
            }
        });
        System.out.println("Test Pass!");
    }

    private CustomApproveButtonTest(String lookAndFeel) {
        System.out.println("Testing Look & Feel : " + lookAndFeel);
        frame = new JFrame("CustomApproveButtonTest");
        this.lookAndFeel = lookAndFeel;

        try {
            UIManager.setLookAndFeel(lookAndFeel);
        } catch (Exception e) {
            fail("Failed to set ", e);
        }

        JFileChooser fileChooser = new JFileChooser();
        fileChooser.setDialogType(JFileChooser.CUSTOM_DIALOG);
        frame.add(fileChooser);
        frame.setVisible(true);
        frame.pack();

        customApproveText = UIManager.getString("FileChooser.customApproveButtonText", fileChooser.getLocale());

        if (customApproveText == null || customApproveText.length() == 0) {
            fail("Cannot find Custom Approve Button text From UIManager Property!");
            return;
        }

        JButton customApproveButton = findCustomApproveButton(fileChooser);

        if (customApproveButton == null) {
            fail("Cannot find Custom Approve Button in FileChooser!");
            return;
        }

        if (frame != null) {
            frame.dispose();
        }

    }

    private JButton findCustomApproveButton(Container container) {
        JButton result = null;

        for (int i = 0; i < container.getComponentCount(); i++) {
            Component c = container.getComponent(i);

            if (c instanceof JButton && customApproveText.equals(((JButton) c).getText())) {
                result = (JButton) c;
                break;
            }

            if (c instanceof Container) {
                JButton button = findCustomApproveButton((Container) c);

                if (button != null && result == null) {
                    result = button;
                    break;
                }
            }
        }
        return result;
    }

    private void fail(String s) {
        if (frame != null) {
            frame.dispose();
        }
        throw new RuntimeException("Test failed: " + s);
    }

    private void fail(String s, Throwable e) {
        throw new RuntimeException(s + lookAndFeel + " LookAndFeel. Error : " + s, e);
    }
}
