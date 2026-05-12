/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABSTRACTWIDGETJOBTRACKER_H
#define KABSTRACTWIDGETJOBTRACKER_H

#include <KJobTrackerInterface>
#include <kjobwidgets_export.h>

#include <memory>

class KJob;
class QWidget;
class KAbstractWidgetJobTrackerPrivate;

/*!
 * \class KAbstractWidgetJobTracker
 * \inmodule KJobWidgets
 *
 * \brief The base class for widget based job trackers.
 */
class KJOBWIDGETS_EXPORT KAbstractWidgetJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    /*!
     * Creates a new KAbstractWidgetJobTracker
     *
     * \a parent the parent of this object and of the widget displaying the job progresses
     */
    explicit KAbstractWidgetJobTracker(QWidget *parent = nullptr);

    ~KAbstractWidgetJobTracker() override;

    // KDE5: move this two virtual methods to be placed correctly (ereslibre)
public Q_SLOTS:
    /*!
     * Register a new job in this tracker.
     * Note that job trackers inheriting from this class can have only one job
     * registered at a time.
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

public:
    /*!
     * The widget associated to this tracker.
     *
     * \a job the job that is assigned the widget we want to return
     * Returns the widget displaying the job progresses
     */
    virtual QWidget *widget(KJob *job) = 0;

    /*!
     * This controls whether the job should be canceled if the dialog is closed.
     *
     * \a job the job's widget that will be stopped when closing
     *
     * \a stopOnClose If true the job will be stopped if the dialog is closed,
     * otherwise the job will continue even on close.
     *
     * \sa stopOnClose()
     */
    void setStopOnClose(KJob *job, bool stopOnClose);

    /*!
     * Checks whether the job will be killed when the dialog is closed.
     *
     * \a job the job's widget that will be stopped when closing
     *
     * Returns \c true if the job is killed on close event, false otherwise.
     *
     * \sa setStopOnClose()
     */
    bool stopOnClose(KJob *job) const;

    /*!
     * This controls whether the dialog should be deleted or only cleaned when
     * the KJob is finished (or canceled).
     *
     * If your dialog is an embedded widget and not a separate window, you should
     * setAutoDelete(false) in the constructor of your custom dialog.
     *
     * \a job the job's widget that is going to be auto-deleted
     *
     * \a autoDelete If false the dialog will only call method slotClean.
     *
     * If true the dialog will be deleted.
     *
     * \sa autoDelete()
     */
    void setAutoDelete(KJob *job, bool autoDelete);

    /*!
     * Checks whether the dialog should be deleted or cleaned.
     *
     * \a job the job's widget that will be auto-deleted
     *
     * Returns \c false if the dialog only calls slotClean, true if it will be deleted
     * \sa setAutoDelete()
     */
    bool autoDelete(KJob *job) const;

protected Q_SLOTS:
    void finished(KJob *job) override;

    /*!
     * This method should be called for correct cancellation of IO operation
     * Connect this to the progress widgets buttons etc.
     *
     * \a job The job that is being stopped
     */
    virtual void slotStop(KJob *job);

    /*!
     * This method should be called for pause/resume
     * Connect this to the progress widgets buttons etc.
     *
     * \a job The job that is being suspended
     */
    virtual void slotSuspend(KJob *job);

    /*!
     * This method should be called for pause/resume
     * Connect this to the progress widgets buttons etc.
     *
     * \a job The job that is being resumed
     */
    virtual void slotResume(KJob *job);

    /*!
     * This method is called when the widget should be cleaned (after job is finished).
     * redefine this for custom behavior.
     *
     * \a job The job that is being cleaned
     */
    virtual void slotClean(KJob *job);

Q_SIGNALS:
    /*!
     * Emitted when the user aborted the operation
     *
     * \a job The job that has been stopped
     */
    void stopped(KJob *job);

    /*!
     * Emitted when the user suspended the operation
     *
     * \a job The job that has been suspended
     */
    void suspend(KJob *job);

    /*!
     * Emitted when the user resumed the operation
     *
     * \a job The job that has been resumed
     */
    void resume(KJob *job);

protected:
    KJOBWIDGETS_NO_EXPORT explicit KAbstractWidgetJobTracker(KAbstractWidgetJobTrackerPrivate &dd, QWidget *parent = nullptr);

protected:
    std::unique_ptr<KAbstractWidgetJobTrackerPrivate> const d_ptr;

private:
    Q_DECLARE_PRIVATE(KAbstractWidgetJobTracker)
};

#endif
