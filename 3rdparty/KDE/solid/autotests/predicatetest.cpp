/*
    SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

// Qt includes
#include <QTest>

// Solid includes
#include <solid/predicate.h>

class PredicateTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParsing();
};

QTEST_MAIN(PredicateTest)

void PredicateTest::testParsing()
{
    Solid::Predicate equalPredicate = Solid::Predicate::fromString(QStringLiteral("IS OpticalDisc"));
    QVERIFY(equalPredicate.isValid());
    QCOMPARE(equalPredicate.interfaceType(), Solid::DeviceInterface::OpticalDisc);
    QCOMPARE(equalPredicate.isInterfaceTypeNegated(), false);

    Solid::Predicate notEqualPredicate = Solid::Predicate::fromString(QStringLiteral("IS_NOT OpticalDisc"));
    QVERIFY(notEqualPredicate.isValid());
    QCOMPARE(notEqualPredicate.interfaceType(), Solid::DeviceInterface::OpticalDisc);
    QCOMPARE(notEqualPredicate.isInterfaceTypeNegated(), true);
}

#include "predicatetest.moc"
