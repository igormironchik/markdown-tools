/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KConfig>
#include <KConfigGroup>

#include <QObject>
#include <QStandardPaths>
#include <QTest>

// clazy:excludeall=non-pod-global-static
static const QString s_test_subdir{QStringLiteral("kconfigtest_subdir/")};
static const QString s_kconfig_test_subdir(s_test_subdir + QLatin1String("kconfigtest"));
static const QString s_string_entry1(QStringLiteral("hello"));

class KConfigBenchmark : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testParsing();
    void testHasKey();
    void testReadEntry();
    void testKConfigGroupKeyList();
};

void KConfigBenchmark::initTestCase()
{
    // ensure we don't use files in the real config directory
    QStandardPaths::setTestModeEnabled(true);

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));
    cg.deleteGroup();
    cg.writeEntry("UsedEntry", s_string_entry1);
}

void KConfigBenchmark::testParsing()
{
    QString fileName =
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + s_test_subdir + QLatin1String("pathtest.ini");
    qputenv("WITHSLASH", "/a/");
    {
        QFile file(fileName);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&file);
        out << "[Test Group]\n"
            << "homePath=$HOME/foo\n"
            << "homePath2=file://$HOME/foo\n"
            << "withSlash=$WITHSLASH/foo\n"
            << "withSlash2=$WITHSLASH\n"
            << "withBraces[$e]=file://${HOME}/foo\n"
            << "URL[$e]=file://${HOME}/foo\n"
            << "hostname[$e]=$(hostname)\n"
            << "escapes=aaa,bb/b,ccc\\,ccc\n"
            << "\n\n"
            << "noeol=foo" // no EOL
            ;
    }

    QStringList groups;
    QBENCHMARK {
        KConfig sc(fileName, KConfig::SimpleConfig);
        groups = sc.groupList();
    }

    const auto expected = QStringList{QStringLiteral("Test Group")};
    QCOMPARE(groups, expected);
}

void KConfigBenchmark::testHasKey()
{
    bool hasUsedKey = false;
    bool hasNotUsedKey = true;

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));

    QBENCHMARK {
        hasUsedKey = cg.hasKey("UsedEntry");
        hasNotUsedKey = cg.hasKey("NotUsedEntry");
    }

    QCOMPARE(hasUsedKey, true);
    QCOMPARE(hasNotUsedKey, false);
}

void KConfigBenchmark::testReadEntry()
{
    QString usedEntry;
    QString notUsedEntry;
    const QString defaultEntry = QStringLiteral("Default");

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));

    QBENCHMARK {
        usedEntry = cg.readEntry("UsedEntry", defaultEntry);
        notUsedEntry = cg.readEntry("NotUsedEntry", defaultEntry);
    }

    QCOMPARE(usedEntry, s_string_entry1);
    QCOMPARE(notUsedEntry, defaultEntry);
}

void KConfigBenchmark::testKConfigGroupKeyList()
{
    QStringList keyList;

    KConfig sc(s_kconfig_test_subdir);
    KConfigGroup cg(&sc, QStringLiteral("Main"));
    cg.writeEntry("Entry2", s_string_entry1);
    cg.writeEntry("Entry3", s_string_entry1);

    QBENCHMARK {
        keyList = cg.keyList();
    }

    const QStringList expectedKeyList{
        QStringLiteral("Entry2"),
        QStringLiteral("Entry3"),
        QStringLiteral("UsedEntry"),
    };
    QCOMPARE(keyList, expectedKeyList);
}

QTEST_GUILESS_MAIN(KConfigBenchmark)

#include "kconfig_benchmark.moc"
