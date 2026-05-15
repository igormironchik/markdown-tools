/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import "components" as Components

Kirigami.ScrollablePage {
    id: root

    readonly property list<string> __icons: [
        "desktop",
        "firefox",
        "vlc",
        "blender",
        "applications-games",
        "blinken",
        "adjustlevels",
        "adjustrgb",
        "cuttlefish",
        "folder-games",
        "applications-network",
        "multimedia-player",
        "applications-utilities",
        "accessories-dictionary",
        "calligraflow",
        "calligrakrita",
        "view-left-close",
        "calligraauthor",
    ]

    property int __currentIconIndex: 0

    function currentIcon(): string {
        return __icons[__currentIconIndex];
    }

    function previousIcon(): void {
        __currentIconIndex = (__currentIconIndex + __icons.length - 1) % __icons.length;
    }

    function nextIcon(): void {
        __currentIconIndex = (__currentIconIndex + 1) % __icons.length;
    }

    Kirigami.ImageColors {
        id: iconPalette
        source: icon.source
    }

    implicitWidth: Kirigami.Units.gridUnit * 20

    header: QQC2.ToolBar {
        contentItem: Kirigami.ActionToolBar {
            alignment: Qt.AlignHCenter
            actions: [
                Kirigami.Action {
                    text: "Previous"
                    icon.name: "go-previous-symbolic"
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    onTriggered: root.previousIcon();
                },
                Kirigami.Action {
                    text: "Next"
                    icon.name: "go-next-symbolic"
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    onTriggered: root.nextIcon();
                }
            ]
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.gridUnit

        Components.Section {
            title: "Icon Name"

            Kirigami.SelectableLabel {
                text: root.currentIcon()
                horizontalAlignment: TextEdit.AlignHCenter
                Layout.fillWidth: true
            }
        }

        Components.Section {
            title: "Icon"

            QQC2.Label {
                text: "Background"
            }

            QQC2.ButtonGroup {
                id: backgroundGroup
                Component.onCompleted: {
                    checkedButton = transparentRadioButton;
                }
            }

            QQC2.RadioButton {
                id: transparentRadioButton
                text: "Transparent"
                Layout.fillWidth: true
                QQC2.ButtonGroup.group: backgroundGroup
            }
            QQC2.RadioButton {
                id: kirigamiThemeRadioButton
                text: "Kirigami.Theme.backgroundColor"
                Layout.fillWidth: true
                QQC2.ButtonGroup.group: backgroundGroup
            }
            QQC2.RadioButton {
                id: contrastRadioButton
                text: "Contrast Color"
                Layout.fillWidth: true
                QQC2.ButtonGroup.group: backgroundGroup
            }

            Components.ColorWell {
                Layout.alignment: Qt.AlignHCenter
                Layout.minimumWidth: 100
                Layout.preferredWidth: 200
                Layout.preferredHeight: 200
                Layout.maximumWidth: 200
                Layout.fillWidth: true

                color: {
                    switch (backgroundGroup.checkedButton) {
                    case transparentRadioButton:
                    default:
                        return "transparent";
                    case kirigamiThemeRadioButton:
                        return Kirigami.Theme.backgroundColor;
                    case contrastRadioButton:
                        return iconPalette.dominantContrast;
                    }
                }
                showLabel: false

                Kirigami.Icon {
                    id: icon
                    anchors.centerIn: parent
                    width: Math.min(128, parent.width - Kirigami.Units.largeSpacing)
                    height: width
                    source: root.currentIcon()
                }
            }
        }

        Components.Section {
            title: "Average Color"

            Components.ColorWell {
                Layout.fillWidth: true
                color: iconPalette.average
            }
        }

        Components.Section {
            title: "Icon Palette"

            Components.PaletteTable {
                swatches: iconPalette.palette
            }
        }
    }
}
