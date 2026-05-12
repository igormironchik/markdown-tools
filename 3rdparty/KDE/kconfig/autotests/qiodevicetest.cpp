// SPDX-License-Identifier: LGPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#include <QBuffer>
#include <QTemporaryFile>
#include <QTest>

#include <KConfig>
#include <KConfigGroup>

using namespace Qt::StringLiterals;

class QIODeviceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testWriteRead()
    {
        // write to file
        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadWrite | QIODevice::Text));
        {
            KConfig config(buffer, KConfig::OpenFlag::SimpleConfig);
            config.group(u"Test"_s).writeEntry("key", "value");
        }
        buffer->close();

        QVERIFY(buffer->open(QIODevice::ReadOnly | QIODevice::Text));
        QVERIFY(buffer->isReadable());

        // check content
        QCOMPARE(buffer->readAll(), "[Test]\nkey=value\n");
        buffer->seek(0);

        // read back the config
        {
            KConfig config(buffer, KConfig::OpenFlag::SimpleConfig);

            QVERIFY(config.hasGroup(u"Test"_s));
            QCOMPARE(config.group(u"Test"_s).readEntry("key"), u"value"_s);
        }
    }

    void testKdeGlobals()
    {
        {
            KConfig glob(QStringLiteral("kdeglobals"));
            KConfigGroup general(&glob, QStringLiteral("General"));
            general.writeEntry("testKG", "1");
            QVERIFY(glob.sync());
        }

        KConfig globRead(QStringLiteral("kdeglobals"));
        const KConfigGroup general(&globRead, QStringLiteral("General"));
        QCOMPARE(general.readEntry("testKG"), QStringLiteral("1"));

        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadOnly | QIODevice::Text));
        {
            KConfig config(buffer, KConfig::OpenFlag::FullConfig);
            QVERIFY(config.hasGroup(u"General"_s));
            config.group(u"General"_s).writeEntry("testKG", "1");
        }
    }

    void testAddConfigSources()
    {
        QTemporaryFile extrafile;
        extrafile.setAutoRemove(false);
        QVERIFY(extrafile.open());
        extrafile.close();

        {
            KConfig glob(extrafile.fileName());
            KConfigGroup extra(&glob, QStringLiteral("Extra"));
            extra.writeEntry("testKG", "1");
            QVERIFY(glob.sync());
        }

        {
            // check preconditions
            KConfig globRead(extrafile.fileName());
            const KConfigGroup extra(&globRead, QStringLiteral("General"));
            QCOMPARE(extra.readEntry("testKG"), QStringLiteral("1"));
        }

        // When using a QBuffer as QIODevice
        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadWrite | QIODevice::Text));
        {
            KConfig config(buffer, KConfig::OpenFlag::CascadeConfig);
            config.addConfigSources({extrafile.fileName()});
            QVERIFY(config.hasGroup(u"Extra"_s));
            QCOMPARE(config.group(u"Extra"_s).readEntry("testKG"), u"1"_s);

            config.group(u"Extra"_s).writeEntry("testKG", "2");
            QCOMPARE(config.group(u"Extra"_s).readEntry("testKG"), u"2"_s);
        }
        buffer->seek(0);
        QCOMPARE(buffer->readAll(), "[Extra]\ntestKG=2\n");

        QFile extraConfigSource(extrafile.fileName());
        QVERIFY(extraConfigSource.open(QIODevice::ReadOnly));
        auto extraConfigInin = QString::fromUtf8(extraConfigSource.readAll());
        extraConfigInin.remove(QLatin1Char('\r'));
        QCOMPARE(extraConfigInin, u"[Extra]\ntestKG=1\n"_s);

        extrafile.remove();
    }

    void testFileToBuffer()
    {
        QTemporaryFile file;
        file.setAutoRemove(false);
        QVERIFY(file.open());
        file.close();
        {
            KConfig glob(file.fileName());
            KConfigGroup extra(&glob, QStringLiteral("Extra"));
            extra.writeEntry("testKG", "1");
            QVERIFY(glob.sync());
        }

        KConfig glob(file.fileName());

        auto buffer = std::make_shared<QBuffer>();
        QVERIFY(buffer->open(QIODevice::ReadWrite | QIODevice::Text));
        KConfig config(buffer, KConfig::OpenFlag::SimpleConfig);
        config.copyFrom(glob);

        QVERIFY(config.isDirty());
        {
            const KConfigGroup extra(&config, QStringLiteral("Extra"));
            QCOMPARE(extra.readEntry("testKG"), QStringLiteral("1"));
        }

        QCOMPARE(buffer->size(), 0);
        QVERIFY(config.sync());
        QVERIFY(!config.isDirty());

        buffer->seek(0);
        QCOMPARE(buffer->readAll(), "[Extra]\ntestKG=1\n");
    }

    void testQFile()
    {
        QTemporaryFile file;
        file.setAutoRemove(false);
        QVERIFY(file.open());
        file.close();

        auto backingFile = std::make_shared<QFile>(file.fileName());
        QVERIFY(backingFile->open(QIODevice::ReadWrite | QIODevice::Text));
        KConfig config(backingFile, KConfig::OpenFlag::SimpleConfig);

        KConfigGroup group(&config, QStringLiteral("Extra"));
        group.writeEntry("testKG", "1");
        QVERIFY(config.isDirty());
        {
            const KConfigGroup extra(&config, QStringLiteral("Extra"));
            QCOMPARE(extra.readEntry("testKG"), QStringLiteral("1"));
        }

        QCOMPARE(backingFile->size(), 0);
        QVERIFY(config.sync());
        QVERIFY(!config.isDirty());

        backingFile->seek(0);
        QCOMPARE(backingFile->readAll(), "[Extra]\ntestKG=1\n");
    }
};

QTEST_MAIN(QIODeviceTest)

#include "qiodevicetest.moc"
