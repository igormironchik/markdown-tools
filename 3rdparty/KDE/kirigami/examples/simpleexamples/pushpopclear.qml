/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    globalDrawer: Kirigami.GlobalDrawer {
        actions: [
            Kirigami.Action {
                text: "push"
                onTriggered: root.pageStack.push(secondPageComponent)
            },
            Kirigami.Action {
                text: "pop"
                onTriggered: root.pageStack.pop()
            },
            Kirigami.Action {
                text: "clear"
                onTriggered: root.pageStack.clear()
            },
            Kirigami.Action {
                text: "replace"
                onTriggered: root.pageStack.replace(secondPageComponent)
            }
        ]
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent
        Kirigami.Page {
            title: "First Page"
            Rectangle {
                anchors.fill: parent
                QQC2.Label {
                    text: "First Page"
                }
            }
        }
    }

    Component {
        id: secondPageComponent
        Kirigami.Page {
            title: "Second Page"
            Rectangle {
                color: "red"
                anchors.fill: parent
                QQC2.Label {
                    text: "Second Page"
                }
            }
        }
    }
}
