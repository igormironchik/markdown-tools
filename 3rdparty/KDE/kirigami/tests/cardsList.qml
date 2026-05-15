/*
 *  SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {

    Component {
        id: delegateComponent
        Kirigami.Card {
            id: card
            required property int index
            contentItem: Label { text: ourlist.prefix + card.index }
        }
    }

    pageStack.initialPage: Kirigami.ScrollablePage {

        Kirigami.CardsListView {
            id: ourlist
            property string prefix: "ciao "

            delegate: delegateComponent

            model: 100
        }
    }
}
