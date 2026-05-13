/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQml
import QtQuick.Controls as QQC2

QQC2.ApplicationWindow {
    id: root
    width: 200 * Application.styleHints.wheelScrollLines + scrollView.leftPadding + scrollView.rightPadding
    height: 200 * Application.styleHints.wheelScrollLines + scrollView.topPadding + scrollView.bottomPadding
    visible: true
    ScrollView {
        id: scrollView
        focus: true
        anchors.fill: parent
        Grid {
            columns: Math.sqrt(visibleChildren.length)
            Repeater {
                model: 500
                delegate: Rectangle {
                    id: rectangle1
                    required property int index
                    implicitWidth: 20 * Application.styleHints.wheelScrollLines
                    implicitHeight: 20 * Application.styleHints.wheelScrollLines
                    gradient: Gradient {
                        orientation: rectangle1.index % 2 ? Gradient.Vertical : Gradient.Horizontal
                        GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                        GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    }
                }
            }
            QQC2.Button {
                id: enableSliderButton
                width: 20 * Application.styleHints.wheelScrollLines
                height: 20 * Application.styleHints.wheelScrollLines
                contentItem: QQC2.Label {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Enable Slider"
                    wrapMode: Text.Wrap
                }
                checked: true
                checkable: true
            }
            QQC2.Slider {
                id: slider
                enabled: enableSliderButton.checked
                width: 20 * Application.styleHints.wheelScrollLines
                height: 20 * Application.styleHints.wheelScrollLines
                wheelEnabled: true
            }
            Repeater {
                model: 500
                delegate: Rectangle {
                    id: rectangle2
                    required property int index
                    implicitWidth: 20 * Application.styleHints.wheelScrollLines
                    implicitHeight: 20 * Application.styleHints.wheelScrollLines
                    gradient: Gradient {
                        orientation: rectangle2.index % 2 ? Gradient.Vertical : Gradient.Horizontal
                        GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                        GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    }
                }
            }
        }
    }
}
