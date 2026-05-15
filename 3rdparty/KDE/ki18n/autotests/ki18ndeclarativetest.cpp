/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTest>

#include <KLocalizedContext>
#include <KLocalizedQmlContext>
#include <QDebug>

using namespace Qt::Literals;

class KI18nDeclarativeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLocalizedContext_data()
    {
        QTest::addColumn<QString>("propertyName");
        QTest::addColumn<QString>("value");

        QTest::newRow("translation") << "testString" << QStringLiteral("Awesome");
        QTest::newRow("singular translation") << "testStringSingular" << QStringLiteral("and 1 other window");
        QTest::newRow("plural translation") << "testStringPlural" << QStringLiteral("and 3 other windows");
        QTest::newRow("plural translation with domain") << "testStringPluralWithDomain" << QStringLiteral("in 3 seconds");
        QTest::newRow("null string arg") << "testNullStringArg" << QStringLiteral("Awesome ");
        QTest::newRow("zero") << "testZero" << QStringLiteral("I'm 0 years old");
    }

    void testLocalizedContext()
    {
        QFETCH(QString, propertyName);
        QFETCH(QString, value);

        KLocalizedContext ctx;

        QQmlApplicationEngine engine;
        engine.rootContext()->setContextObject(&ctx);
        engine.loadFromModule("org.kde.i18n.declarativetest", "Test");
        QVERIFY(!engine.hasError());
        QCOMPARE(engine.rootObjects().size(), 1);
        QObject *object = engine.rootObjects().at(0);
        QVERIFY(object);
        QCOMPARE(object->property(propertyName.toUtf8().constData()).toString(), value);
    }

    void testLocalizedQmlContext_data()
    {
        QTest::addColumn<QString>("propertyName");
        QTest::addColumn<QString>("value");

        QTest::newRow("translation") << "testString" << QStringLiteral("Awesome");
        QTest::newRow("singular translation") << "testStringSingular" << QStringLiteral("and 1 other window");
        QTest::newRow("plural translation") << "testStringPlural" << QStringLiteral("and 3 other windows");
        QTest::newRow("plural translation with domain") << "testStringPluralWithDomain" << QStringLiteral("in 3 seconds");
        QTest::newRow("null string arg") << "testNullStringArg" << QStringLiteral("Awesome ");
        QTest::newRow("zero") << "testZero" << QStringLiteral("I'm 0 years old");
        QTest::newRow("context via import") << "testContextViaImport" << QStringLiteral("Meow"); // only may be used with QmlContext!
    }

    void testLocalizedQmlContext()
    {
        QFETCH(QString, propertyName);
        QFETCH(QString, value);

        QQmlApplicationEngine engine;
        auto ctx = KLocalization::setupLocalizedContext(&engine);
        ctx->setTranslationDomain(u"ki18n-test"_s);

        engine.loadFromModule("org.kde.i18n.declarativetest", "Test");
        QVERIFY(!engine.hasError());
        QCOMPARE(engine.rootObjects().size(), 1);
        QObject *object = engine.rootObjects().at(0);
        QVERIFY(object);
        QCOMPARE(object->property(propertyName.toUtf8().constData()).toString(), value);
    }
};

QTEST_MAIN(KI18nDeclarativeTest)

#include "ki18ndeclarativetest.moc"
