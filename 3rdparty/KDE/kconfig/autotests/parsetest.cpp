/*  This file is part of the KDE libraries
 *  SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QTest>

#include <KConfig>
#include <KConfigGroup>

class ParseTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testParse_data()
    {
        QTest::addColumn<QString>("fileName");

        QTest::addRow("crash1") << QFINDTESTDATA("data/crash1");
    }

    void testParse()
    {
        QFETCH(QString, fileName);

        KConfig config(fileName);

        const auto groups = config.groupList();
        for (const auto &groupName : groups) {
            KConfigGroup group = config.group(groupName);
            group.entryMap();
        }
    }
};

QTEST_GUILESS_MAIN(ParseTest)

#include "parsetest.moc"
