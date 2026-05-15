/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import QtTest
import org.kde.kirigami as Kirigami

TestCase {
    id: testCase

    width: 400
    height: 400
    name: "ScrollablePageTest"
    visible: false

    SignalSpy {
        id: currentItemChangedSpy
        target: mainWindow.pageStack
        signalName: "currentItemChanged"
    }

    component FirstPage: Kirigami.Page {
        id: root
        function clickFirst() {
            (userList.itemAtIndex(0) as QQC.ItemDelegate).clicked();
        }
        property alias view: scroll.view
        view: ListView {
            id: userList
            model: 1
            delegate: QQC.ItemDelegate {
                id: delegate

                required property int index

                width: ListView.view.width
                text: String(index)
                onClicked: {
                    mainWindow.pageStack.pop();
                    mainWindow.pageStack.push(subPageComponent);
                    console.log("clicked")
                }
            }
        }
        QQC.ScrollView {
            id: scroll
            anchors.fill: parent
            property Flickable view
            activeFocusOnTab: false
            contentItem: view
            onViewChanged: {
                view.parent = scroll;
            }
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
        }
    }

    component SubPage: Kirigami.ScrollablePage {
        readonly property alias success: realNametextField.activeFocus
        focus: true
        ColumnLayout {
            QQC.TextField {
                id: realNametextField
                focus: true
                text: "This item should be focused by default"
            }
            QQC.TextField {
                id: userNameField
                text: "ddeeff"
            }
        }
    }

    Component {
        id: subPageComponent

        SubPage { }
    }

    // Simulate KCM.ScrollViewKCM
    Kirigami.ApplicationWindow {
        id: mainWindow
        width: 480
        height: 360
        pageStack.initialPage: FirstPage { }
    }


    function initTestCase() {
        mainWindow.show()
        mainWindow.requestActivate();
        tryVerify(() => mainWindow.active);
    }

    function cleanupTestCase() {
        mainWindow.close()
    }

    function test_defaultFocusInScrollablePage() {
        (mainWindow.pageStack.currentItem as FirstPage).clickFirst();
        if (!(mainWindow.pageStack.currentItem instanceof Kirigami.ScrollablePage)) {
            currentItemChangedSpy.wait()
        }
        verify(mainWindow.pageStack.currentItem instanceof Kirigami.ScrollablePage)
        // Consolidate the workaround for QTBUG-44043
        // https://invent.kde.org/frameworks/kirigami/-/commit/fd253ea5d9fa3f33411e54a596c021f51b5a102a
        tryVerify(() => (mainWindow.pageStack.currentItem as SubPage).success)
    }
}



