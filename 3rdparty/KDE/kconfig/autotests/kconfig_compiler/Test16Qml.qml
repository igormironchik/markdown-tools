/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: MIT
*/

import QtQuick
import org.kde.kconfig.test16

Item {
    property url dir: TestCfg.picturesDir
    property double brightness: TestCfg.brightness
}
