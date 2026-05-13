/*
 *  SPDX-FileCopyrightText: 2021 Nate Graham <nate@kde.org>
 *  SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD

Kirigami.ApplicationWindow {
    GridLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit

        rows: 3
        rowSpacing: Kirigami.Units.gridUnit
        columns: 3
        columnSpacing: Kirigami.Units.gridUnit

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        // Icon + Label
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            Kirigami.Heading {
                text: "Icon + Label"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
            }
        }

        // Label + space reserved for icon
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            Kirigami.Heading {
                text: "Icon + Label + space reserved for icon"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: Kirigami.Units.iconSizes.smallMedium
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: Kirigami.Units.iconSizes.smallMedium
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: Kirigami.Units.iconSizes.smallMedium
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true
                text: "Boom!"
                icon.width: Kirigami.Units.iconSizes.smallMedium
            }
        }

        // Icon + Label + leading and trailing items
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            Kirigami.Heading {
                text: "Icon + Label + leading and trailing items"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            QQC2.ItemDelegate {
                id: plainDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: plainDelegate.text
                        icon: icon.fromControlsIcon(plainDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            QQC2.CheckDelegate {
                id: checkDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: checkDelegate.text
                        icon: icon.fromControlsIcon(checkDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            QQC2.RadioDelegate {
                id: radioDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: radioDelegate.text
                        icon: icon.fromControlsIcon(radioDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            QQC2.SwitchDelegate {
                id: switchDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: switchDelegate.text
                        icon: icon.fromControlsIcon(switchDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
        }

        // Icon + Label + subtitle
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            Kirigami.Heading {
                text: "Icon + Label + subtitle"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
            KD.CheckSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
            KD.RadioSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
            KD.SwitchSubtitleDelegate {
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"
            }
        }

        // Icon + Label + space reserved for subtitle
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            Kirigami.Heading {
                text: "Icon + Label + space reserved for subtitle"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                id: subtitleDelegate1
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: subtitleDelegate1.text
                    icon: icon.fromControlsIcon(subtitleDelegate1.icon)
                    reserveSpaceForSubtitle: true
                }
            }
            KD.CheckSubtitleDelegate {
                id: checkSubtitleDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: checkSubtitleDelegate.text
                    icon: icon.fromControlsIcon(checkSubtitleDelegate.icon)
                    reserveSpaceForSubtitle: true
                }
            }
            KD.RadioSubtitleDelegate {
                id: radioSubtitleDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: radioSubtitleDelegate.text
                    icon: icon.fromControlsIcon(radioSubtitleDelegate.icon)
                    reserveSpaceForSubtitle: true
                }
            }
            KD.SwitchSubtitleDelegate {
                id: switchSubtitleDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"

                contentItem: KD.IconTitleSubtitle {
                    title: switchSubtitleDelegate.text
                    icon: icon.fromControlsIcon(switchSubtitleDelegate.icon)
                    reserveSpaceForSubtitle: true
                }
            }
        }

        // Icon + Label + subtitle + leading and trailing items
        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: 1

            Kirigami.Heading {
                text: "Icon + Label + subtitle + leading/trailing"
                level: 3
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            KD.SubtitleDelegate {
                id: subtitleDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleDelegate.text
                        subtitle: subtitleDelegate.subtitle
                        selected: subtitleDelegate.highlighted || subtitleDelegate.down
                        icon: icon.fromControlsIcon(subtitleDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            KD.CheckSubtitleDelegate {
                id: subtitleCheckDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleCheckDelegate.text
                        subtitle: subtitleCheckDelegate.subtitle
                        selected: subtitleCheckDelegate.highlighted || subtitleCheckDelegate.down
                        icon: icon.fromControlsIcon(subtitleCheckDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            KD.RadioSubtitleDelegate {
                id: subtitleRadioDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleRadioDelegate.text
                        subtitle: subtitleRadioDelegate.subtitle
                        selected: subtitleRadioDelegate.highlighted || subtitleRadioDelegate.down
                        icon: icon.fromControlsIcon(subtitleRadioDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
            KD.SwitchSubtitleDelegate {
                id: subtitleSwitchDelegate
                Layout.fillWidth: true

                icon.name: "edit-bomb"
                text: "Boom!"
                subtitle: "smaller boom"

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Rectangle {
                        radius: height
                        Layout.preferredWidth: Kirigami.Units.largeSpacing
                        Layout.preferredHeight: Kirigami.Units.largeSpacing
                        color: Kirigami.Theme.neutralTextColor
                    }

                    KD.IconTitleSubtitle {
                        Layout.fillWidth: true
                        title: subtitleSwitchDelegate.text
                        subtitle: subtitleSwitchDelegate.subtitle
                        selected: subtitleSwitchDelegate.highlighted || subtitleSwitchDelegate.down
                        icon: icon.fromControlsIcon(subtitleSwitchDelegate.icon)
                    }

                    QQC2.Button {
                        icon.name: "edit-delete"
                        text: "Defuse the bomb!"
                    }
                }
            }
        }
    }
}

