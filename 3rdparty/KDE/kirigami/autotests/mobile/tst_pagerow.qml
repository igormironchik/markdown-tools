/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtTest
import org.kde.kirigami as Kirigami

TestCase {
    id: testCase

    width: 400
    height: 400
    name: "MobilePageRowTest"
    visible: false

    Component {
        id: applicationComponent
        Kirigami.ApplicationWindow {
            // Mobile pagerow logic branches at 40 gridUnits boundary
            width: Kirigami.Units.gridUnit * 30
            height: 400
        }
    }

    Component {
        id: pageComponent
        Kirigami.Page {
            id: page
            property alias closeButton: closeButton
            title: "TestPageComponent"
            QQC2.Button {
                id: closeButton
                anchors.centerIn: parent
                objectName: "CloseDialogButton"
                text: "Close"
                onClicked: page.closeDialog();
            }
        }
    }

    // The following methods are adaptation of QtTest internals

    function waitForWindowActive(window: Window) {
        tryVerify(() => window.active);
    }

    function ensureWindowShown(window: Window) {
        window.requestActivate();
        waitForWindowActive(window);
        wait(0);
    }

    function init() {
        verify(Kirigami.Settings.isMobile);
    }

    function test_pushDialogLayer() {
        const app = createTemporaryObject(applicationComponent, this);
        verify(app);
        ensureWindowShown(app);

        verify(app.pageStack.layers instanceof QQC2.StackView);
        compare(app.pageStack.layers.depth, 1);
        {
            const page = app.pageStack.pushDialogLayer(pageComponent);
            verify(page instanceof Kirigami.Page);
            compare(page.title, "TestPageComponent");
            // Wait for it to finish animating
            tryVerify(() => !app.pageStack.layers.busy);
            compare(app.pageStack.layers.depth, 2);
            mouseClick(page.closeButton);
            tryVerify(() => !app.pageStack.layers.busy);
            compare(app.pageStack.layers.depth, 1);
        }
        app.width = Kirigami.Units.gridUnit * 50;
        {
            const page = app.pageStack.pushDialogLayer(pageComponent);
            verify(page instanceof Kirigami.Page);
            compare(page.title, "TestPageComponent");
            verify(!app.pageStack.layers.busy);
            compare(app.pageStack.layers.depth, 1);
            mouseClick(page.closeButton);
        }
    }
}
