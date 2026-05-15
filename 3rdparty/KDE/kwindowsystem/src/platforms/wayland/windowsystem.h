/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINDOWSYSTEM_H
#define WINDOWSYSTEM_H

#include "kwindowsystem_p.h"

#include <QObject>

class WindowManagement;

class WindowSystem : public QObject, public KWindowSystemPrivateV4
{
    Q_OBJECT
public:
    WindowSystem();
    ~WindowSystem() override;
    void activateWindow(QWindow *win, long time) override;
#if KWINDOWSYSTEM_BUILD_DEPRECATED_SINCE(6, 19)
    void requestToken(QWindow *win, uint32_t serial, const QString &app_id) override;
#endif
    quint32 lastInputSerial(QWindow *window) override;
    void setCurrentToken(const QString &token) override;
    bool showingDesktop() override;
    void setShowingDesktop(bool showing) override;
    void exportWindow(QWindow *window) override;
    void unexportWindow(QWindow *window) override;
    void setMainWindow(QWindow *window, const QString &handle) override;
    QFuture<QString> xdgActivationToken(QWindow *window, uint32_t serial, const QString &appId) override;
    void setXdgToplevelTag(QWindow *window, const QString &tag) override;
    void setXdgToplevelDescription(QWindow *window, const QString &description) override;

private:
    static void doSetMainWindow(QWindow *window, const QString &handle);
    static QString consumeCurrentActivationToken();
    WindowManagement *m_windowManagement;
};

#endif
