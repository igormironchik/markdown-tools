/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOBTRACKERINTERFACE_H
#define KJOBTRACKERINTERFACE_H

#include <kcoreaddons_export.h>
#include <kjob.h>

#include <QObject>
#include <QPair>

#include <memory>

/*!
 * \class KJobTrackerInterface
 * \inmodule KCoreAddons
 *
 * \brief The interface to implement to track the progresses of a job.
 */
class KCOREADDONS_EXPORT KJobTrackerInterface : public QObject
{
    Q_OBJECT

public:
    /*!
     * Creates a new KJobTrackerInterface
     *
     * \a parent the parent object
     */
    explicit KJobTrackerInterface(QObject *parent = nullptr);

    ~KJobTrackerInterface() override;

public Q_SLOTS:
    /*!
     * Register a new job in this tracker.
     * The default implementation connects the following KJob signals
     * to the respective protected slots of this class:
     * \list
     * \li finished() (also connected to the unregisterJob() slot)
     * \li suspended()
     * \li resumed()
     * \li description()
     * \li infoMessage()
     * \li totalAmount()
     * \li processedAmount()
     * \li percent()
     * \li speed()
     * \endlist
     *
     * If you re-implement this method, you may want to call the default
     * implementation or add at least:
     *
     * \code
     * connect(job, &KJob::finished, this, &MyJobTracker::unregisterJob);
     * \endcode
     *
     * so that you won't have to manually call unregisterJob().
     *
     * \a job the job to register
     *
     * \sa unregisterJob()
     */
    virtual void registerJob(KJob *job);

    /*!
     * Unregister a job from this tracker.
     *
     * \note You need to manually call this method only if you re-implemented
     * registerJob() without connecting KJob::finished to this slot.
     *
     * \a job the job to unregister
     *
     * \sa registerJob()
     */
    virtual void unregisterJob(KJob *job);

protected Q_SLOTS:
    /*!
     * Called when a job is finished, in any case. It is used to notify
     * that the job is terminated and that progress UI (if any) can be hidden.
     *
     * \a job the job that emitted this signal
     */
    virtual void finished(KJob *job);

    /*!
     * Called when a job is suspended.
     *
     * \a job the job that emitted this signal
     */
    virtual void suspended(KJob *job);

    /*!
     * Called when a job is resumed.
     *
     * \a job the job that emitted this signal
     */
    virtual void resumed(KJob *job);

    /*!
     * Called to display general description of a job. A description has
     * a title and two optional fields which can be used to complete the
     * description.
     *
     * Examples of titles are "Copying", "Creating resource", etc.
     * The fields of the description can be "Source" with an URL, and,
     * "Destination" with an URL for a "Copying" description.
     *
     * \a job the job that emitted this signal
     *
     * \a title the general description of the job
     *
     * \a field1 first field (localized name and value)
     *
     * \a field2 second field (localized name and value)
     */
    virtual void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);

    /*!
     * Called to display state information about a job.
     * Examples of message are "Resolving host", "Connecting to host...", etc.
     *
     * \a job the job that emitted this signal
     *
     * \a message the info message
     */
    virtual void infoMessage(KJob *job, const QString &message);

    /*!
     * Emitted to display a warning about a job.
     *
     * \a job the job that emitted this signal
     *
     * \a message the warning message
     */
    virtual void warning(KJob *job, const QString &message);

    /*!
     * Called when we know the amount a job will have to process. The unit of this
     * amount is provided too. It can be called several times for a given job if the job
     * manages several different units.
     *
     * \a job the job that emitted this signal
     *
     * \a unit the unit of the total amount
     *
     * \a amount the total amount
     */
    virtual void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount);

    /*!
     * Regularly called to show the progress of a job by giving the current amount.
     * The unit of this amount is provided too. It can be called several times for a given
     * job if the job manages several different units.
     *
     * \a job the job that emitted this signal
     *
     * \a unit the unit of the processed amount
     *
     * \a amount the processed amount
     */
    virtual void processedAmount(KJob *job, KJob::Unit unit, qulonglong amount);

    /*!
     * Called to show the overall progress of the job.
     * Note that this is not called for finished jobs.
     *
     * \a job the job that emitted this signal
     *
     * \a percent the percentage
     */
    virtual void percent(KJob *job, unsigned long percent);

    /*!
     * Called to show the speed of the job.
     *
     * \a job the job that emitted this signal
     *
     * \a value the current speed of the job
     */
    virtual void speed(KJob *job, unsigned long value);

private:
    std::unique_ptr<class KJobTrackerInterfacePrivate> const d;
};

#endif
