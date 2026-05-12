/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTest>

#include "test16.h"

class Test16Qml : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQml()
    {
        QQmlApplicationEngine engine;

        Test16 cfg;

        qmlRegisterSingletonInstance("org.kde.kconfig.test16", 1, 0, "TestCfg", &cfg);

        engine.loadFromModule("org.kde.kconfig.test16", "Test16Qml");

        QVERIFY(!engine.rootObjects().isEmpty());
    }
};

QTEST_MAIN(Test16Qml)

#include "test16qml.moc"
