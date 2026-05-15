/*
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: main

    pageStack.initialPage: Kirigami.ScrollablePage {
        ListView {
            model: 25
            delegate: Kirigami.SwipeListItem {
                supportsMouseEvents: false
                actions: [
                    Kirigami.Action {
                        icon.name: "go-up"
                    }
                ]
                contentItem: Label {
                    elide: Text.ElideRight
                    text: "big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana big banana"
                }
            }
        }
    }
}
