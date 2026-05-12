/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWINDOWSYSTEM_P_H
#define KWINDOWSYSTEM_P_H

#include "netwm_def.h"

#include <QFuture>
#include <QStringList>
#include <QWidgetList> //For WId
#include <kwindowsystem_export.h>

class NETWinInfo;

class KWINDOWSYSTEM_EXPORT KWindowSystemPrivate : public NET
{
public:
    virtual ~KWindowSystemPrivate();
    virtual void activateWindow(QWindow *win, long time = 0) = 0;
    virtual bool showingDesktop() = 0;
    virtual void setShowingDesktop(bool showing) = 0;
};

class KWINDOWSYSTEM_EXPORT KWindowSystemPrivateV2 : public KWindowSystemPrivate
{
public:
#if KWINDOWSYSTEM_BUILD_DEPRECATED_SINCE(6, 19)
    virtual void requestToken(QWindow *win, uint32_t serial, const QString &app_id) = 0;
#endif
    virtual void setCurrentToken(const QString &token) = 0;
    virtual quint32 lastInputSerial(QWindow *window) = 0;
    virtual void setMainWindow(QWindow *window, const QString &handle) = 0;
    virtual void exportWindow(QWindow *window) = 0;
    virtual void unexportWindow(QWindow *window) = 0;
};

class KWINDOWSYSTEM_EXPORT KWindowSystemPrivateV3 : public KWindowSystemPrivateV2
{
public:
    virtual QFuture<QString> xdgActivationToken(QWindow *window, uint32_t serial, const QString &appId) = 0;
};

class KWINDOWSYSTEM_EXPORT KWindowSystemPrivateV4 : public KWindowSystemPrivateV3
{
public:
    virtual void setXdgToplevelTag(QWindow *window, const QString &tag) = 0;
    virtual void setXdgToplevelDescription(QWindow *window, const QString &description) = 0;
};

#endif
