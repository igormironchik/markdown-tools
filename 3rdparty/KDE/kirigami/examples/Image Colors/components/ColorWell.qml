/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

RowLayout {
    id: root

    required property color color

    property bool showLabel: true

    default property alias __contentData: contentWrapper.data

    Layout.fillWidth: false
    Layout.fillHeight: false

    Rectangle {
        id: borderGraphics

        Layout.alignment: Qt.AlignVCenter | (root.showLabel ? Qt.AlignLeft : Qt.AlignHCenter)
        Layout.fillWidth: !root.showLabel
        Layout.fillHeight: !root.showLabel

        implicitWidth: 32
        implicitHeight: 32

        border.width: 1
        border.color: Kirigami.Theme.textColor
        color: "transparent"
        radius: Kirigami.Units.cornerRadius + border.width

        Rectangle {
            id: contentWrapper
            z: -1
            anchors.fill: parent
            anchors.margins: borderGraphics.border.width
            border.width: 2
            border.color: Kirigami.Theme.backgroundColor
            color: root.color
            radius: Kirigami.Units.cornerRadius
        }

        Image {
            z: -2
            anchors.fill: contentWrapper
            anchors.margins: contentWrapper.border.width
            source: Qt.resolvedUrl("checkerboard.svg")
            fillMode: Image.Tile
            visible: root.color.a < 1.0
        }
    }

    Kirigami.SelectableLabel {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: root.showLabel
        verticalAlignment: TextEdit.AlignVCenter
        text: root.color.toString()
    }
}

