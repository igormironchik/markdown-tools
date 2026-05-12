/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_STORAGEACCESS_H
#define SOLID_STORAGEACCESS_H

#include <solid/solid_export.h>

#include <QVariant>
#include <solid/deviceinterface.h>
#include <solid/solidnamespace.h>

namespace Solid
{
class StorageAccessPrivate;
class Device;

/*!
 * \class Solid::StorageAccess
 * \inheaderfile Solid/StorageAccess
 * \inmodule Solid
 *
 * \brief This device interface is available on volume devices to access them
 * (i.e. mount or unmount them).
 *
 * A volume is anything that can contain data (partition, optical disc,
 * memory card). It's a particular kind of block device.
 */
class SOLID_EXPORT StorageAccess : public DeviceInterface
{
    Q_OBJECT

    /*!
     * \property Solid::StorageAccess::accessible
     */
    Q_PROPERTY(bool accessible READ isAccessible)

    /*!
     * \property Solid::StorageAccess::filePath
     */
    Q_PROPERTY(QString filePath READ filePath)

    /*!
     * \property Solid::StorageAccess::ignored
     */
    Q_PROPERTY(bool ignored READ isIgnored)

    /*!
     * \property Solid::StorageAccess::encrypted
     */
    Q_PROPERTY(bool encrypted READ isEncrypted)

    Q_DECLARE_PRIVATE(StorageAccess)
    friend class Device;

private:
    /*!
     * \internal
     *
     * Creates a new StorageAccess object.
     *
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     *
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit StorageAccess(QObject *backendObject);

public:
    ~StorageAccess() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the StorageAccess device interface.
     *
     * Returns the StorageVolume device interface type
     * \sa Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::StorageAccess;
    }

    /*!
     * Indicates if this volume is mounted.
     *
     * Returns true if the volume is mounted
     */
    bool isAccessible() const;

    /*!
     * Retrieves the absolute path of this volume mountpoint.
     *
     * Returns the absolute path to the mount point if the volume is
     * mounted, QString() otherwise
     */
    QString filePath() const;

    /*!
     * Indicates if this volume should be ignored by applications.
     *
     * If it should be ignored, it generally means that it should be
     * invisible to the user. It's useful for firmware partitions or
     * OS reinstall partitions on some systems.
     *
     * Returns true if the volume should be ignored
     */
    bool isIgnored() const;

    /*!
     * Checks if source of the storage is encrypted.
     *
     * Returns true if storage is encrypted one
     *
     * \since 5.80
     */
    bool isEncrypted() const;

    /*!
     * Mounts the volume.
     *
     * Returns false if the operation is not supported, true if the
     * operation is attempted
     */
    bool setup();

    /*!
     * Unmounts the volume.
     *
     * Returns false if the operation is not supported, true if the
     * operation is attempted
     */
    bool teardown();

    /*!
     * Indicates if this volume can check for filesystem errors.
     *
     * Returns true if the volume is can be checked
     */
    bool canCheck() const;

    /*!
     * Checks the filesystem for consistency avoiding any modifications or repairs.
     *
     * Mounted or unsupported filesystems will result in an error.
     *
     * Returns Whether the filesystem is undamaged.
     */
    bool check();

    /*!
     * Indicates if the filesystem of this volume supports repair
     * attempts. It does not indicate if such an attempt will succeed.
     *
     * Returns true if the volume is can be repaired
     */
    bool canRepair() const;

    /*!
     * Tries to repair the filesystem.
     *
     * Mounted or unsupported filesystems will result in an error.
     *
     * Returns Whether the filesystem could be successfully repaired
     */
    bool repair();

Q_SIGNALS:
    /*!
     * This signal is emitted when the accessiblity of this device
     * has changed.
     *
     * \a accessible true if the volume is accessible, false otherwise
     *
     * \a udi the UDI of the volume
     */
    void accessibilityChanged(bool accessible, const QString &udi);

    /*!
     * This signal is emitted when the attempted setting up of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * \a error type of error that occurred, if any
     *
     * \a errorData more information about the error, if any
     *
     * \a udi the UDI of the volume
     */
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    /*!
     * This signal is emitted when the attempted tearing down of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * \a error type of error that occurred, if any
     *
     * \a errorData more information about the error, if any
     *
     * \a udi the UDI of the volume
     */
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    /*!
     * This signal is emitted when a setup of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * \a udi the UDI of the volume
     */
    void setupRequested(const QString &udi);

    /*!
     * This signal is emitted when a teardown of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process
     *
     * \a udi the UDI of the volume
     */
    void teardownRequested(const QString &udi);

    /*!
     * This signal is emitted when a check of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * \a udi the UDI of the volume
     *
     * \since 6.11
     */
    void checkRequested(const QString &udi);

    /*!
     * This signal is emitted when the attempted check of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * \a error type of error that occurred, if any
     *
     * \a errorData more information about the error, if any
     *
     * \a udi the UDI of the volume
     *
     * \since 6.11
     */
    void checkDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    /*!
     * This signal is emitted when a repair of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * \a udi the UDI of the volume
     */
    void repairRequested(const QString &udi);

    /*!
     * This signal is emitted when the attempted repaired of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * \a error type of error that occurred, if any
     *
     * \a errorData more information about the error, if any
     *
     * \a udi the UDI of the volume
     */
    void repairDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

protected:
    SOLID_NO_EXPORT StorageAccess(StorageAccessPrivate &dd, QObject *backendObject);
};
}

#endif
