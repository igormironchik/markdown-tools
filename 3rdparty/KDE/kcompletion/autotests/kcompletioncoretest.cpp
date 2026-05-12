/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Thomas Braxton <brax108@cox.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompletioncoretest.h"
#include <QSignalSpy>
#include <QTest>
#define clampet strings[0]
#define coolcat strings[1]
#define carpet strings[2]
#define carp strings[3]
#define wclampet wstrings[0]
#define wcoolcat wstrings[1]
#define wcarpet wstrings[2]
#define wcarp wstrings[3]

void Test_KCompletion::initTestCase()
{
    strings = QStringList{QStringLiteral("clampet@test.org"), //
                          QStringLiteral("coolcat@test.org"),
                          QStringLiteral("carpet@test.org"),
                          QStringLiteral("carp@test.org")};
    wstrings = QStringList{QStringLiteral("clampet@test.org:30"),
                           QStringLiteral("coolcat@test.org:20"),
                           QStringLiteral("carpet@test.org:40"),
                           QStringLiteral("carp@test.org:7")};
    qRegisterMetaType<QStringList>("QStringList");
}

void Test_KCompletion::isEmpty()
{
    KCompletion completion;
    QVERIFY(completion.isEmpty());
    QVERIFY(completion.items().isEmpty());
}

void Test_KCompletion::insertionOrder()
{
    KCompletion completion;
    QSignalSpy spy1(&completion, &KCompletion::match);
    QSignalSpy spy3(&completion, &KCompletion::multipleMatches);

    completion.setOrder(KCompletion::Insertion);
    QCOMPARE(completion.order(), KCompletion::Insertion);

    completion.setItems(strings);
    QCOMPARE(completion.items().count(), strings.count());

    completion.setCompletionMode(KCompletion::CompletionShell);
    QCOMPARE(completion.makeCompletion(QStringLiteral("ca")), QStringLiteral("carp"));
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy1.takeFirst().at(0).toString(), QLatin1String("carp"));
    QCOMPARE(spy3.count(), 1);
    spy3.takeFirst();

    QSignalSpy spy2(&completion, &KCompletion::matches);
    completion.makeCompletion(QStringLiteral("ca"));
    QCOMPARE(spy2.count(), 1);
    QCOMPARE(spy3.count(), 0); // shouldn't be signaled on 2nd call
    QStringList matches = spy2.takeFirst().at(0).toStringList();
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], carp);

    completion.setCompletionMode(KCompletion::CompletionAuto);
    QCOMPARE(completion.makeCompletion(QStringLiteral("ca")), carpet);
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy1.takeFirst().at(0).toString(), carpet);
}

void Test_KCompletion::sortedOrder()
{
    KCompletion completion;
    QSignalSpy spy1(&completion, &KCompletion::match);
    QSignalSpy spy3(&completion, &KCompletion::multipleMatches);

    completion.setOrder(KCompletion::Sorted);
    QCOMPARE(completion.order(), KCompletion::Sorted);

    completion.setItems(strings);
    QCOMPARE(completion.items().count(), 4);

    completion.setCompletionMode(KCompletion::CompletionShell);
    QCOMPARE(completion.makeCompletion(QStringLiteral("ca")), QStringLiteral("carp"));
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy1.takeFirst().at(0).toString(), QStringLiteral("carp"));
    QCOMPARE(spy3.count(), 1);
    spy3.takeFirst();

    QSignalSpy spy2(&completion, &KCompletion::matches);
    completion.makeCompletion(QStringLiteral("ca"));
    QCOMPARE(spy2.count(), 1);
    QCOMPARE(spy3.count(), 0); // shouldn't be signaled on 2nd call

    QStringList matches = spy2.takeFirst().at(0).toStringList();
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carp);
    QCOMPARE(matches[1], carpet);

    completion.setCompletionMode(KCompletion::CompletionAuto);
    QCOMPARE(completion.makeCompletion(QStringLiteral("ca")), carp);
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(spy1.takeFirst().at(0).toString(), carp);
}

void Test_KCompletion::weightedOrder()
{
    KCompletion completion;
    QSignalSpy spy1(&completion, &KCompletion::match);
    QSignalSpy spy3(&completion, &KCompletion::multipleMatches);

    completion.setOrder(KCompletion::Weighted);
    QCOMPARE(completion.order(), KCompletion::Weighted);

    completion.setItems(wstrings);
    QCOMPARE(completion.items().count(), 4);

    completion.setCompletionMode(KCompletion::CompletionShell);
    QCOMPARE(completion.makeCompletion(QStringLiteral("ca")), QStringLiteral("carp"));
    spy1.takeFirst(); // empty the list
    QCOMPARE(spy3.count(), 1);
    spy3.takeFirst();

    QSignalSpy spy2(&completion, &KCompletion::matches);
    completion.makeCompletion(QStringLiteral("ca"));
    QCOMPARE(spy2.count(), 1);
    QCOMPARE(spy3.count(), 0); // shouldn't be signaled on 2nd call

    QStringList matches = spy2.takeFirst().at(0).toStringList();
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], carp);

    completion.setCompletionMode(KCompletion::CompletionAuto);
    QCOMPARE(completion.makeCompletion(QStringLiteral("ca")), carpet);

    matches = completion.substringCompletion(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 3);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], coolcat);
    QCOMPARE(matches[2], carp);
}

void Test_KCompletion::substringCompletion_Insertion()
{
    KCompletion completion;
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.setOrder(KCompletion::Insertion);
    completion.setItems(strings);
    QCOMPARE(completion.items().count(), 4);

    QStringList matches = completion.substringCompletion(QStringLiteral("c"));
    QCOMPARE(matches.count(), 4);
    QCOMPARE(matches[0], clampet);
    QCOMPARE(matches[1], coolcat);
    QCOMPARE(matches[2], carpet);
    QCOMPARE(matches[3], carp);

    matches = completion.substringCompletion(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 3);
    QCOMPARE(matches[0], coolcat);
    QCOMPARE(matches[1], carpet);
    QCOMPARE(matches[2], carp);

    matches = completion.substringCompletion(QStringLiteral("car"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], carp);

    matches = completion.substringCompletion(QStringLiteral("pet"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], clampet);
    QCOMPARE(matches[1], carpet);
}

void Test_KCompletion::substringCompletion_Sorted()
{
    KCompletion completion;
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.setOrder(KCompletion::Sorted);
    completion.setItems(strings);
    QCOMPARE(completion.items().count(), 4);

    QStringList matches = completion.substringCompletion(QStringLiteral("c"));
    QCOMPARE(matches.count(), 4);
    QCOMPARE(matches[0], carp);
    QCOMPARE(matches[1], carpet);
    QCOMPARE(matches[2], clampet);
    QCOMPARE(matches[3], coolcat);

    matches = completion.substringCompletion(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 3);
    QCOMPARE(matches[0], carp);
    QCOMPARE(matches[1], carpet);
    QCOMPARE(matches[2], coolcat);

    matches = completion.substringCompletion(QStringLiteral("car"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carp);
    QCOMPARE(matches[1], carpet);

    matches = completion.substringCompletion(QStringLiteral("pet"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], clampet);
}

void Test_KCompletion::substringCompletion_Weighted()
{
    KCompletion completion;
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.setOrder(KCompletion::Weighted);
    completion.setItems(wstrings);
    QCOMPARE(completion.items().count(), 4);

    QStringList matches = completion.substringCompletion(QStringLiteral("c"));
    QCOMPARE(matches.count(), 4);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], clampet);
    QCOMPARE(matches[2], coolcat);
    QCOMPARE(matches[3], carp);

    matches = completion.substringCompletion(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 3);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], coolcat);
    QCOMPARE(matches[2], carp);

    matches = completion.substringCompletion(QStringLiteral("car"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], carp);

    matches = completion.substringCompletion(QStringLiteral("pet"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], clampet);
}

void Test_KCompletion::allMatches_Insertion()
{
    KCompletion completion;
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.setOrder(KCompletion::Insertion);
    completion.setItems(strings);
    QCOMPARE(completion.items().count(), 4);

    QStringList matches = completion.allMatches(QStringLiteral("c"));
    QCOMPARE(matches.count(), 4);
    QCOMPARE(matches[0], clampet);
    QCOMPARE(matches[1], coolcat);
    QCOMPARE(matches[2], carpet);
    QCOMPARE(matches[3], carp);

    matches = completion.allMatches(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], carp);

    matches = completion.allMatches(QStringLiteral("pet"));
    QCOMPARE(matches.count(), 0);
}

void Test_KCompletion::allMatches_Sorted()
{
    KCompletion completion;
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.setOrder(KCompletion::Sorted);
    completion.setItems(strings);
    QCOMPARE(completion.items().count(), 4);

    QStringList matches = completion.allMatches(QStringLiteral("c"));
    QCOMPARE(matches.count(), 4);
    QCOMPARE(matches[0], carp);
    QCOMPARE(matches[1], carpet);
    QCOMPARE(matches[2], clampet);
    QCOMPARE(matches[3], coolcat);

    matches = completion.allMatches(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carp);
    QCOMPARE(matches[1], carpet);

    matches = completion.allMatches(QStringLiteral("pet"));
    QCOMPARE(matches.count(), 0);
}

void Test_KCompletion::allMatches_Weighted()
{
    KCompletion completion;
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.setOrder(KCompletion::Weighted);
    completion.setItems(wstrings);
    QCOMPARE(completion.items().count(), 4);

    QStringList matches = completion.allMatches(QStringLiteral("c"));
    QCOMPARE(matches.count(), 4);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], clampet);
    QCOMPARE(matches[2], coolcat);
    QCOMPARE(matches[3], carp);

    matches = completion.allMatches(QStringLiteral("ca"));
    QCOMPARE(matches.count(), 2);
    QCOMPARE(matches[0], carpet);
    QCOMPARE(matches[1], carp);

    matches = completion.allMatches(QStringLiteral("pet"));
    QCOMPARE(matches.count(), 0);
}

void Test_KCompletion::cycleMatches_Insertion()
{
    KCompletion completion;
    completion.setOrder(KCompletion::Insertion);
    completion.setItems(strings);
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.makeCompletion(QStringLiteral("ca"));
    QCOMPARE(completion.nextMatch(), carpet);
    QCOMPARE(completion.nextMatch(), carp);
    QCOMPARE(completion.previousMatch(), carpet);
    QCOMPARE(completion.previousMatch(), carp);
}

void Test_KCompletion::cycleMatches_Sorted()
{
    KCompletion completion;
    completion.setOrder(KCompletion::Sorted);
    completion.setItems(strings);
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.makeCompletion(QStringLiteral("ca"));
    QCOMPARE(completion.nextMatch(), carp);
    QCOMPARE(completion.nextMatch(), carpet);
    QCOMPARE(completion.previousMatch(), carp);
    QCOMPARE(completion.previousMatch(), carpet);
}

void Test_KCompletion::cycleMatches_Weighted()
{
    KCompletion completion;
    completion.setOrder(KCompletion::Weighted);
    completion.setItems(wstrings);
    completion.setCompletionMode(KCompletion::CompletionAuto);

    completion.makeCompletion(QStringLiteral("ca"));
    QCOMPARE(completion.nextMatch(), carpet);
    QCOMPARE(completion.nextMatch(), carp);
    QCOMPARE(completion.previousMatch(), carpet);
    QCOMPARE(completion.previousMatch(), carp);
}

QTEST_MAIN(Test_KCompletion)

#include "moc_kcompletioncoretest.cpp"
