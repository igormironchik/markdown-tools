/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDISKSMANAGER_H
#define UDISKSMANAGER_H

#include "dbus/manager.h"
#include "udisks2.h"
#include "udisksdevice.h"

#include <solid/devices/ifaces/devicemanager.h>

#include <QSet>

using PropertyMap = QMap<QString, QVariantMap>;

namespace Solid
{
namespace Backends
{
namespace UDisks2
{
class Manager : public Solid::Ifaces::DeviceManager
{
    Q_OBJECT

public:
    enum FetchMode {
        FetchIfNeeded,
        CachedOnly,
    };

    Manager(QObject *parent);
    std::unique_ptr<QObject> createDevice(const QString &udi) override;
    QStringList devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type) override;
    QStringList allDevices() override;
    QSet<Solid::DeviceInterface::Type> supportedInterfaces() const override;
    QString udiPrefix() const override;
    ~Manager() override;

    bool hasInterface(const QString &udi, const QString &interface);
    QMap<QString, PropertyMap> allProperties();
    PropertyMap deviceProperties(const QString &udi);
    QVariant deviceProperty(const QString &udi, const QString &name, FetchMode fetchMode = FetchIfNeeded);

Q_SIGNALS:
    void propertyChanged(const QString &udi, const QMap<QString, int> &changes);

private Q_SLOTS:
    void slotInterfacesAdded(const QDBusObjectPath &object_path, const VariantMapMap &interfaces_and_properties);
    void slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void slotPropertiesChanged(const QDBusMessage &msg);

private:
    QMap<QString, PropertyMap> deviceCache();
    void updateBackend(const QString &udi);
    QSet<Solid::DeviceInterface::Type> m_supportedInterfaces;
    org::freedesktop::DBus::ObjectManager m_manager;

    // Optical media can be removed from the list of "known" devices
    // but we still want to cache their properties for when the media is inserted back.
    QStringList m_devices;
    QMap<QString, PropertyMap> m_cache;
};

}
}
}
#endif // UDISKSMANAGER_H
