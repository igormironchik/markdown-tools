/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import org.kde.kirigami as Kirigami
import QtTest

TestCase {
    id: root
    readonly property Kirigami.WheelHandler wheelHandler: (loader.item instanceof ScrollView) ? (loader.item as ScrollViewComponent).wheelHandler : (loader.item as FlickableComponent).wheelHandler
    readonly property Flickable flickable: (loader.item instanceof ScrollView) ? (loader.item as ScrollView).contentItem as Flickable : loader.item as Flickable
    readonly property Item inputItem: flickable
    readonly property real hstep: wheelHandler.horizontalStepSize
    readonly property real vstep: wheelHandler.verticalStepSize
    readonly property real pageWidth: flickable.width - flickable.leftMargin - flickable.rightMargin
    readonly property real pageHeight: flickable.height - flickable.topMargin - flickable.bottomMargin
    readonly property real contentWidth: flickable.contentWidth
    readonly property real contentHeight: flickable.contentHeight
    readonly property real leftMargin: (loader.item instanceof ScrollView) ? (loader.item as ScrollView).leftPadding : (loader.item as Flickable).leftMargin

    name: "WheelHandler scrolling"
    visible: true
    when: windowShown
    width: loader.implicitWidth
    height: loader.implicitHeight

    function wheelScrolling(angleDelta = 120) {
        let x = flickable.contentX
        let y = flickable.contentY
        const angleDeltaFactor = angleDelta / 120
        mouseWheel(inputItem, leftMargin, 0, -angleDelta, -angleDelta, Qt.NoButton)
        tryCompare(flickable, "contentX", Math.round(x + hstep * angleDeltaFactor), Kirigami.Units.longDuration * 2, "+xTick")
        x = flickable.contentX
        tryCompare(flickable, "contentY", Math.round(y + vstep * angleDeltaFactor), Kirigami.Units.longDuration * 2, "+yTick")
        y = flickable.contentY

        mouseWheel(inputItem, leftMargin, 0, angleDelta, angleDelta, Qt.NoButton)
        tryCompare(flickable, "contentX", Math.round(x - hstep * angleDeltaFactor), Kirigami.Units.longDuration * 2, "-xTick")
        x = flickable.contentX
        tryCompare(flickable, "contentY", Math.round(y - vstep * angleDeltaFactor), Kirigami.Units.longDuration * 2, "-yTick")
        y = flickable.contentY

        if (Qt.platform.pluginName !== "xcb") {
            mouseWheel(inputItem, leftMargin, 0, 0, -angleDelta, Qt.NoButton, Qt.AltModifier)
            tryCompare(flickable, "contentX", Math.round(x + hstep * angleDeltaFactor), Kirigami.Units.longDuration * 2, "+h_yTick")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, Kirigami.Units.longDuration * 2, "no +yTick")

            mouseWheel(inputItem, leftMargin, 0, 0, angleDelta, Qt.NoButton, Qt.AltModifier)
            tryCompare(flickable, "contentX", Math.round(x - hstep * angleDeltaFactor), Kirigami.Units.longDuration * 2, "-h_yTick")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, Kirigami.Units.longDuration * 2, "no -yTick")
        }

        mouseWheel(inputItem, leftMargin, 0, -angleDelta, -angleDelta, Qt.NoButton, wheelHandler.pageScrollModifiers)
        tryCompare(flickable, "contentX", Math.round(x + pageWidth * angleDeltaFactor), Kirigami.Units.longDuration * 2, "+xPage")
        x = flickable.contentX
        tryCompare(flickable, "contentY", Math.round(y + pageHeight * angleDeltaFactor), Kirigami.Units.longDuration * 2, "+yPage")
        y = flickable.contentY

        mouseWheel(inputItem, leftMargin, 0, angleDelta, angleDelta, Qt.NoButton, wheelHandler.pageScrollModifiers)
        tryCompare(flickable, "contentX", Math.round(x - pageWidth * angleDeltaFactor), Kirigami.Units.longDuration * 2, "-xPage")
        x = flickable.contentX
        tryCompare(flickable, "contentY", Math.round(y - pageHeight * angleDeltaFactor), Kirigami.Units.longDuration * 2, "-yPage")
        y = flickable.contentY

        if (Qt.platform.pluginName !== "xcb") {
            mouseWheel(inputItem, leftMargin, 0, 0, -angleDelta, Qt.NoButton,
                    Qt.AltModifier | wheelHandler.pageScrollModifiers)
            tryCompare(flickable, "contentX", Math.round(x + pageWidth * angleDeltaFactor), Kirigami.Units.longDuration * 2, "+h_yPage")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, Kirigami.Units.longDuration * 2, "no +yPage")

            mouseWheel(inputItem, leftMargin, 0, 0, angleDelta, Qt.NoButton,
                    Qt.AltModifier | wheelHandler.pageScrollModifiers)
            tryCompare(flickable, "contentX", Math.round(x - pageWidth * angleDeltaFactor), Kirigami.Units.longDuration * 2, "-h_yPage")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, Kirigami.Units.longDuration * 2, "no -yPage")
        }
    }

    function test_WheelScrolling() {
        // HID 1bcf:08a0 Mouse
        // Angle delta is 120, like most mice.
        loader.sourceComponent = flickableComponent
        wheelScrolling()
        loader.sourceComponent = scrollViewComponent
        wheelScrolling()
    }

    function test_HiResWheelScrolling() {
        // Logitech MX Master 3
        // Main wheel angle delta is at least 16, plus multiples of 8 when scrolling faster.
        loader.sourceComponent = flickableComponent
        wheelScrolling(16)
        loader.sourceComponent = scrollViewComponent
        wheelScrolling(16)
    }

    function test_TouchpadScrolling() {
        // UNIW0001:00 093A:0255 Touchpad
        // 2 finger scroll angle delta is at least 3, but larger increments are used when scrolling faster.
        loader.sourceComponent = flickableComponent
        wheelScrolling(3)
        loader.sourceComponent = scrollViewComponent
        wheelScrolling(3)
    }

    function keyboardScrolling() {
        const originalX = flickable.contentX
        const originalY = flickable.contentY
        let x = originalX
        let y = originalY
        keyClick(Qt.Key_Right)
        tryCompare(flickable, "contentX", x + hstep, Kirigami.Units.longDuration * 2, "Key_Right")
        x = flickable.contentX

        keyClick(Qt.Key_Left)
        tryCompare(flickable, "contentX", x - hstep, Kirigami.Units.longDuration * 2, "Key_Left")
        x = flickable.contentX

        keyClick(Qt.Key_Down)
        tryCompare(flickable, "contentY", y + vstep, Kirigami.Units.longDuration * 2, "Key_Down")
        y = flickable.contentY

        keyClick(Qt.Key_Up)
        tryCompare(flickable, "contentY", y - vstep, Kirigami.Units.longDuration * 2, "Key_Up")
        y = flickable.contentY

        keyClick(Qt.Key_PageDown)
        tryCompare(flickable, "contentY", y + pageHeight, Kirigami.Units.longDuration * 2, "Key_PageDown")
        y = flickable.contentY

        keyClick(Qt.Key_PageUp)
        tryCompare(flickable, "contentY", y - pageHeight, Kirigami.Units.longDuration * 2, "Key_PageUp")
        y = flickable.contentY

        keyClick(Qt.Key_End)
        tryCompare(flickable, "contentY", contentHeight - pageHeight, Kirigami.Units.longDuration * 2, "Key_End")
        y = flickable.contentY

        keyClick(Qt.Key_Home)
        tryCompare(flickable, "contentY", originalY, Kirigami.Units.longDuration * 2, "Key_Home")
        y = flickable.contentY

        keyClick(Qt.Key_PageDown, Qt.AltModifier)
        tryCompare(flickable, "contentX", x + pageWidth, Kirigami.Units.longDuration * 2, "h_Key_PageDown")
        x = flickable.contentX

        keyClick(Qt.Key_PageUp, Qt.AltModifier)
        tryCompare(flickable, "contentX", x - pageWidth, Kirigami.Units.longDuration * 2, "h_Key_PageUp")
        x = flickable.contentX

        keyClick(Qt.Key_End, Qt.AltModifier)
        tryCompare(flickable, "contentX", contentWidth - pageWidth, Kirigami.Units.longDuration * 2, "h_Key_End")
        x = flickable.contentX

        keyClick(Qt.Key_Home, Qt.AltModifier)
        tryCompare(flickable, "contentX", originalX, Kirigami.Units.longDuration * 2, "h_Key_Home")
    }

    function test_KeyboardScrolling() {
        loader.sourceComponent = flickableComponent
        keyboardScrolling()
        loader.sourceComponent = scrollViewComponent
        keyboardScrolling()
    }

    function test_StepSize() {
        loader.sourceComponent = flickableComponent
        // 101 is a value unlikely to be used by any user's combination of settings and hardware
        wheelHandler.verticalStepSize = 101
        wheelHandler.horizontalStepSize = 101
        wheelScrolling()
        keyboardScrolling()
        // reset to default
        wheelHandler.verticalStepSize = undefined
        wheelHandler.horizontalStepSize = undefined
        verify(wheelHandler.verticalStepSize == 20 * Application.styleHints.wheelScrollLines, "default verticalStepSize")
        verify(wheelHandler.horizontalStepSize == 20 * Application.styleHints.wheelScrollLines, "default horizontalStepSize")

        loader.sourceComponent = scrollViewComponent
        wheelHandler.verticalStepSize = 101
        wheelHandler.horizontalStepSize = 101
        wheelScrolling()
        keyboardScrolling()
        wheelHandler.verticalStepSize = undefined
        wheelHandler.horizontalStepSize = undefined
        verify(wheelHandler.verticalStepSize == 20 * Application.styleHints.wheelScrollLines, "default verticalStepSize")
        verify(wheelHandler.horizontalStepSize == 20 * Application.styleHints.wheelScrollLines, "default horizontalStepSize")
    }

    Loader {
        id: loader
        anchors.fill: parent
        focus: true
        sourceComponent: flickableComponent
    }

    component FlickableComponent: ScrollableFlickable {
        id: flickable
        readonly property alias wheelHandler: wheelHandler
        focus: true
        Kirigami.WheelHandler {
            id: wheelHandler
            target: flickable
            keyNavigationEnabled: true
        }
    }

    Component {
        id: flickableComponent

        FlickableComponent { }
    }

    component ScrollViewComponent: ScrollView {
        readonly property alias wheelHandler: wheelHandler
        focus: true
        implicitWidth: flickable.implicitWidth + leftPadding + rightPadding
        implicitHeight: flickable.implicitHeight + topPadding + bottomPadding
        contentItem: ContentFlickable {
            id: flickable
        }
        data: [
            Kirigami.WheelHandler {
                id: wheelHandler
                target: flickable
                keyNavigationEnabled: true
            }
        ]
    }

    Component {
        id: scrollViewComponent

        ScrollViewComponent { }
    }
}
