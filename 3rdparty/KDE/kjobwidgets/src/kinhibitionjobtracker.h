/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KINHIBITIONJOBTRACKER_H
#define KINHIBITIONJOBTRACKER_H

#include <KJobTrackerInterface>
#include <kjobwidgets_export.h>

#include <memory>

class KJob;
class KInhibitionJobTrackerPrivate;

/*!
 * \class KInhibitionJobTracker
 *
 * \inmodule KJobWidgets
 *
 * \brief Block standby mode while a job is running.
 *
 * This job tracker will prevent the system from going into standby mode
 * while a given job is running. This can be used to guard potentially lengthy
 * operations, such as copying files, encoding videos, extracting archives, etc.
 *
 * When the job is started, a power management inhibition is posted after a
 * short delay. When the job is paused, the inhibition is lifted until the job
 * is resumed again.
 *
 * \since 6.18
 */
class KJOBWIDGETS_EXPORT KInhibitionJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /*!
     * Creates a new KInhibitionJobTracker
     *
     * \a parent the parent object
     */
    explicit KInhibitionJobTracker(QObject *parent = nullptr);

    /*!
     * Creates a new KJobTrackerInterface
     *
     * \a parent the parent object
     */
    ~KInhibitionJobTracker() override;

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

private:
    std::unique_ptr<KInhibitionJobTrackerPrivate> const d;
};

#endif // KINHIBITIONJOBTRACKER_H
