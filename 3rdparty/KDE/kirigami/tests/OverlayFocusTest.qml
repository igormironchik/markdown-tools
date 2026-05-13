/*
 *  SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>
 *  SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

Rectangle {
    id: background

    implicitWidth: 600
    implicitHeight: 600
    color: Kirigami.Theme.backgroundColor

    Kirigami.FormLayout {
        id: layout
        anchors.centerIn: parent

        QQC2.Button {
            Layout.fillWidth: true
            text: "Open overlay sheet"
            onClicked: sheet.open()
        }
    }

    Kirigami.OverlaySheet {
        id: sheet
        parent: background

        header: QQC2.TextField {
            id: headerText
            focus: true
        }
        footer: QQC2.TextField {
            id: footerText
        }

        ListView {
            id: content
            model: 10

            delegate: QQC2.ItemDelegate {
                required property int modelData
                text: "Item " + modelData
                width: parent.width
            }
        }
    }
}
