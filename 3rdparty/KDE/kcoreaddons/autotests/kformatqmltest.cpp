/*
    SPDX-FileCopyrightText: ⓒ 2025 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlContext>
#include <QQmlEngine>
#include <QtQuickTest>

using namespace Qt::Literals;

void setupEnvironment()
{
    qputenv("TZ", "Asia/Kolkata");
    QLocale::setDefault(QLocale(u"en_GB"));
}
Q_CONSTRUCTOR_FUNCTION(setupEnvironment)

class Object
{
    Q_GADGET
    Q_PROPERTY(QDateTime euTime MEMBER m_euTime)
    Q_PROPERTY(QDateTime indiaTime MEMBER m_indiaTime)
    Q_PROPERTY(QDateTime utcTime MEMBER m_utcTime)
    Q_PROPERTY(QString systemTimeZone READ systemTimeZone)
private:
    QDateTime m_euTime = {{2025, 5, 29}, {12, 34}, QTimeZone("Europe/Brussels")};
    QDateTime m_indiaTime = {{2025, 5, 29}, {12, 34}, QTimeZone("Asia/Kolkata")};
    QDateTime m_utcTime = {{2025, 5, 29}, {12, 34}, QTimeZone::utc()};

    [[nodiscard]] QString systemTimeZone() const
    {
        return QString::fromUtf8(QTimeZone::systemTimeZone().id());
    }
};

class KFormatQmlSetup : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine *engine)
    {
        engine->rootContext()->setContextProperty(u"_obj"_s, QVariant::fromValue(Object()));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(kformatqmltest, KFormatQmlSetup)

#include "kformatqmltest.moc"
