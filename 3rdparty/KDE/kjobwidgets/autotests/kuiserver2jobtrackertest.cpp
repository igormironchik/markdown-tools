/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../tests/kjobtrackerstest.h"
#include <QDebug>

#include <QObject>
#include <QSignalSpy>
#include <QTest>
#include <kjobwidgets_export.h>
#include <kstatusbarjobtracker.h>
#include <kuiserverjobtracker.h>
#include <kwidgetjobtracker.h>

class KTestJobTracker : public KJobTrackerInterface
{
    Q_OBJECT

public:
    KTestJobTracker(QObject *parent = nullptr)
        : KJobTrackerInterface(parent)
    {
    }

public Q_SLOTS:
    void registerJob(KJob *job) override
    {
        registeredJobs.append(job);
    }
    void unregisterJob(KJob *job) override
    {
        unregisteredJobs.append(job);
    }

protected Q_SLOTS:
    void finished(KJob *job) override
    {
        finishedSpy.append(job);
    }
    void suspended(KJob *job) override
    {
        suspendedSpy.append(job);
    }
    void resumed(KJob *job) override
    {
        resumedSpy.append(job);
    }
    void description(KJob * /*job */, const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override
    {
        jobTitle = title;
        jobField1 = field1;
        jobField2 = field2;
    }
    void infoMessage(KJob * /* job */, const QString &message) override
    {
        jobInfoMessage = message;
    }
    void warning(KJob * /* job */, const QString &message) override
    {
        jobWarning = message;
    }
    void totalAmount(KJob * /* job */, KJob::Unit unit, qulonglong amount) override
    {
        jobTotalUnit = unit;
        jobTotalAmount = amount;
    }
    void processedAmount(KJob * /* job */, KJob::Unit unit, qulonglong amount) override
    {
        jobUnit = unit;
        jobAmount = amount;
    }
    void percent(KJob * /* job */, unsigned long percent) override
    {
        jobPercent = percent;
    }
    void speed(KJob * /* job */, unsigned long value) override
    {
        jobSpeed = value;
    }

public:
    QList<KJob *> registeredJobs;
    QList<KJob *> unregisteredJobs;
    QList<KJob *> finishedSpy;
    QList<KJob *> suspendedSpy;
    QList<KJob *> resumedSpy;
    QString jobTitle;
    QPair<QString, QString> jobField1;
    QPair<QString, QString> jobField2;
    QString jobInfoMessage;
    QString jobWarning;
    KJob::Unit jobTotalUnit;
    qulonglong jobTotalAmount;
    KJob::Unit jobUnit;
    qulonglong jobAmount;

    unsigned long jobPercent;
    unsigned long jobSpeed;
};

class KUiServerV2JobTrackerTest : public QObject
{
    void simpleTest()
    {
        KTestJob *testJob = new KTestJob(10 /* 100000 bytes to process */, 5);

        KTestJobTracker *tracker = new KTestJobTracker(this);
        tracker->registerJob(testJob);

        QCOMPARE(tracker->registeredJobs.at(0), testJob);
        QCOMPARE(tracker->finishedSpy.count(), 0);

        testJob->start();

        QSignalSpy jobFinishedSpy(testJob, &KJob::finished);

        QVERIFY(jobFinishedSpy.wait());

        QCOMPARE(tracker->finishedSpy.count(), 1);
    }
};

QTEST_GUILESS_MAIN(KUiServerV2JobTrackerTest)

#include "kuiserver2jobtrackertest.moc"
