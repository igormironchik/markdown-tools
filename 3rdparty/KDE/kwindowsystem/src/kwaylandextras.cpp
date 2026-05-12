/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwaylandextras.h"

#include "kwindowsystem.h"
#include "kwindowsystem_p.h"

#include <QTimer>

KWaylandExtras::KWaylandExtras()
    : QObject()
{
}

KWaylandExtras::~KWaylandExtras() = default;

KWaylandExtras *KWaylandExtras::self()
{
    static KWaylandExtras instance;
    return &instance;
}

#if KWINDOWSYSTEM_BUILD_DEPRECATED_SINCE(6, 19)
void KWaylandExtras::requestXdgActivationToken(QWindow *window, uint32_t serial, const QString &app_id)
{
    auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func());
    if (!dv2) {
        // Ensure that xdgActivationTokenArrived is always emitted asynchronously
        QTimer::singleShot(0, [serial] {
            Q_EMIT KWaylandExtras::self()->xdgActivationTokenArrived(serial, {});
        });

        return;
    }
    dv2->requestToken(window, serial, app_id);
}
#endif

quint32 KWaylandExtras::lastInputSerial(QWindow *window)
{
    auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func());
    if (!dv2) {
        return 0;
    }
    return dv2->lastInputSerial(window);
}

void KWaylandExtras::exportWindow(QWindow *window)
{
    if (auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func())) {
        dv2->exportWindow(window);
    }
}

void KWaylandExtras::unexportWindow(QWindow *window)
{
    if (auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func())) {
        dv2->unexportWindow(window);
    }
}

QFuture<QString> KWaylandExtras::xdgActivationToken(QWindow *window, uint32_t serial, const QString &appId)
{
    if (auto dv3 = dynamic_cast<KWindowSystemPrivateV3 *>(KWindowSystem::d_func())) {
        return dv3->xdgActivationToken(window, serial, appId);
    } else {
#if KWINDOWSYSTEM_BUILD_DEPRECATED_SINCE(6, 19)
        requestXdgActivationToken(window, serial, appId);
        QPromise<QString> promise;
        auto future = promise.future();

        connect(
            self(),
            &KWaylandExtras::xdgActivationTokenArrived,
            self(),
            [p = std::move(promise)](int /*serial*/, const QString &token) mutable {
                p.addResult(token);
                p.finish();
            },
            Qt::SingleShotConnection);

        return future;
#else
        return QtFuture::makeReadyValueFuture(QString());
#endif
    }
}

QFuture<QString> KWaylandExtras::xdgActivationToken(QWindow *window, const QString &appId)
{
    return xdgActivationToken(window, lastInputSerial(window), appId);
}

void KWaylandExtras::setXdgToplevelTag(QWindow *window, const QString &tag)
{
    if (auto dv4 = dynamic_cast<KWindowSystemPrivateV4 *>(KWindowSystem::d_func())) {
        dv4->setXdgToplevelTag(window, tag);
    }
}

void KWaylandExtras::setXdgToplevelDescription(QWindow *window, const QString &description)
{
    if (auto dv4 = dynamic_cast<KWindowSystemPrivateV4 *>(KWindowSystem::d_func())) {
        dv4->setXdgToplevelDescription(window, description);
    }
}

#include "moc_kwaylandextras.cpp"
