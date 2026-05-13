/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as QtDialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import "components" as Components
import "ImageProviders.mjs" as ImageProviders

QQC2.Page {
    id: root

    readonly property alias image: image

    readonly property Kirigami.ImageColors imagePalette: Kirigami.ImageColors {
        id: imagePalette
        source: image
    }

    readonly property /*ImageProviders.ImageData*/ var imageData: __imageData

    readonly property Item overlay: overlay

    property /*XMLHttpRequest*/var __xhr: null
    property /*ImageProviders.ImageData*/ var __imageData: null

    function __abort(): void {
        if (__xhr !== null) {
            __xhr.abort();
            __xhr = null;
        }
    }

    function randomizeSource(): void {
        __abort();
        const provider = new ImageProviders.UnsplashProvider();
        __xhr = provider.getRandom(imageData => {
            __xhr = null;
            __imageData = imageData;
        });
    }

    function openChooserDialog(): void {
        fileDialog.open();
    }

    function setImageSource(fileUrl: url): void {
        __abort();
        const provider = new ImageProviders.FileProvider(fileUrl);
        __imageData = provider.imageData();
    }

    Component.onCompleted: {
        randomizeSource();
    }

    header: QQC2.ToolBar {
        contentItem: Kirigami.ActionToolBar {
            alignment: Qt.AlignHCenter
            actions: [
                Kirigami.Action {
                    icon.name: "shuffle-symbolic"
                    text: "Random Image"
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    onTriggered: root.randomizeSource();
                },
                Kirigami.Action {
                    icon.name: "insert-image-symbolic"
                    text: "Open Image…"
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    onTriggered: root.openChooserDialog();
                }
            ]
        }
    }

    QtDialogs.FileDialog {
        id: fileDialog

        title: "Open Image"
        nameFilters: ["Media Files (*.jpg *.png *.svg)"]
        fileMode: QtDialogs.FileDialog.OpenFile
        options: QtDialogs.FileDialog.ReadOnly

        onAccepted: {
            root.setImageSource(selectedFile);
        }
    }

    Image {
        id: image

        anchors.fill: parent
        source: root.__imageData?.image_url ?? "";
        fillMode: Image.PreserveAspectFit

        onStatusChanged: {
            imagePalette.update();
        }
    }

    QQC2.Pane {
        id: overlay

        anchors.centerIn: parent

        width: Kirigami.Units.gridUnit * 15

        Kirigami.Theme.backgroundColor: imagePalette.background
        Kirigami.Theme.textColor: imagePalette.foreground
        Kirigami.Theme.highlightColor: imagePalette.highlight

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            border.width: 1
            border.color: Kirigami.Theme.textColor
            radius: Kirigami.Units.cornerRadius
            opacity: 0.8
        }

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            QQC2.Label {
                text: "Highlight Color:\nLorem Ipsum dolor sit amet"
                color: Kirigami.Theme.highlightColor
                wrapMode: Text.Wrap
            }

            Components.ColorWell {
                Layout.fillWidth: true
                color: Kirigami.Theme.highlightColor
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: "Text Color:\nLorem Ipsum dolor sit amet"
                color: Kirigami.Theme.textColor
                wrapMode: Text.Wrap
            }

            Components.ColorWell {
                Layout.fillWidth: true
                color: Kirigami.Theme.textColor
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: "Controls with inherited theme"
                color: Kirigami.Theme.textColor
                wrapMode: Text.Wrap
            }


            RowLayout {
                spacing: Kirigami.Units.largeSpacing

                QQC2.TextField {
                    Kirigami.Theme.inherit: true
                    Layout.fillWidth: true
                    text: "text"
                }

                QQC2.Button {
                    Kirigami.Theme.inherit: true
                    text: "Ok"
                }
            }
        }
    }
}
