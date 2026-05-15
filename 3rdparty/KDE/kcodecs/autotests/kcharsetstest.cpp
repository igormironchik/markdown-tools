/*
    SPDX-FileCopyrightText: 2011 Romain Perier <bambi@kubuntu.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDebug>
#include <QObject>
#include <QString>
#include <QTest>
#include <kcharsets.h>

using namespace Qt::Literals;

static bool encodingNameHasADescription(const QString &encodingName, const QStringList &descriptions)
{
    return std::any_of(descriptions.cbegin(), descriptions.cend(), [&encodingName](const QString &description) {
        return description.contains(encodingName);
    });
}

class KCharsetsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSingleton();
    void testFromEntity();
    void testToEntity();
    void testResolveEntities();
    void testEncodingNames();
    void benchmarkFromEntity();
};

void KCharsetsTest::testSingleton()
{
    QVERIFY(KCharsets::charsets() != nullptr);
    QCOMPARE(KCharsets::charsets(), KCharsets::charsets());
}

void KCharsetsTest::testFromEntity()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->fromEntity(QString::fromLatin1("&#1234")), QChar(1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("&#x1234")), QChar(0x1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("#x1234")), QChar(0x1234));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("lt")), QChar::fromLatin1('<'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("gt")), QChar::fromLatin1('>'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("quot")), QChar::fromLatin1('"'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("amp")), QChar::fromLatin1('&'));
    QCOMPARE(singleton->fromEntity(QString::fromLatin1("apos")), QChar::fromLatin1('\''));
    QCOMPARE(singleton->fromEntity(u"aposgarbagesuffix"_s), QChar());
    QCOMPARE(singleton->fromEntity(u"thetasym"_s), QChar(0x03d1));
    QCOMPARE(singleton->fromEntity(u"thetasymgarbagesuffix"_s), QChar());
}

void KCharsetsTest::testToEntity()
{
    KCharsets *singleton = KCharsets::charsets();

    // Character entity references are not implemented,
    // numeric character references are used in all cases
    QCOMPARE(singleton->toEntity(QChar(0x1234)), QString::fromLatin1("&#x1234;"));
}

void KCharsetsTest::testResolveEntities()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->resolveEntities(QString::fromLatin1("&quot;&apos;&lt;Hello &amp;World&gt;&apos;&quot;")),
             QString::fromLatin1("\"\'<Hello &World>\'\""));
}

void KCharsetsTest::testEncodingNames()
{
    KCharsets *singleton = KCharsets::charsets();

    const auto descriptiveEncodingNames = singleton->descriptiveEncodingNames();
    const auto availableEncodingNames = singleton->availableEncodingNames();
    const auto encodingsByScript = singleton->encodingsByScript();

    QCOMPARE(descriptiveEncodingNames.count(), availableEncodingNames.count());

    for (const QString &encodingName : availableEncodingNames) {
        QVERIFY(encodingNameHasADescription(encodingName, descriptiveEncodingNames));

        auto description = singleton->descriptionForEncoding(encodingName);
        QVERIFY(!description.isEmpty());
        QCOMPARE(singleton->encodingForName(description), encodingName);
    }
}

void KCharsetsTest::benchmarkFromEntity()
{
    KCharsets *singleton = KCharsets::charsets();

    QCOMPARE(singleton->fromEntity(u"AElig"_s), QChar(0x00c6));
    QCOMPARE(singleton->fromEntity(u"zwj"_s), QChar(0x200d));

    QBENCHMARK {
        singleton->fromEntity(u"AElig"_s);
        singleton->fromEntity(u"zwj"_s);
    }
}

QTEST_MAIN(KCharsetsTest)

#include "kcharsetstest.moc"
