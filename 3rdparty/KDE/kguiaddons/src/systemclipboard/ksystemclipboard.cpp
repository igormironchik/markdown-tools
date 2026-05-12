/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ksystemclipboard.h"
#include "kguiaddons_debug.h"

#include "qtclipboard_p.h"
#include "waylandclipboard_p.h"
#include "wlrwaylandclipboard_p.h"

#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>

KSystemClipboard *KSystemClipboard::instance()
{
    if (!qGuiApp || qGuiApp->closingDown()) {
        return nullptr;
    }
    static KSystemClipboard *systemClipboard = nullptr;

#ifdef WITH_WAYLAND
    static bool s_waylandChecked = false;
    if (!systemClipboard && qGuiApp->platformName().startsWith(QLatin1String("wayland")) && !s_waylandChecked) {
        s_waylandChecked = true;

        if (auto waylandClipboard = WaylandClipboard::create(qApp)) {
            systemClipboard = waylandClipboard;
        } else if (auto waylandClipboard = WlrWaylandClipboard::create(qApp)) {
            systemClipboard = waylandClipboard;
        } else {
            qCWarning(KGUIADDONS_LOG) << "Could not init WaylandClipboard, falling back to QtClipboard.";
        }
    }
#endif

    if (!systemClipboard) {
        systemClipboard = new QtClipboard(qApp);
    }

    return systemClipboard;
}

QString KSystemClipboard::text(QClipboard::Mode mode)
{
    const QMimeData *data = mimeData(mode);
    if (data) {
        return data->text();
    }
    return QString();
}

KSystemClipboard::KSystemClipboard(QObject *parent)
    : QObject(parent)
{
}

#include "moc_ksystemclipboard.cpp"
