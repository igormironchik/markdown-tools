/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.guiaddons

ColumnLayout {
    Label { text: "Mixing" }
    RowLayout {
        TextField {
            id: color1
            text: "#ff0000"
        }
        TextField {
            id: color2
            text: "#00ff00"
        }
    }
    Text {
        text: "Hue: " + ColorUtils.hue(color1.text) + " Chroma: " + ColorUtils.chroma(color1.text) + " Luma: " + ColorUtils.luma(color1.text)
    }
    Text {
        text: "Contrast ratio: " + ColorUtils.contrastRatio(color1.text, color2.text)
    }
    RowLayout {
        Slider {
            id: lightSlider1
        }
        Slider {
            id: lightSlider2
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ColorUtils.lighten(color1.text, lightSlider1.value, lightSlider2.value)
        }
    }
    RowLayout {
        Slider {
            id: darkSlider1
        }
        Slider {
            id: darkSlider2
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ColorUtils.darken(color1.text, darkSlider1.value, darkSlider2.value)
        }
    }
    RowLayout {
        Slider {
            id: shadeSlider1
        }
        Slider {
            id: shadeSlider2
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ColorUtils.shade(color1.text, shadeSlider1.value, shadeSlider2.value)
        }
    }
    RowLayout {
        Slider {
            id: tintSlider
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ColorUtils.tint(color1.text, color2.text, tintSlider.value)
        }
    }
    RowLayout {
        Slider {
            id: mixSlider
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ColorUtils.mix(color1.text, color2.text, mixSlider.value)
        }
    }
}
