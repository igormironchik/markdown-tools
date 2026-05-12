/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KUISERVERJOBTRACKER_H
#define KUISERVERJOBTRACKER_H

#include <kjobwidgets_export.h>

#if KJOBWIDGETS_ENABLE_DEPRECATED_SINCE(6, 17)

#include <KJobTrackerInterface>

class KJob;

/*!
 * \class KUiServerJobTracker
 *
 * \inmodule KJobWidgets
 *
 * \brief The interface to implement to track the progresses of a job.
 * \deprecated[6.17]
 * Use KUiServerV2JobTracker
 */
class KJOBWIDGETS_EXPORT KJOBWIDGETS_DEPRECATED_VERSION(6, 17, "Use KUiServerV2JobTracker") KUiServerJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /*!
     * Creates a new KJobTrackerInterface
     *
     * \a parent the parent object
     */
    explicit KUiServerJobTracker(QObject *parent = nullptr);

    ~KUiServerJobTracker() override;

    /*!
     * Register a new job in this tracker.
     *
     * \a job the job to register
     */
    void registerJob(KJob *job) override;

    /*!
     * Unregister a job from this tracker.
     *
     * \a job the job to unregister
     */
    void unregisterJob(KJob *job) override;

protected Q_SLOTS:
    void finished(KJob *job) override;
    void suspended(KJob *job) override;
    void resumed(KJob *job) override;
    void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override;
    void infoMessage(KJob *job, const QString &message) override;
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void percent(KJob *job, unsigned long percent) override;
    void speed(KJob *job, unsigned long value) override;

private:
    class Private;
    std::unique_ptr<Private> const d;

    Q_PRIVATE_SLOT(d, void _k_killJob())
};
#endif // KWIDGETS_ENABLE_DEPRECATED_SINCE

#endif
