/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KINHIBITIONJOBTRACKER_P_H
#define KINHIBITIONJOBTRACKER_P_H

#include <config-kjobwidgets.h>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#elif HAVE_QTDBUS
#include <QDBusObjectPath>
#endif

#include <QHash>
#include <QObject>
#include <QString>

class QTimer;

class KJob;
class KInhibitionJobTracker;

class Inhibition : public QObject
{
    Q_OBJECT

public:
    explicit Inhibition(const QString &appName, const QString &reason, QObject *parent = nullptr);
    ~Inhibition() override;

private:
    void inhibit();
    void uninhibit();

    QString m_appName;
    QString m_reason;

#ifdef Q_OS_WINDOWS
    HANDLE m_handle = INVALID_HANDLE_VALUE;
#elif HAVE_QTDBUS
    QDBusObjectPath m_portalInhibitionRequest; // portal.
    std::optional<uint> m_inhibitionCookie; // fdo.
#endif
};

class KInhibitionJobTrackerPrivate
{
public:
    explicit KInhibitionJobTrackerPrivate(KInhibitionJobTracker *q);

    void inhibit(KJob *job);

    QHash<KJob *, QString> reasons;
    QHash<KJob *, QTimer *> timers;
    QHash<KJob *, Inhibition *> inhibitions;

private:
    KInhibitionJobTracker *const q;

    void doInhibit(KJob *job);
};

#endif // KINHIBITIONJOBTRACKER_P_H
