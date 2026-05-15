/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QFileSystemWatcher>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

class TestKWindowsystemPlatformWayland : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testWithHelper();

private:
    std::unique_ptr<QProcess> m_westonProcess;
    QTemporaryDir m_xdgRuntimeDir;
};

void TestKWindowsystemPlatformWayland::initTestCase()
{
    // we need weston, else skip this
    const QString westonExec = QStandardPaths::findExecutable(QStringLiteral("weston"));
    if (westonExec.isEmpty()) {
        QSKIP("Weston is not installed");
    }

    // we need a valid XDG_RUNTIME_DIR, separate it from the one the user might have
    QVERIFY(m_xdgRuntimeDir.isValid());
    qputenv("XDG_RUNTIME_DIR", m_xdgRuntimeDir.path().toLocal8Bit());

    // start Weston
    m_westonProcess.reset(new QProcess);
    m_westonProcess->setProgram(westonExec);
    m_westonProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    m_westonProcess->setArguments(QStringList({QStringLiteral("--socket=kwindowsystem-platform-wayland-0"),
                                               QStringLiteral("--backend=headless"),
                                               QStringLiteral("--shell=kiosk"),
                                               QStringLiteral("--no-config")}));
    m_westonProcess->start();
    QVERIFY(m_westonProcess->waitForStarted());

    // wait for the socket to appear
    const QDir runtimeDir(m_xdgRuntimeDir.path());
    QTRY_VERIFY_WITH_TIMEOUT(runtimeDir.exists(QStringLiteral("kwindowsystem-platform-wayland-0")), 5000);
}

void TestKWindowsystemPlatformWayland::cleanupTestCase()
{
    if (!m_westonProcess) {
        return;
    }
    m_westonProcess->terminate();
    QVERIFY(m_westonProcess->waitForFinished());
    m_westonProcess.reset();
}

void TestKWindowsystemPlatformWayland::testWithHelper()
{
    // This test starts a helper binary on platform wayland
    // it executes the actual test and will return 0 on success, and an error value otherwise
    QString processName = QFINDTESTDATA("kwindowsystem_platform_wayland_helper");
    QVERIFY(!processName.isEmpty());

    QProcess helper;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("WAYLAND_DISPLAY"), QStringLiteral("kwindowsystem-platform-wayland-0"));
    helper.setProgram(processName);
    helper.setProcessEnvironment(env);
    helper.start();
    QVERIFY(helper.waitForFinished());
    QCOMPARE(helper.exitCode(), 0);
}

QTEST_GUILESS_MAIN(TestKWindowsystemPlatformWayland)
#include "kwindowsystem_platform_wayland_test.moc"
