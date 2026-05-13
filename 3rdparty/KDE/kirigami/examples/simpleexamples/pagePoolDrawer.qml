/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    Kirigami.PagePool {
        id: mainPagePool
    }

    globalDrawer: Kirigami.GlobalDrawer {
        modal: !root.wideScreen
        width: Kirigami.Units.gridUnit * 10

        actions: [
            Kirigami.PagePoolAction {
                text: i18n("Page1")
                icon.name: "speedometer"
                pagePool: mainPagePool
                page: "SimplePage.qml"
            },
            Kirigami.PagePoolAction {
                text: i18n("Page2")
                icon.name: "window-duplicate"
                pagePool: mainPagePool
                page: "MultipleColumnsGallery.qml"
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPagePool.loadPage("SimplePage.qml")

}
