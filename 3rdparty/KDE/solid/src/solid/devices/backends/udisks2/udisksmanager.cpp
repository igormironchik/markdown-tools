/*
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksmanager.h"
#include "dbus/manager.h"
#include "udisks_debug.h"
#include "udisksopticaldisc.h"
#include "udisksutils.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDomDocument>

#include <algorithm>

#include "../shared/rootdevice.h"
#include "solid/genericinterface.h"

using namespace Solid::Backends::UDisks2;
using namespace Solid::Backends::Shared;

Manager::Manager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent)
    , m_manager(org::freedesktop::DBus::ObjectManager(QStringLiteral(UD2_DBUS_SERVICE), QStringLiteral(UD2_DBUS_PATH), QDBusConnection::systemBus()))
{
    m_supportedInterfaces = {
        Solid::DeviceInterface::GenericInterface,
        Solid::DeviceInterface::Block,
        Solid::DeviceInterface::StorageAccess,
        Solid::DeviceInterface::StorageDrive,
        Solid::DeviceInterface::OpticalDrive,
        Solid::DeviceInterface::OpticalDisc,
        Solid::DeviceInterface::StorageVolume,
    };

    qDBusRegisterMetaType<QList<QDBusObjectPath>>();
    qDBusRegisterMetaType<QVariantMap>();
    qDBusRegisterMetaType<VariantMapMap>();
    qDBusRegisterMetaType<DBUSManagerStruct>();

    connect(&m_manager, &org::freedesktop::DBus::ObjectManager::InterfacesAdded, this, &Manager::slotInterfacesAdded);
    connect(&m_manager, &org::freedesktop::DBus::ObjectManager::InterfacesRemoved, this, &Manager::slotInterfacesRemoved);

    QDBusConnection::systemBus()
        .connect(QStringLiteral(UD2_DBUS_SERVICE), QString() /*any path*/, QStringLiteral(DBUS_INTERFACE_PROPS), QStringLiteral("PropertiesChanged"), this, SLOT(slotPropertiesChanged(QDBusMessage)));
}

Manager::~Manager()
{
}

bool Manager::hasInterface(const QString &udi, const QString &interface)
{
    return deviceCache().value(udi).contains(interface);
}

QMap<QString, PropertyMap> Manager::allProperties()
{
    return deviceCache();
}

PropertyMap Manager::deviceProperties(const QString &udi)
{
    return deviceCache().value(udi);
}

QVariant Manager::deviceProperty(const QString &udi, const QString &name, Manager::FetchMode fetchMode)
{
    const auto props = deviceProperties(udi);

    // Loop through all interfaces looking for a property.
    for (auto it = props.begin(), end = props.end(); it != end; ++it) {
        const QString iface = it.key();
        const auto valueIt = it->constFind(name);
        if (valueIt != it->constEnd()) {
            if (!valueIt->isValid() && fetchMode == FetchIfNeeded) {
                QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE), udi, QStringLiteral(DBUS_INTERFACE_PROPS), QStringLiteral("Get"));
                call.setArguments({iface, name});
                QDBusReply<QVariant> reply = QDBusConnection::systemBus().call(call);

                /* We don't check for error here and store the item in the cache anyway so next time we don't have to
                 * do the DBus call to find out it does not exist but just check whether
                 * prop(key).isValid() */
                const QVariant value = Utils::sanitizeValue(reply.value());
                m_cache[udi][iface][name] = value;
                return value;
            }

            return *valueIt;
        }
    }

    return QVariant();
}

std::unique_ptr<QObject> Manager::createDevice(const QString &udi)
{
    if (udi == udiPrefix()) {
        std::unique_ptr<RootDevice> root = std::make_unique<RootDevice>(udi);

        root->setProduct(tr("Storage"));
        root->setDescription(tr("Storage devices"));
        root->setIcon(QStringLiteral("server-database")); // Obviously wasn't meant for that, but maps nicely in oxygen icon set :-p

        return root;
    } else if (deviceCache().contains(udi)) {
        return std::make_unique<Device>(this, udi);
    } else {
        return nullptr;
    }
}

QStringList Manager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList result;

    const auto devices = deviceCache();

    if (!parentUdi.isEmpty()) {
        for (auto it = devices.keyBegin(), end = devices.keyEnd(); it != end; ++it) {
            Device device(this, *it);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi) {
                result << *it;
            }
        }

        return result;
    } else if (type != Solid::DeviceInterface::Unknown) {
        for (auto it = devices.keyBegin(), end = devices.keyEnd(); it != end; ++it) {
            Device device(this, *it);
            if (device.queryDeviceInterface(type)) {
                result << *it;
            }
        }

        return result;
    }

    return devices.keys();
}

QStringList Manager::allDevices()
{
    m_devices.clear();
    m_cache.clear();

    org::freedesktop::DBus::ObjectManager manager(QStringLiteral(UD2_DBUS_SERVICE), QStringLiteral(UD2_DBUS_PATH), QDBusConnection::systemBus());
    QDBusPendingReply<DBUSManagerStruct> reply = manager.GetManagedObjects();
    reply.waitForFinished();
    if (reply.isError()) {
        qCWarning(UDISKS2) << "Failed to fetch all devices" << reply.error().name() << reply.error().message();
        return m_devices;
    }

    const auto items = reply.value();
    for (auto it = items.begin(), end = items.end(); it != end; ++it) {
        const QString udi = it.key().path();

        if (!udi.startsWith(QLatin1String(UD2_DBUS_PATH_BLOCKDEVICES)) && !udi.startsWith(QLatin1String(UD2_DBUS_PATH_DRIVES))) {
            continue;
        }

        VariantMapMap mapMap = it.value();
        for (QVariantMap &map : mapMap) {
            map = Utils::sanitizeValue(map);
        }
        m_devices.append(udi);
        m_cache.insert(udi, mapMap);
    }

    // Filter out empty optical drives.
    m_devices.erase(std::remove_if(m_devices.begin(),
                                   m_devices.end(),
                                   [this](const QString &udi) {
                                       Device device(this, udi);
                                       return device.mightBeOpticalDisc() && !device.isOpticalDisc();
                                   }),
                    m_devices.end());

    return m_devices;
}

QSet<Solid::DeviceInterface::Type> Manager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QString Manager::udiPrefix() const
{
    return QStringLiteral(UD2_UDI_DISKS_PREFIX);
}

void Manager::slotInterfacesAdded(const QDBusObjectPath &object_path, const VariantMapMap &interfaces_and_properties)
{
    const QString udi = object_path.path();

    /* Ignore jobs */
    if (udi.startsWith(QStringLiteral(UD2_DBUS_PATH_JOBS))) {
        return;
    }

    qCDebug(UDISKS2) << udi << "has new interfaces:" << interfaces_and_properties.keys();

    auto cachedIt = m_cache.find(udi);
    if (cachedIt == m_cache.end()) {
        cachedIt = m_cache.insert(udi, VariantMapMap{});
    }

    // We need to re-fetch all existing interfaces to ensure by the time we emit "add" for FileSystem
    // the rest is up to date (e.g. if Loop gets updated after we gained FileSystem) some propertes aren't updated yet.
    // We'll skip Block as every device we are interested in will be a Block device.
    QStringList oldInterfaces = cachedIt->keys();
    oldInterfaces.removeOne(QStringLiteral(UD2_DBUS_INTERFACE_BLOCK));

    for (auto it = interfaces_and_properties.begin(), end = interfaces_and_properties.end(); it != end; ++it) {
        // Filters generic DBus interfaces.
        if (!it.key().startsWith(QLatin1String(UD2_DBUS_SERVICE))) {
            continue;
        }
        cachedIt->insert(it.key(), Utils::sanitizeValue(it.value()));
    }

    for (const QString &interface : oldInterfaces) {
        QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE), udi, QStringLiteral(DBUS_INTERFACE_PROPS), QStringLiteral("GetAll"));
        call.setArguments({interface});
        QDBusReply<QVariantMap> reply = QDBusConnection::systemBus().call(call);
        if (reply.isValid()) {
            cachedIt->insert(interface, Utils::sanitizeValue(reply.value()));
        }
    }

    bool isNewDevice = !m_devices.contains(udi);
    if (isNewDevice) {
        // Check if it is an empty optical drive, and if so, ignore it.
        Device device(this, udi);
        if (device.mightBeOpticalDisc() && !device.isOpticalDisc()) {
            qCDebug(UDISKS2) << "\tIt's a new empty optical drive, ignoring";
            isNewDevice = false;
        } else {
            qCDebug(UDISKS2) << "\tIt's a new device, emitting added";
            m_devices.append(udi);
        }
    }

    if (isNewDevice) {
        Q_EMIT deviceAdded(udi);
    } else if (interfaces_and_properties.contains(QLatin1String(UD2_DBUS_INTERFACE_FILESYSTEM))) {
        // re-emit in case of 2-stage devices like N9 or some Android phones
        Q_EMIT deviceRemoved(udi);
        Q_EMIT deviceAdded(udi);
    }

    // TODO invalidate drive? updateBackend did that
}

void Manager::slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    const QString udi = object_path.path();
    if (udi.isEmpty()) {
        return;
    }

    /* Ignore jobs */
    if (udi.startsWith(QStringLiteral(UD2_DBUS_PATH_JOBS))) {
        return;
    }

    auto cachedIt = m_cache.find(udi);
    if (cachedIt == m_cache.end()) {
        return;
    }

    qCDebug(UDISKS2) << udi << "lost interfaces:" << interfaces;

    for (const QString &iface : interfaces) {
        cachedIt->remove(iface);
    }

    /*
     * Determine left interfaces. The device backend may have processed the
     * InterfacesRemoved signal already, but the result set is the same
     * independent if the backend or the manager processes the signal first.
     */
    if (cachedIt->isEmpty()) {
        qCDebug(UDISKS2) << "\tThere are no more interface, emitting device removal";
        Q_EMIT deviceRemoved(udi);
        m_cache.remove(udi);
        m_devices.removeOne(udi);
    } else {
        /*
         * Changes in the interface composition may change if a device
         * matches a Predicate. We have to do a remove-and-readd cycle
         * as there is no dedicated signal for Predicate reevaluation.
         */
        Q_EMIT deviceRemoved(udi);
        Q_EMIT deviceAdded(udi);
    }
}

void Manager::slotPropertiesChanged(const QDBusMessage &msg)
{
    const QString udi = msg.path();

    if (udi.isEmpty() || !udi.startsWith(QLatin1String(UD2_UDI_DISKS_PREFIX)) || udi.startsWith(QLatin1String(UD2_DBUS_PATH_JOBS))) {
        return;
    }

    const auto args = msg.arguments();
    if (Q_UNLIKELY(args.size() != 3)) {
        return;
    }

    const QString iface = qdbus_cast<QString>(args.at(0));
    const QVariantMap changed = qdbus_cast<QVariantMap>(args.at(1));
    const QStringList invalidated = qdbus_cast<QStringList>(args.at(2));

    auto cachedIt = m_cache.find(udi);
    if (cachedIt == m_cache.end()) {
        return;
    }

    const bool knownDevice = m_devices.contains(udi);

    // Update cache of internal devices even if we don't advertise them at this time.
    QMap<QString, int> changeMap;

    for (const QString &prop : invalidated) {
        // Invalid QVariant() marks property that exists but needs to be fetched first.
        (*cachedIt)[iface].insert(prop, QVariant());
        changeMap.insert(prop, Solid::GenericInterface::PropertyModified);
    }

    for (auto it = changed.begin(), end = changed.end(); it != end; ++it) {
        (*cachedIt)[iface].insert(it.key(), Utils::sanitizeValue(it.value()));
        changeMap.insert(it.key(), Solid::GenericInterface::PropertyModified);
    }

    // Only announce the change if the device is advertised.
    if (knownDevice && !changeMap.isEmpty()) {
        Q_EMIT propertyChanged(udi, changeMap);
    }

    // Special handling for optical media insertion/removal.
    if (iface == QLatin1String(UD2_DBUS_INTERFACE_BLOCK) && (changed.contains(QStringLiteral("Size")) || invalidated.contains(QStringLiteral("Size")))) {
        qulonglong size = deviceProperty(udi, QStringLiteral("Size")).toULongLong();

        const bool mediaInserted = !knownDevice && size > 0;
        const bool mediaRemoved = knownDevice && size == 0;

        if (mediaInserted || mediaRemoved) {
            Device device(this, udi);
            if (device.mightBeOpticalDisc()) {
                if (!knownDevice && size > 0) { // we don't know the optical disc, got inserted.
                    const OpticalDisc disc(&device);
                    // If it is a data disc, wait for its FileSystem interface to be announced instead,
                    // otherwise we'll add this disc twice. But if we don't add it here, we will miss
                    // Audio CDs that will never have a FileSystem interface.
                    if (!disc.availableContent().testFlag(Solid::OpticalDisc::Data)) {
                        m_devices.append(udi);
                        Q_EMIT deviceAdded(udi);
                    }
                } else if (knownDevice && size == 0) { // we know the optical disc, got removed.
                    Q_EMIT deviceRemoved(udi);
                    // Keeping the cache as we never fetch all device properties again after the initial query.
                    m_devices.removeOne(udi);
                }
            }
        }
    }

    // TODO invalidate drive? updateBackend did that
}

QMap<QString, PropertyMap> Manager::deviceCache()
{
    if (m_cache.isEmpty()) {
        allDevices();
    }

    return m_cache;
}

#include "moc_udisksmanager.cpp"
