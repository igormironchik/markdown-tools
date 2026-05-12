/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>

namespace
{
const QString s_testMetadataFile = QDir::homePath() + "/.qttest/cache/kcrash-metadata/test.ini";
} // namespace

class KCrashTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // Don't bring up drkonqi
        qputenv("KDE_DEBUG", "1");
        // change to the bin dir
        QDir::setCurrent(QCoreApplication::applicationDirPath());
    }
    void init()
    {
        QFile::remove(s_testMetadataFile);
    }
    void testAutoRestart();
    void testAutoRestartDirectly();
    void testEmergencySave();
    void testPartialMetadata();
};

static const char s_logFileName[] = "kcrashtest_log";

static QByteArray readLogFile()
{
    QFile logFile(QFile::encodeName(s_logFileName));
    if (!logFile.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    return logFile.readAll();
}

static void startCrasher(const QByteArray &flag, const QByteArray &expectedOutput, const QHash<QString, QString> &extraEnv = {})
{
    QFile::remove(QFile::encodeName(s_logFileName));

    QProcess proc;
    QString processName;
#ifdef Q_OS_WIN
    QVERIFY(QFile::exists("./test_crasher.exe"));
    processName = "test_crasher.exe";
#else
    QVERIFY(QFile::exists("./test_crasher"));
    processName = QStringLiteral("./test_crasher");
#endif
    // qDebug() << proc.args();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("ASAN_OPTIONS"), QStringLiteral("handle_segv=0,poison_heap=0")); // Disable ASAN
    for (const auto &[key, value] : extraEnv.asKeyValueRange()) {
        env.insert(key, value);
    }
    proc.setProcessEnvironment(env);
    proc.setProcessChannelMode(QProcess::ForwardedChannels);
    proc.start(processName, QStringList() << flag);
    bool ok = proc.waitForFinished();
    QVERIFY(ok);

    QByteArray logData;
    for (int i = 0; i < 50; ++i) {
        logData = readLogFile();
        if (logData == expectedOutput) {
            return;
        }
        QTest::qSleep(100);
    }
    qDebug() << proc.readAllStandardError();
    QCOMPARE(QString(logData), QString(expectedOutput));
}

void KCrashTest::testAutoRestart() // use kdeinit if possible, otherwise directly (ex: on CI)
{
    startCrasher("AR", "starting AR\nautorestarted AR\n");

#if defined(Q_OS_LINUX)
    // Complete metadata file should have been written
    QFile testFile(s_testMetadataFile);
    QVERIFY(testFile.open(QIODevice::ReadOnly));
    const QByteArray content = testFile.readAll();
    QVERIFY(content.contains("[KCrash]\n"));
    QVERIFY(content.contains("[KCrashComplete]\n"));
#endif
}

void KCrashTest::testAutoRestartDirectly() // test directly (so a developer can test the CI case)
{
    startCrasher("ARD", "starting ARD\nautorestarted ARD\n");
}

void KCrashTest::testEmergencySave()
{
    startCrasher("ES", "starting ES\nsaveFunction called\n");
}

void KCrashTest::testPartialMetadata()
{
    startCrasher("PartialMetadata", "PartialMetadata\n", {{"KCRASH_CRASH_IN_HANDLER", "1"}});

#if defined(Q_OS_LINUX)
    // The file should not contain [KCrashComplete] since the crash happened after QApplication destruction
    QFile testFile(s_testMetadataFile);
    QVERIFY(testFile.open(QIODevice::ReadOnly));
    const QByteArray content = testFile.readAll();
    QVERIFY(content.contains("[KCrash]\n")); // But it should contain the KCrash section
    QVERIFY(!content.contains("[KCrashComplete]\n"));
#endif
}

QTEST_MAIN(KCrashTest)

#include "kcrashtest.moc"
