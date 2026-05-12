// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

import QtQuick
import org.kde.coreaddons as KCoreAddons

ListView {
    model: ListModel {
        dynamicRoles: true
        Component.onCompleted: {
            for (let i in KCoreAddons.KOSRelease) {
                this.append({name: i, value: KCoreAddons.KOSRelease[i]})
            }
        }
    }
    delegate: Text {
        required property string name
        required property var value
        text: "%1: %2".arg(name).arg(value)
    }
}
