// SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.coreaddons

ColumnLayout {
    TextField {
        id: input
        text: "www.kde.org *bold* +491234567890"
    }
    Text {
        text: KTextToHTML.convertToHtml(input.text, KTextToHTMLOptions.PreserveSpaces)
        textFormat: Text.RichText
        onLinkActivated: (link) => { Qt.openUrlExternally(link); }
    }
    Text {
        text: KTextToHTML.convertToHtml(input.text,  KTextToHTMLOptions.HighlightText)
        textFormat: Text.RichText
        onLinkActivated: (link) => { Qt.openUrlExternally(link); }
    }
    Text {
        text: KTextToHTML.convertToHtml(input.text,  KTextToHTMLOptions.ConvertPhoneNumbers)
        textFormat: Text.RichText
        onLinkActivated: (link) => { Qt.openUrlExternally(link); }
    }
}
