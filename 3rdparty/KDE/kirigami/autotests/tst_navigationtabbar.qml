/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import QtTest
import org.kde.kirigami as Kirigami

TestCase {
    id: root

    name: "NavigationTabBarTest"
    visible: true
    when: windowShown

    width: 500
    height: 500

    Component {
        id: emptyComponent
        Kirigami.NavigationTabBar {}
    }

    Component {
        id: tActionComponent
        T.Action {
            text: "Action"
        }
    }

    Component {
        id: kActionComponent
        Kirigami.Action {}
    }

    function test_create() {
        failOnWarning(/error|null/i);
        {
            const tabbar = createTemporaryObject(emptyComponent, this);
            verify(tabbar);
        }
        {
            const tAction = createTemporaryObject(tActionComponent, this);
            const kAction = createTemporaryObject(kActionComponent, this);
            const tabbar = createTemporaryObject(emptyComponent, this);
            verify(tabbar);
            tabbar.actions = [tAction, kAction];
            compare(tabbar.visibleActions, [tAction, kAction]);

            const kInvisibleAction = createTemporaryObject(kActionComponent, this, {
                visible: false,
            });
            verify(kInvisibleAction);
            tabbar.actions.push(kInvisibleAction);
            compare(tabbar.visibleActions, [tAction, kAction]);

            tabbar.actions.push(null);
            compare(tabbar.visibleActions, [tAction, kAction]);

            const tAction2 = createTemporaryObject(tActionComponent, this);
            tabbar.actions.push(tAction2);
            compare(tabbar.visibleActions, [tAction, kAction, tAction2]);
        }
    }

    Component {
        id: pageWithHeader
        QQC2.Page {
            header: Kirigami.NavigationTabBar {}
        }
    }

    Component {
        id: pageWithFooter
        QQC2.Page {
            footer: Kirigami.NavigationTabBar {}
        }
    }

    function test_qqc2_page_position() {
        {
            const page = createTemporaryObject(pageWithHeader, this);
            verify(page);
            const bar = page.header;
            verify(bar instanceof Kirigami.NavigationTabBar);
            compare(bar.position, QQC2.ToolBar.Header);
        }
        {
            const page = createTemporaryObject(pageWithFooter, this);
            verify(page);
            const bar = page.footer;
            verify(bar instanceof Kirigami.NavigationTabBar);
            compare(bar.position, QQC2.ToolBar.Footer);
        }
    }

    Component {
        id: windowWithHeader
        QQC2.ApplicationWindow {
            header: Kirigami.NavigationTabBar {}
        }
    }

    Component {
        id: windowWithFooter
        QQC2.ApplicationWindow {
            footer: Kirigami.NavigationTabBar {}
        }
    }

    function test_qqc2_window_position() {
        {
            const window = createTemporaryObject(windowWithHeader, this);
            verify(window);
            const bar = window.header;
            verify(bar instanceof Kirigami.NavigationTabBar);
            compare(bar.position, QQC2.ToolBar.Header);
        }
        {
            const window = createTemporaryObject(windowWithFooter, this);
            verify(window);
            const bar = window.footer;
            verify(bar instanceof Kirigami.NavigationTabBar);
            compare(bar.position, QQC2.ToolBar.Footer);
        }
    }

    SignalSpy {
        id: actionTriggeredSpy
        signalName: "triggered"
    }

    function test_mnemonics() {
        const tabbar = createTemporaryObject(emptyComponent, this);

        const action = createTemporaryObject(tActionComponent, this);

        tabbar.actions = [action];

        actionTriggeredSpy.target = action;

        // action text "Action" should automatically get "A" mnemonic.
        keyClick(Qt.Key_A, Qt.AltModifier);

        actionTriggeredSpy.wait();
        compare(actionTriggeredSpy.count, 1);
    }
}
