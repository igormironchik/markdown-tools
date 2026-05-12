/*
    SPDX-FileCopyrightText: 2025 Julius Künzel <julius.kuenzel@kde.org>

    SPDX-License-Identifier: MIT
*/

import QtQuick
import org.kde.kconfig.test17

Item {
    property url dir: Test17.picturesDir
    property double brightness: Test17.brightness
    property double brightnessDefault: Test17.defaultBrightnessValue
}
