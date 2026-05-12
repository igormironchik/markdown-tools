/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kjobwidgets.h>

#include "kinhibitionjobtracker.h"
#include "kinhibitionjobtracker_p.h"

#include "debug.h"

#include <KJob>

#include <QCoreApplication>
#include <QTimer>
#include <QVariant>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#elif HAVE_QTDBUS
#include <KSandbox>

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "inhibit_interface.h"
#include "portal_inhibit_interface.h"
#include "portal_request_interface.h"

static constexpr QLatin1String g_portalServiceName{"org.freedesktop.portal.Desktop"};
static constexpr QLatin1String g_portalInhibitObjectPath{"/org/freedesktop/portal/desktop"};

static constexpr QLatin1String g_inhibitServiceName{"org.freedesktop.PowerManagement.Inhibit"};
static constexpr QLatin1String g_inhibitObjectPath{"/org/freedesktop/PowerManagement/Inhibit"};
#endif

KInhibitionJobTrackerPrivate::KInhibitionJobTrackerPrivate(KInhibitionJobTracker *q)
    : q(q)
{
}

void KInhibitionJobTrackerPrivate::inhibit(KJob *job)
{
    if (timers.contains(job) || inhibitions.contains(job)) {
        return;
    }

    qCDebug(KJOBWIDGETS) << "Inhibition scheduled in 10s...";
    // Avoid DBus traffic for short-lived jobs.
    QTimer *timer = new QTimer(job);
    QObject::connect(timer, &QObject::destroyed, q, [this, job] {
        timers.remove(job);
    });
    timer->setSingleShot(true);
    timer->setInterval(10000);
    timer->callOnTimeout(q, [this, job] {
        doInhibit(job);
    });
    timer->start();
    timers.insert(job, timer);
}

void KInhibitionJobTrackerPrivate::doInhibit(KJob *job)
{
    QString appName = job->property("desktopFileName").toString();
    if (appName.isEmpty()) {
        // desktopFileName is in QGuiApplication but we're in Core here.
        appName = QCoreApplication::instance()->property("desktopFileName").toString();
    }
    if (appName.isEmpty()) {
        appName = QCoreApplication::applicationName();
    }

    QString reason = reasons.value(job);
    if (reason.isEmpty()) {
        reason = QCoreApplication::translate("KInhibitionJobTracker", "Job in progress");
    }

    auto *inhibition = new Inhibition(appName, reason);
    Q_ASSERT(!inhibitions.contains(job));
    inhibitions.insert(job, inhibition);
}

Inhibition::Inhibition(const QString &appName, const QString &reason, QObject *parent)
    : QObject(parent)
    , m_appName(appName)
    , m_reason(reason)
{
    inhibit();
}

Inhibition::~Inhibition()
{
    uninhibit();
}

void Inhibition::inhibit()
{
#ifdef Q_OS_WINDOWS
    qCDebug(KJOBWIDGETS) << "Requesting power management inhibition with reason" << m_reason;
    REASON_CONTEXT context{};
    context.Version = POWER_REQUEST_CONTEXT_VERSION;
    context.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
    context.Reason.SimpleReasonString = reinterpret_cast<LPWSTR>(const_cast<ushort *>(m_reason.utf16()));

    HANDLE handle = PowerCreateRequest(&context);
    if (handle == INVALID_HANDLE_VALUE) {
        qWarning(KJOBWIDGETS).nospace() << "Failed to create inhibition request with reason " << m_reason << ": " << GetLastError();
    } else if (!PowerSetRequest(handle, PowerRequestExecutionRequired)) {
        qWarning(KJOBWIDGETS).nospace() << "Failed to set inhibition request with reason " << m_reason << ": " << GetLastError();
        CloseHandle(handle);
    } else {
        m_handle = handle;
    }
#elif HAVE_QTDBUS
    if (KSandbox::isInside()) {
        qCDebug(KJOBWIDGETS) << "Sending portal inhibition with reason" << m_reason;
        org::freedesktop::portal::Inhibit inhibitInterface{g_portalServiceName, g_portalInhibitObjectPath, QDBusConnection::sessionBus()};
        QVariantMap args;
        if (!m_reason.isEmpty()) {
            args.insert(QStringLiteral("reason"), m_reason);
        }
        auto call = inhibitInterface.Inhibit(QString() /* window */, 4 /* Suspend */, args);
        // This is not parented to the job, so we can properly clean up the inhibiton
        // should the inhibition be destroyed before the inhibition has been processed.
        auto *watcher = new QDBusPendingCallWatcher(call);
        QPointer<Inhibition> guard(this);
        connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this, guard, watcher, reason = m_reason] {
            QDBusPendingReply<QDBusObjectPath> reply = *watcher;

            if (reply.isError()) {
                qCWarning(KJOBWIDGETS).nospace() << "Failed to inhibit suspend with reason " << reason << ": " << reply.error().message();
            } else {
                const QDBusObjectPath requestPath = reply.value();

                // By the time the inhibition returned, the inhibition was already gone. Uninhibit again.
                if (!guard) {
                    org::freedesktop::portal::Request requestInterface{g_portalServiceName, requestPath.path(), QDBusConnection::sessionBus()};
                    requestInterface.Close();
                } else {
                    m_portalInhibitionRequest = requestPath;
                }
            }

            watcher->deleteLater();
        });
    } else {
        qCDebug(KJOBWIDGETS) << "Sending freedesktop inhibition with reason" << m_reason;
        org::freedesktop::PowerManagement::Inhibit inhibitInterface{g_inhibitServiceName, g_inhibitObjectPath, QDBusConnection::sessionBus()};
        auto call = inhibitInterface.Inhibit(m_appName, m_reason);
        auto *watcher = new QDBusPendingCallWatcher(call);
        QPointer<Inhibition> guard(this);
        connect(watcher, &QDBusPendingCallWatcher::finished, watcher, [this, guard, watcher, appName = m_appName, reason = m_reason] {
            QDBusPendingReply<uint> reply = *watcher;

            if (reply.isError()) {
                qCWarning(KJOBWIDGETS).nospace() << "Failed to inhibit suspend for " << appName << " with reason " << reason << ": " << reply.error().message();
            } else {
                const uint cookie = reply.value();

                if (!guard) {
                    org::freedesktop::PowerManagement::Inhibit inhibitInterface{g_inhibitServiceName, g_inhibitObjectPath, QDBusConnection::sessionBus()};
                    inhibitInterface.UnInhibit(cookie);
                } else {
                    m_inhibitionCookie = cookie;
                }
            }

            watcher->deleteLater();
        });
    }
#else
    qCDebug(KJOBWIDGETS) << "KInhibitionJobTracker is not supported on this platform";
#endif
}

void Inhibition::uninhibit()
{
#ifdef Q_OS_WINDOWS
    if (m_handle != INVALID_HANDLE_VALUE) {
        if (!PowerClearRequest(m_handle, PowerRequestExecutionRequired)) {
            qWarning(KJOBWIDGETS) << "Failed to clear inhibition request:" << GetLastError();
        }
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
#elif HAVE_QTDBUS
    if (!m_portalInhibitionRequest.path().isEmpty()) {
        org::freedesktop::portal::Request requestInterface{g_portalServiceName, m_portalInhibitionRequest.path(), QDBusConnection::sessionBus()};
        auto call = requestInterface.Close();
        auto *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
            QDBusPendingReply<> reply = *watcher;

            if (reply.isError()) {
                qCWarning(KJOBWIDGETS) << "Failed to uninhibit suspend:" << reply.error().message();
            } else {
                m_portalInhibitionRequest = QDBusObjectPath();
            }

            watcher->deleteLater();
        });
    } else if (m_inhibitionCookie) {
        org::freedesktop::PowerManagement::Inhibit inhibitInterface{g_inhibitServiceName, g_inhibitObjectPath, QDBusConnection::sessionBus()};
        const int cookie = *m_inhibitionCookie;
        auto call = inhibitInterface.UnInhibit(cookie);
        auto *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, cookie] {
            QDBusPendingReply<> reply = *watcher;

            if (reply.isError()) {
                qCWarning(KJOBWIDGETS).nospace() << "Failed to uninhibit suspend for cookie" << cookie << ": " << reply.error().message();
            } else {
                m_inhibitionCookie.reset();
            }

            watcher->deleteLater();
        });
    }
#endif
}

KInhibitionJobTracker::KInhibitionJobTracker(QObject *parent)
    : KJobTrackerInterface(parent)
    , d(std::make_unique<KInhibitionJobTrackerPrivate>(this))
{
}

KInhibitionJobTracker::~KInhibitionJobTracker()
{
    if (!d->inhibitions.isEmpty()) {
        qCWarning(KJOBWIDGETS) << "A KInhibitionJobTracker instance contains" << d->inhibitions.size() << "stalled jobs";
        qDeleteAll(d->inhibitions);
    }
}

void KInhibitionJobTracker::registerJob(KJob *job)
{
    if (d->inhibitions.contains(job)) {
        return;
    }

    if (!job->isSuspended() && !job->error()) {
        d->inhibit(job);
    }

    KJobTrackerInterface::registerJob(job);
}

void KInhibitionJobTracker::unregisterJob(KJob *job)
{
    KJobTrackerInterface::unregisterJob(job);
    finished(job);
}

void KInhibitionJobTracker::finished(KJob *job)
{
    delete d->inhibitions.take(job);
    delete d->timers.take(job);
    d->reasons.remove(job);
}

void KInhibitionJobTracker::suspended(KJob *job)
{
    delete d->inhibitions.take(job);
    delete d->timers.take(job);
}

void KInhibitionJobTracker::resumed(KJob *job)
{
    d->inhibit(job);
}

void KInhibitionJobTracker::description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    Q_UNUSED(field1);
    Q_UNUSED(field2);
    d->reasons.insert(job, title);
    // Not recreating the inhibition just to update the title,
    // lifting an inhibition after some time might trigger any action
    // that was suppressed by it.
}

#include "moc_kinhibitionjobtracker.cpp"
#include "moc_kinhibitionjobtracker_p.cpp"
