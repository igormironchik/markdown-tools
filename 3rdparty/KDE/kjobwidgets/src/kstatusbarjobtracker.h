/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Matej Koss <koss@miesto.sk>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSTATUSBARJOBTRACKER_H
#define KSTATUSBARJOBTRACKER_H

#include <kabstractwidgetjobtracker.h>

class KStatusBarJobTrackerPrivate;

/*!
 * \class KStatusBarJobTracker
 *
 * \inmodule KJobWidgets
 *
 * \brief This class implements a job tracker with a widget suited for embedding in a
 * status bar.
 */
class KJOBWIDGETS_EXPORT KStatusBarJobTracker : public KAbstractWidgetJobTracker
{
    Q_OBJECT

public:
    /*!
     * \value NoInformation Does not show any information
     * \value LabelOnly Shows an informative label for job progress
     * \value ProgressOnly Shows a progress bar with the job completion
     */
    enum StatusBarMode {
        NoInformation = 0x0000,
        LabelOnly = 0x0001,
        ProgressOnly = 0x0002,
    };
    Q_DECLARE_FLAGS(StatusBarModes, StatusBarMode)

    /*!
     * Creates a new KStatusBarJobTracker
     *
     * \a parent the parent of this object and of the widget displaying the job progresses
     *
     * \a button true to display a stop button allowing to kill the job, false otherwise
     */
    explicit KStatusBarJobTracker(QWidget *parent = nullptr, bool button = true);

    ~KStatusBarJobTracker() override;

    void registerJob(KJob *job) override;

    void unregisterJob(KJob *job) override;

    QWidget *widget(KJob *job) override;

    /*!
     * Sets the mode of the status bar.
     *
     * \a statusBarMode what information the status bar will show (see StatusBarMode).
     *                      LabelOnly by default
     */
    void setStatusBarMode(StatusBarModes statusBarMode);

public Q_SLOTS:
    void description(KJob *job, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override;
    void totalAmount(KJob *job, KJob::Unit unit, qulonglong amount) override;
    void percent(KJob *job, unsigned long percent) override;
    void speed(KJob *job, unsigned long value) override;
    void slotClean(KJob *job) override;

private:
    Q_DECLARE_PRIVATE(KStatusBarJobTracker)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KStatusBarJobTracker::StatusBarModes)

#endif
