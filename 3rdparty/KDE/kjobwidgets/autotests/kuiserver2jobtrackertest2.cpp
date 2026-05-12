/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Méven Car <meven@kde.org>
    SPDX-FileCopyrightText: 2024 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kuiserverv2jobtracker.h>

#include <QDebug>
#include <QFileInfo>
#include <QObject>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>

#include <kjobwidgets_export.h>
#include <kstatusbarjobtracker.h>
#include <kuiserverjobtracker.h>
#include <kwidgetjobtracker.h>
#include <qobject.h>
#include <qsignalspy.h>
#include <qtest.h>

#include <KJob>

class TestJob : public KJob
{
    Q_OBJECT

public:
    TestJob()
    {
        m_timer.callOnTimeout(this, &TestJob::emitResult);
    }

    ~TestJob() override = default;

    void start() override
    {
        m_timer.start(100);
    }

    bool doSuspend() override
    {
        m_timer.stop();
        return true;
    }

    bool doResume() override
    {
        m_timer.start();
        return true;
    }
    bool doKill() override
    {
        m_timer.stop();
        return true;
    }

private:
    QTimer m_timer;
};

class KUiServerV2JobTrackerTest2 : public QObject
{
    Q_OBJECT
public:
    KUiServerV2JobTrackerTest2(QObject *parent = nullptr)
        : QObject(parent)
    {
        qGuiApp->setDesktopFileName(QStringLiteral("nonexisting.desktop"));
    }

private Q_SLOTS:
    void simpleTest()
    {
        QPointer<TestJob> job = new TestJob;

        KUiServerV2JobTracker *tracker = new KUiServerV2JobTracker(this);
        tracker->registerJob(job);

        job->start();
        job->suspend();
        job->resume();
        QTRY_VERIFY(job == nullptr);

        delete tracker;
    }
};

QTEST_MAIN(KUiServerV2JobTrackerTest2)

#include "kuiserver2jobtrackertest2.moc"
