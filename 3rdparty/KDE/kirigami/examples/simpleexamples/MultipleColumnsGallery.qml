/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page

    Layout.fillWidth: true
    implicitWidth: Kirigami.Units.gridUnit * (Math.floor(Math.random() * 35) + 8)

    title: "Multiple Columns"

    actions: [
        Kirigami.Action {
            text:"Action for buttons"
            icon.name: "bookmarks"
            onTriggered: print("Action 1 clicked")
        },
        Kirigami.Action {
            text:"Action 2"
            icon.name: "folder"
            enabled: false
        }
    ]

    ColumnLayout {
        width: page.width
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "This page is used to test multiple columns: you can push and pop an arbitrary number of pages, each new page will have a random implicit width between 8 and 35 grid units.\nIf you enlarge the window enough, you can test how the application behaves with multiple columns."
        }
        Item {
            Layout.minimumWidth: Kirigami.Units.gridUnit *2
            Layout.minimumHeight: Layout.minimumWidth
        }
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            text: "Page implicitWidth: " + page.implicitWidth
        }
        QQC2.Button {
            text: "Push Another Page"
            Layout.alignment: Qt.AlignHCenter
            onClicked: Kirigami.PageStack.push(Qt.resolvedUrl("MultipleColumnsGallery.qml"));
        }
        QQC2.Button {
            text: "Pop A Page"
            Layout.alignment: Qt.AlignHCenter
            onClicked: Kirigami.PageStack.pop();
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            QQC2.TextField {
                id: edit
                text: page.title
            }
            QQC2.Button {
                text: "Rename Page"
                onClicked: page.title = edit.text;
            }
        }
        Kirigami.SearchField {
            id: searchField
            Layout.alignment: Qt.AlignHCenter
            onAccepted: console.log("Search text is " + text);
        }
        Kirigami.PasswordField {
            id: passwordField
            Layout.alignment: Qt.AlignHCenter
            onAccepted: console.log("Password")
        }
    }
}
