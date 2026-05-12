/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DEVICEINTERFACE_H
#define SOLID_DEVICEINTERFACE_H

#include <QObject>

#include <solid/solid_export.h>

namespace Solid
{
class Device;
class DevicePrivate;
class Predicate;
class DeviceInterfacePrivate;

/*!
 * \class Solid::DeviceInterface
 * \inheaderfile Solid/DeviceInterface
 * \inmodule Solid
 *
 * \brief Base class of all the device interfaces.
 *
 * A device interface describes what a device can do. A device generally has
 * a set of device interfaces.
 */
class SOLID_EXPORT DeviceInterface : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DeviceInterface)

public:
    /*!
     * This enum type defines the type of device interface that a Device can have.
     *
     * \value Unknown An undetermined device interface
     * \value GenericInterface
     * \value Processor A processor
     * \value Block A block device
     * \value StorageAccess A mechanism to access data on a storage device
     * \value StorageDrive A storage drive
     * \value OpticalDrive An optical drive (CD-ROM, DVD, ...)
     * \value StorageVolume A volume
     * \value OpticalDisc An optical disc
     * \value Camera A digital camera
     * \value PortableMediaPlayer A portable media player
     * \value Battery
     * \value NetworkShare A network share interface
     * \omitvalue Last
     */
    enum Type {
        Unknown = 0,
        GenericInterface = 1,
        Processor = 2,
        Block = 3,
        StorageAccess = 4,
        StorageDrive = 5,
        OpticalDrive = 6,
        StorageVolume = 7,
        OpticalDisc = 8,
        Camera = 9,
        PortableMediaPlayer = 10,
        Battery = 12,
        NetworkShare = 14,
        Last = 0xffff,
    };
    Q_ENUM(Type)

    ~DeviceInterface() override;

    /*!
     * Indicates if this device interface is valid.
     *
     * A device interface is considered valid if the device it is referring is available in the system.
     *
     * Returns \c true if this device interface's device is available, \c false otherwise
     */
    bool isValid() const;

    /*!
     * Returns the class name of the device interface type
     */
    static QString typeToString(Type type);

    /*!
     * Returns the device interface type for the given class name
     */
    static Type stringToType(const QString &type);

    /*!
     * Returns a description suitable to display in the UI of the device interface type
     * \since 4.4
     */
    static QString typeDescription(Type type);

protected:
    /*!
     * \internal
     * Creates a new DeviceInterface object.
     *
     * \a dd the private d member. It will take care of deleting it upon destruction.
     *
     * \a backendObject the device interface object provided by the backend
     */
    SOLID_NO_EXPORT DeviceInterface(DeviceInterfacePrivate &dd, QObject *backendObject);

    DeviceInterfacePrivate *d_ptr;

private:
    friend class Device;
    friend class DevicePrivate;
};
}

#endif
