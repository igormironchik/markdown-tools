/*
    SPDX-FileCopyrightText: 2025 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ksystemclockskewnotifierengine_dbus.h"

#include <QDBusConnection>

std::shared_ptr<KDBusSystemClockSkewNotifierEngine> KDBusSystemClockSkewNotifierEngine::create()
{
    return std::make_shared<KDBusSystemClockSkewNotifierEngine>();
}

KDBusSystemClockSkewNotifierEngine::KDBusSystemClockSkewNotifierEngine()
{
    QDBusConnection::sessionBus().connect(QString(),
                                          QStringLiteral("/org/kde/kcmshell_clock"),
                                          QStringLiteral("org.kde.kcmshell_clock"),
                                          QStringLiteral("clockUpdated"),
                                          this,
                                          SLOT(skewed()));

    QDBusConnection::sessionBus().connect(QStringLiteral("org.kde.Solid.PowerManagement"),
                                          QStringLiteral("/org/kde/Solid/PowerManagement/Actions/SuspendSession"),
                                          QStringLiteral("org.kde.Solid.PowerManagement.Actions.SuspendSession"),
                                          QStringLiteral("resumingFromSuspend"),
                                          this,
                                          SLOT(skewed()));
}

#include "moc_ksystemclockskewnotifierengine_dbus.cpp"
