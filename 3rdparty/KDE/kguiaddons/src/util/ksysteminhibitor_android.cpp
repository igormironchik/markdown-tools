// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-FileCopyrightText: 2025 Volker Krause <vkrause@kde.org>

#include "ksysteminhibitor.h"

#include <QCoreApplication>
#include <QJniObject>

class KSystemInhibitorPrivate
{
public:
    KSystemInhibitor::Types m_types;
};

KSystemInhibitor::KSystemInhibitor([[maybe_unused]] const QString &reason, Types types, [[maybe_unused]] QWindow *window, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<KSystemInhibitorPrivate>())
{
    d->m_types = types;
    if (d->m_types & KSystemInhibitor::Type::Idle) {
        QJniObject::callStaticMethod<void>("org.kde.guiaddons.KSystemInhibitor",
                                           "setLockInhibitionOn",
                                           "(Landroid/app/Activity;)V",
                                           QNativeInterface::QAndroidApplication::context());
    }
}

KSystemInhibitor::~KSystemInhibitor()
{
    if (d->m_types & KSystemInhibitor::Type::Idle) {
        QJniObject::callStaticMethod<void>("org.kde.guiaddons.KSystemInhibitor",
                                           "setLockInhibitionOff",
                                           "(Landroid/app/Activity;)V",
                                           QNativeInterface::QAndroidApplication::context());
    }
}

#include "moc_ksysteminhibitor.cpp"
