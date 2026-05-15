/*
    SPDX-FileCopyrightText: 2005-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "devicemanager_p.h" //krazy:exclude=includes (devicenotifier.h is the header file for this class)
#include "devicenotifier.h"

#include "device.h"
#include "device_p.h"
#include "predicate.h"
#include "storageaccess.h"
#include "storagevolume.h"

#include "ifaces/device.h"
#include "ifaces/devicemanager.h"

#include <set>

Q_GLOBAL_STATIC(Solid::DeviceManagerStorage, globalDeviceStorage)

Solid::DeviceManagerPrivate::DeviceManagerPrivate()
    : m_nullDevice(new DevicePrivate(QString()))
{
    loadBackends();

    const auto backends = managerBackends();
    for (const auto &backend : backends) {
        connect(backend, &Solid::Ifaces::DeviceManager::deviceAdded, this, &Solid::DeviceManagerPrivate::_k_deviceAdded);
        connect(backend, &Solid::Ifaces::DeviceManager::deviceRemoved, this, &Solid::DeviceManagerPrivate::_k_deviceRemoved);
    }
}

Solid::DeviceManagerPrivate::~DeviceManagerPrivate()
{
    const auto backends = managerBackends();
    for (const auto &backend : backends) {
        disconnect(backend, &Solid::Ifaces::DeviceManager::deviceAdded, this, &Solid::DeviceManagerPrivate::_k_deviceAdded);
        disconnect(backend, &Solid::Ifaces::DeviceManager::deviceRemoved, this, &Solid::DeviceManagerPrivate::_k_deviceRemoved);
    }

    // take a copy as m_devicesMap is changed when DevicePrivate is destroyed
    const auto deviceMap = m_devicesMap;
    for (QPointer<DevicePrivate> dev : deviceMap) {
        if (!dev.data()->ref.deref()) {
            delete dev.data();
        }
    }

    m_devicesMap.clear();
}

QList<Solid::Device> Solid::Device::allDevices()
{
    QList<Device> list;
    const auto backends = globalDeviceStorage->managerBackends();

    for (const auto &backend : backends) {
        const auto udis = backend->allDevices();
        for (const auto &udi : udis) {
            list.append(Device(udi));
        }
    }

    return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const QString &predicate, const QString &parentUdi)
{
    Predicate p = Predicate::fromString(predicate);

    if (p.isValid()) {
        return listFromQuery(p, parentUdi);
    } else {
        return QList<Device>();
    }
}

QList<Solid::Device> Solid::Device::listFromType(const DeviceInterface::Type &type, const QString &parentUdi)
{
    QList<Device> list;
    const auto backends = globalDeviceStorage->managerBackends();

    for (const auto &backend : backends) {
        if (!backend->supportedInterfaces().contains(type)) {
            continue;
        }

        const auto udis = backend->devicesFromQuery(parentUdi, type);
        for (const auto &udi : udis) {
            list.append(Device(udi));
        }
    }

    return list;
}

QList<Solid::Device> Solid::Device::listFromQuery(const Predicate &predicate, const QString &parentUdi)
{
    QList<Device> list;
    const auto usedTypes = predicate.usedTypes();
    const auto backends = globalDeviceStorage->managerBackends();

    for (const auto &backend : backends) {
        QStringList udis;
        if (predicate.isValid()) {
            auto supportedTypes = backend->supportedInterfaces();
            if (supportedTypes.intersect(usedTypes).isEmpty()) {
                continue;
            }

            auto sortedTypes = supportedTypes.values();
            std::sort(sortedTypes.begin(), sortedTypes.end());
            for (const auto &type : std::as_const(sortedTypes)) {
                udis += backend->devicesFromQuery(parentUdi, type);
            }
        } else {
            udis += backend->allDevices();
        }

        std::set<QString> seen;
        for (const auto &udi : std::as_const(udis)) {
            const auto [it, isInserted] = seen.insert(udi);
            if (!isInserted) {
                continue;
            }
            const Device dev(udi);

            bool matches = false;

            if (!predicate.isValid()) {
                matches = true;
            } else {
                matches = predicate.matches(dev);
            }

            if (matches) {
                list.append(dev);
            }
        }
    }

    return list;
}

Solid::Device Solid::Device::storageAccessFromPath(const QString &path)
{
    const QList<Device> list = Solid::Device::listFromType(DeviceInterface::Type::StorageAccess);
    Device match;
    int match_length = 0;
    for (const Device &device : list) {
        auto storageVolume = device.as<StorageVolume>();
        if (storageVolume && storageVolume->usage() != StorageVolume::UsageType::FileSystem) {
            continue;
        }

        auto storageAccess = device.as<StorageAccess>();

        if (!storageAccess) {
            continue;
        }

        QString mountPath = storageAccess->filePath();

        if (mountPath.size() <= match_length || !path.startsWith(mountPath)) {
            continue;
        }

        const auto realLength = mountPath.back() == QLatin1Char('/') ? mountPath.size() - 1 : mountPath.size();

        // `startsWith` implies `path.size() >= mountPath.size()`
        if (path.size() == realLength || path[realLength] == QLatin1Char('/')) {
            match_length = realLength;
            match = device;
        }
    }
    return match;
}

Solid::DeviceNotifier *Solid::DeviceNotifier::instance()
{
    return globalDeviceStorage->notifier();
}

void Solid::DeviceManagerPrivate::_k_deviceAdded(const QString &udi)
{
    if (m_devicesMap.contains(udi)) {
        DevicePrivate *dev = m_devicesMap[udi].data();

        // OK, this one was requested somewhere was invalid
        // and now becomes magically valid!

        if (dev && dev->backendObject() == nullptr) {
            dev->setBackendObject(createBackendObject(udi));
            Q_ASSERT(dev->backendObject() != nullptr);
        }
    }

    Q_EMIT deviceAdded(udi);
}

void Solid::DeviceManagerPrivate::_k_deviceRemoved(const QString &udi)
{
    if (m_devicesMap.contains(udi)) {
        DevicePrivate *dev = m_devicesMap[udi].data();

        // OK, this one was requested somewhere was valid
        // and now becomes magically invalid!

        if (dev) {
            dev->setBackendObject(nullptr);
        }
    }

    Q_EMIT deviceRemoved(udi);
}

Solid::DevicePrivate *Solid::DeviceManagerPrivate::findRegisteredDevice(const QString &udi)
{
    if (udi.isEmpty()) {
        return m_nullDevice.data();
    } else if (m_devicesMap.contains(udi)) {
        return m_devicesMap[udi].data();
    } else {
        DevicePrivate *devData = new DevicePrivate(udi);
        devData->setBackendObject(createBackendObject(udi));

        m_devicesMap[udi] = devData;
        m_reverseMap[devData] = udi;

        connect(devData, &QObject::destroyed, this, [this, devData] {
            QString udi = m_reverseMap.take(devData);

            if (!udi.isEmpty()) {
                m_devicesMap.remove(udi);
            }
        });

        return devData;
    }
}

std::unique_ptr<Solid::Ifaces::Device> Solid::DeviceManagerPrivate::createBackendObject(const QString &udi)
{
    const auto backends = globalDeviceStorage->managerBackends();

    for (const auto &backend : backends) {
        if (!udi.startsWith(backend->udiPrefix())) {
            continue;
        }

        Ifaces::Device *iface = nullptr;

        std::unique_ptr<QObject> object = backend->createDevice(udi);
        iface = qobject_cast<Ifaces::Device *>(object.get());

        if (iface != nullptr) {
            return std::unique_ptr<Solid::Ifaces::Device>(qobject_cast<Ifaces::Device *>(object.release()));
        }
    }

    return nullptr;
}

Solid::DeviceManagerStorage::DeviceManagerStorage()
{
}

QList<Solid::Ifaces::DeviceManager *> Solid::DeviceManagerStorage::managerBackends()
{
    ensureManagerCreated();
    return m_storage.localData()->managerBackends();
}

Solid::DeviceNotifier *Solid::DeviceManagerStorage::notifier()
{
    ensureManagerCreated();
    return m_storage.localData();
}

void Solid::DeviceManagerStorage::ensureManagerCreated()
{
    if (!m_storage.hasLocalData()) {
        m_storage.setLocalData(new DeviceManagerPrivate());
    }
}

#include "moc_devicemanager_p.cpp"
#include "moc_devicenotifier.cpp"
