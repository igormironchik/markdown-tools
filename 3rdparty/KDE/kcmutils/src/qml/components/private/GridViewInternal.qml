/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

GridView {
    id: view

    property int implicitCellWidth: Kirigami.Units.gridUnit * 10
    property int implicitCellHeight: Math.round(implicitCellWidth / 1.6) + Kirigami.Units.gridUnit*3

    /*
     * Allow to highlight the selected item with Kirigami.Theme.neutralTextColor
     */
    property bool neutralHighlight: false

    onCurrentIndexChanged: positionViewAtIndex(currentIndex, GridView.Contain);

    QtObject {
        id: internal
        readonly property int availableWidth: scroll.width - internal.scrollBarSpace - 4
        readonly property int scrollBarSpace: scroll.QQC2.ScrollBar.vertical.width
    }
    anchors {
        fill: parent
        margins: 2
        leftMargin: 2 + (scroll.QQC2.ScrollBar.vertical.visible ? 0 : Math.round(internal.scrollBarSpace / 2))
    }
    clip: true
    activeFocusOnTab: true

    Keys.onTabPressed: event => nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
    Keys.onBacktabPressed: event => nextItemInFocusChain(false).forceActiveFocus(Qt.TabFocusReason)

    cellWidth: {
        const intendedWidth = implicitCellWidth + Kirigami.Units.gridUnit;
        const columnsWithIntendedWidth = Math.floor(internal.availableWidth / intendedWidth);
        return Math.floor(internal.availableWidth / Math.max(columnsWithIntendedWidth, 1));
    }
    cellHeight: implicitCellHeight

    keyNavigationEnabled: true
    keyNavigationWraps: true
    highlightMoveDuration: 0

    Loader {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.gridUnit * 8)
        active: parent.count === 0 && !startupTimer.running
        opacity: active && status === Loader.Ready ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        sourceComponent: Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            icon.name: "edit-none"
            text: i18nd("kcmutils6", "No items found")
        }
    }

    // The view can take a bit of time to initialize itself when the KCM first
    // loads, during which time count is 0, which would cause the placeholder
    // message to appear for a moment and then disappear. To prevent this, let's
    // suppress it appearing for a moment after the KCM loads.
    Timer {
        id: startupTimer
        interval: Kirigami.Units.humanMoment
        running: false
    }
    Component.onCompleted: {
        startupTimer.start()
    }
}
