// SPDX-FileCopyrightText: 2024 g10 Code GmbH
// SPDX-FileContributor: Tobias Fella <tobias.fella@gnupg.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QTest>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QProcess>
#include <QSettings>

using namespace Qt::Literals;

class RegistryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        qApp->setOrganizationName(QStringLiteral("KDE"));

        auto hkcuPath = QFINDTESTDATA("registry_test_hkcu.reg");
        QVERIFY(hkcuPath.length() > 0);
        QCOMPARE(QProcess::execute(QStringLiteral("reg"), {QStringLiteral("import"), hkcuPath}), 0);
        QFile::remove(QStringLiteral("registry_test_hkcu.reg"));

        auto hklmPath = QFINDTESTDATA("registry_test_hklm.reg");
        QVERIFY(hklmPath.length() > 0);
        if (QProcess::execute(QStringLiteral("reg"), {QStringLiteral("import"), hklmPath})) {
            qWarning() << "Unable to import HKLM registry keys (missing privileges?). Import them manually and repeat test.";
        }
        QFile::remove(QStringLiteral("registry_test_hklm.reg"));
    }
    void testReadValues()
    {
        auto config = KSharedConfig::openConfig();

        KConfigGroup defaultGroup(config, QStringLiteral("<default>"));
        QCOMPARE(defaultGroup.readEntry("should_be_1", 0), 1);

        KConfigGroup group(config, QStringLiteral("fooGroup"));
        QCOMPARE(group.readEntry("should_be_5", 1), 5);
        QCOMPARE(group.readEntry("should_be_hello", QString()), QStringLiteral("hello"));

        KConfigGroup nestedGroup(&group, QStringLiteral("nestedGroup"));
        QCOMPARE(nestedGroup.readEntry("should_be_1", 0), 1);

        KConfigGroup immutableGroup(config, QStringLiteral("immutableGroup"));
        QCOMPARE(immutableGroup.readEntry("should_be_hello", QString()), QStringLiteral("hello"));
        KConfigGroup immutableNestedGroup(&immutableGroup, QStringLiteral("nestedGroup"));
        QCOMPARE(immutableNestedGroup.readEntry("should_be_hello", QString()), QStringLiteral("hello"));
    }

    void testImmutability()
    {
        auto config = KSharedConfig::openConfig();
        KConfigGroup group(config, QStringLiteral("fooGroup"));
        QVERIFY(!group.isImmutable());
        QVERIFY(!group.isEntryImmutable("should_be_5"));
        QVERIFY(group.isEntryImmutable("should_be_immutable"));

        KConfigGroup nestedGroup(&group, QStringLiteral("nestedGroup"));
        QVERIFY(!nestedGroup.isImmutable());

        KConfigGroup immutableGroup(config, QStringLiteral("immutableGroup"));
        QVERIFY(immutableGroup.isImmutable());
        QVERIFY(immutableGroup.isEntryImmutable("normal_entry"));
        QVERIFY(immutableGroup.isEntryImmutable("immutable_entry"));

        QVERIFY(immutableGroup.hasGroup(QStringLiteral("nestedGroup")));
        KConfigGroup immutableNestedGroup(&immutableGroup, QStringLiteral("nestedGroup"));
        QVERIFY(immutableNestedGroup.isImmutable());
    }

    void testFileName()
    {
        QCOMPARE(KSharedConfig::openConfig(QStringLiteral("named_config"))->group(QStringLiteral("<default>")).readEntry("value", 0), 1);
        // trailing "rc" is removed from file name
        QCOMPARE(KSharedConfig::openConfig(QStringLiteral("named_configrc"))->group(QStringLiteral("<default>")).readEntry("value", 0), 1);
    }

    void testHierarchy()
    {
        auto config = KSharedConfig::openConfig();
        KConfigGroup group(config, QStringLiteral("<default>"));
        QCOMPARE(group.readEntry("value_from_hklm", QString()), QStringLiteral("hklm"));
        QCOMPARE(group.readEntry("overwritten_value", QString()), QStringLiteral("hkcu"));
        QCOMPARE(group.readEntry("immutable_value", QString()), QStringLiteral("hklm"));
        QVERIFY(group.isEntryImmutable("immutable_value"));
        QVERIFY(!group.isEntryImmutable("value_from_hklm"));

        KConfigGroup immutableGroup(config, QStringLiteral("inherit_immutable_group"));
        QCOMPARE(immutableGroup.readEntry("test_value", 0), 0);
    }

    void testNotExistingRegistryKeyIsNotCreated()
    {
        KSharedConfig::openConfig(u"does_not_exist"_s);
        QSettings settings(u"HKEY_CURRENT_USER\\SOFTWARE"_s, QSettings::NativeFormat);
        QVERIFY(settings.childGroups().contains(u"KDE"_s));
        settings.beginGroup(u"KDE"_s);
        const QStringList allKeys = settings.allKeys();
        QVERIFY(!settings.childGroups().contains(u"does_not_exist"_s));
    }
};

QTEST_MAIN(RegistryTest)

#include "registrytest.moc"
