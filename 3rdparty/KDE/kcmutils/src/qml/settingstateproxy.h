/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2020 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SETTINGSTATEPROXY_H
#define SETTINGSTATEPROXY_H

#include <QObject>
#include <QPointer>
#include <QQmlEngine>

#include <KCoreConfigSkeleton>

/*!
 * \qmltype SettingStateProxy
 * \inqmlmodule org.kde.kcmutils
 * \brief Allows to represent the state of a particular setting
 * in a config object in a declarative way.
 *
 * \since 5.73
 */
class SettingStateProxy : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /*!
     * \qmlproperty bool SettingStateProxy::configObject
     * \brief The config object which will be monitored for setting state changes.
     */
    Q_PROPERTY(KCoreConfigSkeleton *configObject READ configObject WRITE setConfigObject NOTIFY configObjectChanged)

    /*!
     * \qmlproperty bool SettingStateProxy::settingName
     * \brief The name of the setting in the config object.
     */
    Q_PROPERTY(QString settingName READ settingName WRITE setSettingName NOTIFY settingNameChanged)

    /*!
     * \qmlproperty bool SettingStateProxy::immutable
     * \brief Indicates if the setting is marked as immutable.
     */
    Q_PROPERTY(bool immutable READ isImmutable NOTIFY immutableChanged)

    /*!
     * \qmlproperty bool SettingStateProxy::defaulted
     * \brief Indicates if the setting differs from its default value.
     */
    Q_PROPERTY(bool defaulted READ isDefaulted NOTIFY defaultedChanged)

public:
    using QObject::QObject;

    KCoreConfigSkeleton *configObject() const;
    void setConfigObject(KCoreConfigSkeleton *configObject);

    QString settingName() const;
    void setSettingName(const QString &settingName);

    bool isImmutable() const;
    bool isDefaulted() const;

Q_SIGNALS:
    void configObjectChanged();
    void settingNameChanged();

    void immutableChanged();
    void defaultedChanged();

private Q_SLOTS:
    /*!
     * \qmlmethod void SettingStateProxy::updateState()
     */
    void updateState();

private:
    void connectSetting();

    QPointer<KCoreConfigSkeleton> m_configObject;
    QString m_settingName;
    bool m_immutable = false;
    bool m_defaulted = true;
};

#endif
