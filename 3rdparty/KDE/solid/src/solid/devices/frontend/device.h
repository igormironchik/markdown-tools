/*
    SPDX-FileCopyrightText: 2005-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DEVICE_H
#define SOLID_DEVICE_H

#include <QList>
#include <QSharedData>

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class DevicePrivate;

/*!
 * \class Solid::Device
 * \inheaderfile Solid/Device
 * \inmodule Solid
 *
 * \brief This class allows applications to deal with devices available in the
 * underlying system.
 *
 * Device stores a reference to device data provided by the backend.
 * Device objects are designed to be used by value. Copying these objects
 * is quite cheap, so using pointers to the me is generally not needed.
 */
class SOLID_EXPORT Device
{
public:
    /*!
     * Retrieves all the devices available in the underlying system.
     *
     * Returns the list of the devices available
     */
    static QList<Device> allDevices();

    /*!
     * Retrieves a list of devices of the system given matching the given
     * constraints (parent and device interface type)
     *
     * \a type device interface type available on the devices we're looking for, or DeviceInterface::Unknown
     * if there's no constraint on the device interfaces
     *
     * \a parentUdi UDI of the parent of the devices we're searching for, or QString()
     * if there's no constraint on the parent
     *
     * Returns the list of devices corresponding to the given constraints
     * \sa Solid::Predicate
     */
    static QList<Device> listFromType(const DeviceInterface::Type &type, const QString &parentUdi = QString());

    /*!
     * Retrieves a list of devices of the system given matching the given
     * constraints (parent and predicate)
     *
     * \a predicate Predicate that the devices we're searching for must verify
     *
     * \a parentUdi UDI of the parent of the devices we're searching for, or QString()
     * if there's no constraint on the parent
     *
     * Returns the list of devices corresponding to the given constraints
     * \sa Solid::Predicate
     */
    static QList<Device> listFromQuery(const Predicate &predicate, const QString &parentUdi = QString());

    /*!
     * Convenience function see above.
     *
     * Returns the list of devices
     */
    static QList<Device> listFromQuery(const QString &predicate, const QString &parentUdi = QString());

    /*!
     * Returns the Device containing the filesystem for the given path
     *
     * \a path is the canonical path to a filesystem entry, e.g. a file or directory
     *
     * Returns a Device containing the given \a path.
     *
     * For Devices implementing the
     * StorageVolume interface only ones matching UsageType::FileSystem are
     * returned, i.e. no backing encrypted volumes.
     * \since 5.73
     * \sa QFileInfo::canonicalFilePath
     */
    static Device storageAccessFromPath(const QString &path);

    /*!
     * Constructs a device for a given Universal Device Identifier (UDI).
     *
     * \a udi the udi of the device to create
     */
    explicit Device(const QString &udi = QString());

    Device(const Device &device);

    ~Device();

    Device &operator=(const Device &device);

    /*!
     * Indicates if this device is valid.
     *
     * A device is considered valid if it's available in the system.
     *
     * Returns \c true if this device is available, \c false otherwise
     */
    bool isValid() const;

    /*!
     * Retrieves the Universal Device Identifier (UDI).
     *
     * \warning Don't use the UDI for anything except communication with Solid. Also don't store
     * UDIs as there's no guarantee that the UDI stays the same when the hardware setup changed.
     * The UDI is a unique identifier that is local to the computer in question and for the
     * current boot session. The UDIs may change after a reboot.
     * Similar hardware in other computers may have different values; different
     * hardware could have the same UDI.
     *
     * Returns the udi of the device
     */
    QString udi() const;

    /*!
     * Retrieves the Universal Device Identifier (UDI)
     * of the Device's parent.
     *
     * Returns the udi of the device's parent
     */
    QString parentUdi() const;

    /*!
     * Retrieves the parent of the Device.
     *
     * Returns the device's parent
     * \sa parentUdi()
     */
    Device parent() const;

    /*!
     * Retrieves the name of the device vendor.
     *
     * Returns the vendor name
     */
    QString vendor() const;

    /*!
     * Retrieves the name of the product corresponding to this device.
     *
     * Returns the product name
     */
    QString product() const;

    /*!
     * Retrieves the name of the icon representing this device.
     * The naming follows the freedesktop.org specification.
     *
     * Returns the icon name
     */
    QString icon() const;

    /*!
     * Retrieves the names of the emblems representing the state of this device.
     * The naming follows the freedesktop.org specification.
     *
     * Returns the emblem names
     * \since 4.4
     */
    QStringList emblems() const;

    /*!
     * Retrieves the display Name to use for this device.
     * Same as description when not defined.
     *
     * Returns the display Name
     * \since 5.71
     */
    QString displayName() const;

    /*!
     * Retrieves the description of device.
     *
     * Returns the description
     * \since 4.4
     */
    QString description() const;

    /*!
     * Tests if a device interface is available from the device.
     *
     * \a type the device interface type to query
     *
     * Returns true if the device interface is available, false otherwise
     */
    bool isDeviceInterface(const DeviceInterface::Type &type) const;

    /*!
     * Retrieves a specialized interface to interact with the device corresponding to
     * a particular device interface.
     *
     * \a type the device interface type
     *
     * Returns a pointer to the device interface interface if it exists, 0 otherwise
     */
    DeviceInterface *asDeviceInterface(const DeviceInterface::Type &type);

    /*!
     * Retrieves a specialized interface to interact with the device corresponding to
     * a particular device interface.
     *
     * \a type the device interface type
     *
     * Returns a pointer to the device interface interface if it exists, 0 otherwise
     */
    const DeviceInterface *asDeviceInterface(const DeviceInterface::Type &type) const;

    /*!
     * Retrieves a specialized interface to interact with the device corresponding
     * to a given device interface.
     *
     * Returns a pointer to the device interface if it exists, 0 otherwise
     */
    template<class DevIface>
    DevIface *as()
    {
        DeviceInterface::Type type = DevIface::deviceInterfaceType();
        DeviceInterface *iface = asDeviceInterface(type);
        return qobject_cast<DevIface *>(iface);
    }

    /*!
     * Retrieves a specialized interface to interact with the device corresponding
     * to a given device interface.
     *
     * Returns a pointer to the device interface if it exists, 0 otherwise
     */
    template<class DevIface>
    const DevIface *as() const
    {
        DeviceInterface::Type type = DevIface::deviceInterfaceType();
        const DeviceInterface *iface = asDeviceInterface(type);
        return qobject_cast<const DevIface *>(iface);
    }

    /*!
     * Tests if a device provides a given device interface.
     *
     * Returns true if the interface is available, false otherwise
     */
    template<class DevIface>
    bool is() const
    {
        return isDeviceInterface(DevIface::deviceInterfaceType());
    }

private:
    QExplicitlySharedDataPointer<DevicePrivate> d;
    friend class DeviceManagerPrivate;
};
}

Q_DECLARE_TYPEINFO(Solid::Device, Q_RELOCATABLE_TYPE);

#endif
