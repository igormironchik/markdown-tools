/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.ApplicationWindow {
    id: root

    width: 800
    height: 600
    minimumWidth: 600
    visible: true

    QQC2.SplitView {
        id: splitView
        anchors.fill: parent

        IconPage {
            QQC2.SplitView.fillWidth: false
            QQC2.SplitView.preferredWidth: Kirigami.Units.gridUnit * 12
            QQC2.SplitView.minimumWidth: Kirigami.Units.gridUnit * 10
            clip: true
        }

        ImagePage {
            id: imagePage

            QQC2.SplitView.fillWidth: true
            QQC2.SplitView.minimumWidth: Kirigami.Units.gridUnit * 10
            clip: true
        }

        ImageStatsPage {
            QQC2.SplitView.fillWidth: false
            QQC2.SplitView.preferredWidth: Kirigami.Units.gridUnit * 12
            QQC2.SplitView.minimumWidth: Kirigami.Units.gridUnit * 10
            clip: true

            image: imagePage.image
            imagePalette: imagePage.imagePalette
            imageData: imagePage.imageData
            overlay: imagePage.overlay
        }
    }
}
