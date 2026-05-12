/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ksystemclockskewnotifierengine_p.h"

#include "config-util.h"

#if defined(Q_OS_LINUX)
#include "ksystemclockskewnotifierengine_linux.h"
#elif HAVE_QTDBUS
#include "ksystemclockskewnotifierengine_dbus.h"
#endif

std::shared_ptr<KSystemClockSkewNotifierEngine> KSystemClockSkewNotifierEngine::globalInstance()
{
    static std::weak_ptr<KSystemClockSkewNotifierEngine> singleton;
    if (auto instance = singleton.lock()) {
        return instance;
    }

    std::shared_ptr<KSystemClockSkewNotifierEngine> instance;
#if defined(Q_OS_LINUX)
    instance = KLinuxSystemClockSkewNotifierEngine::create();
#elif HAVE_QTDBUS
    instance = KDBusSystemClockSkewNotifierEngine::create();
#endif

    singleton = instance;
    return instance;
}

KSystemClockSkewNotifierEngine::KSystemClockSkewNotifierEngine(QObject *parent)
    : QObject(parent)
{
}

#include "moc_ksystemclockskewnotifierengine_p.cpp"
