/*
    SPDX-FileCopyrightText: 2025 Julius Künzel <julius.kuenzel@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTest>

class Test17Qml : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testQml()
    {
        QQmlApplicationEngine engine;

        engine.loadFromModule("org.kde.kconfig.test17", "Test17Qml");

        QVERIFY(!engine.rootObjects().isEmpty());
    }
};

QTEST_MAIN(Test17Qml)

#include "test17qml.moc"
