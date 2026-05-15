/*
 *  SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    property int defaultColumnWidth: Kirigami.Units.gridUnit * 13
    property int columnWidth: defaultColumnWidth

    pageStack.defaultColumnWidth: columnWidth
    pageStack.initialPage: [firstPageComponent, secondPageComponent]

    MouseArea {
        id: dragHandle

        visible: root.pageStack.wideMode

        anchors.top: parent.top
        anchors.bottom: parent.bottom

        x: root.columnWidth - (width / 2)
        width: 2

        property int dragRange: (Kirigami.Units.gridUnit * 5)
        property int _lastX: -1

        cursorShape: Qt.SplitHCursor

        onPressed: mouse => {
            _lastX = mouse.x;
        }

        onPositionChanged: mouse => {
            if (mouse.x > _lastX) {
                root.columnWidth = Math.min((root.defaultColumnWidth + dragRange),
                    root.columnWidth + (mouse.x - _lastX));
            } else if (mouse.x < _lastX) {
                root.columnWidth = Math.max((root.defaultColumnWidth - dragRange),
                    root.columnWidth - (_lastX - mouse.x));
            }
        }

        Rectangle {
            anchors.fill: parent

            color: "blue"
        }
    }

    Component {
        id: firstPageComponent

        Kirigami.Page {
            id: firstPage

            background: Rectangle { color: "red" }
        }
    }

    Component {
        id: secondPageComponent

        Kirigami.Page {
            id: secondPage

            background: Rectangle { color: "green" }
        }
    }
}
