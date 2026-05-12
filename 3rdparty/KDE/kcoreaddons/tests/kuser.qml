// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import org.kde.coreaddons as KCoreAddons

Column {
    KCoreAddons.KUser {
        id: kuser
    }

    Text {
        text: kuser.fullName
    }
    Text {
        text: kuser.loginName
    }
    Text {
        text: kuser.os
    }
    Text {
        text: kuser.host
    }

    Image {
        id: faceIcon
        source: kuser.faceIconUrl
    }
}
