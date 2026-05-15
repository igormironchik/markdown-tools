/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUISERVERV2JOBTRACKER_H
#define KUISERVERV2JOBTRACKER_H

#include <KJobTrackerInterface>
#include <kjobwidgets_export.h>

#include <memory>

class KJob;
class KUiServerV2JobTrackerPrivate;

// TODO KF7 rename to KUiServerJobTracker

/*!
 * \class KUiServerV2JobTracker
 *
 * \inmodule KJobWidgets
 *
 * \brief The interface to implement to track the progresses of a job.
 *
 * \since 5.81
 */
class KJOBWIDGETS_EXPORT KUiServerV2JobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /*!
     * Creates a new KJobTrackerInterface
     *
     * \a parent the parent object
     */
    explicit KUiServerV2JobTracker(QObject *parent = nullptr);

    ~KUiServerV2JobTracker() override;

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
    virtual void description(KJob *job, const QString &title,
                             const QPair<QString, QString> &field1,
                             const QPair<QString, QString> &field2) override;
    void infoMessage(KJob *job, const QString &message) override;
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void percent(KJob *job, unsigned long percent) override;
    void speed(KJob *job, unsigned long value) override;

private:
    std::unique_ptr<KUiServerV2JobTrackerPrivate> const d;
};

#endif
