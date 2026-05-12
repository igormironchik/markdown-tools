/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <kshareddatacache.h>

#include <QTest>

#include <QObject>
#include <QStandardPaths>
#include <QString>
#include <string.h> // strcpy

class KSharedDataCacheTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void simpleInsert();
    void remove();
};

void KSharedDataCacheTest::initTestCase()
{
}

static QString makeCacheFileName(const QString &cacheName)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/") + cacheName + QLatin1String(".kcache");
}

void KSharedDataCacheTest::simpleInsert()
{
    const QLatin1String cacheName("myTestCache");
    const QLatin1String key("mypic");
    // clear the cache
    QFile file(makeCacheFileName(cacheName));
    if (file.exists()) {
        QVERIFY(file.remove());
    }
    // insert something into it
    KSharedDataCache cache(cacheName, 5 * 1024 * 1024);
#ifndef Q_OS_WIN // the windows implementation is currently only memory based and not really shared
    QVERIFY(file.exists()); // make sure we got the cache filename right
#endif
    QByteArray data;
    data.resize(9228);
    strcpy(data.data(), "Hello world");
    QVERIFY(cache.insert(key, data));
    // read it out again
    QByteArray result;
    QVERIFY(cache.find(key, &result));
    QCOMPARE(result, data);
    // another insert
    strcpy(data.data(), "Hello KDE");
    QVERIFY(cache.insert(key, data));
    // and another read
    QVERIFY(cache.find(key, &result));
    QCOMPARE(result, data);
}

void KSharedDataCacheTest::remove()
{
    const QLatin1String cacheName("remove");

    // remove a cache from previous test runs
    QFile file(makeCacheFileName(cacheName));
    if (file.exists()) {
        QVERIFY(file.remove());
    }

    KSharedDataCache cache(cacheName, 5 * 1024 * 1024);

    // foo is not in the cache yet
    QVERIFY(!cache.remove(QStringLiteral("foo")));

    // add a foo item
    QVERIFY(cache.insert(QStringLiteral("foo"), QByteArrayLiteral("bar")));
    QVERIFY(cache.contains(QStringLiteral("foo")));

    // remove foo
    QVERIFY(cache.remove(QStringLiteral("foo")));
    QVERIFY(!cache.contains(QStringLiteral("foo")));

    // attempt to remove foo again
    QVERIFY(!cache.remove(QStringLiteral("foo")));
    QVERIFY(!cache.contains(QStringLiteral("foo")));
}

QTEST_MAIN(KSharedDataCacheTest)

#include "kshareddatacachetest.moc"
