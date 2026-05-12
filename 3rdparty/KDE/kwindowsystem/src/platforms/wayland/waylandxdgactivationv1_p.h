/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAYLANDXDGACTIVATIONV1_P_H
#define WAYLANDXDGACTIVATIONV1_P_H

#include "qwayland-xdg-activation-v1.h"

#include <QFuture>
#include <QObject>
#include <QPromise>
#include <QtWaylandClient/QWaylandClientExtension>

class QWaylandSurface;

class WaylandXdgActivationTokenV1 : public QObject, public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT

public:
    WaylandXdgActivationTokenV1()
    {
        m_promise.start();
    }

    ~WaylandXdgActivationTokenV1() override
    {
        destroy();
    }

    QFuture<QString> future() const
    {
        return m_promise.future();
    }

protected:
    void xdg_activation_token_v1_done(const QString &token) override
    {
        m_promise.addResult(token);
        m_promise.finish();

        Q_EMIT done(token);
        deleteLater();
    }

Q_SIGNALS:
    void done(const QString &token);

private:
    QPromise<QString> m_promise;
};

class WaylandXdgActivationV1 : public QWaylandClientExtensionTemplate<WaylandXdgActivationV1>, public QtWayland::xdg_activation_v1
{
public:
    ~WaylandXdgActivationV1() override;

    static WaylandXdgActivationV1 *self();

    WaylandXdgActivationTokenV1 *requestXdgActivationToken(wl_seat *seat, struct ::wl_surface *surface, uint32_t serial, const QString &app_id);

private:
    WaylandXdgActivationV1();
};

#endif
