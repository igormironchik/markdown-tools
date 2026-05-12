/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BLOCK_H
#define SOLID_BLOCK_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class BlockPrivate;
class Device;

/*!
 * \class Solid::Block
 * \inheaderfile Solid/Block
 * \inmodule Solid
 *
 * \brief This device interface is available on block devices.
 *
 * A block device is an addressable device such as drive or partition.
 * It is possible to interact with such a device using a special file
 * in the system.
 */
class SOLID_EXPORT Block : public DeviceInterface
{
    Q_OBJECT

    /*!
     * \property Solid::Block::major
     */
    Q_PROPERTY(int major READ deviceMajor)

    /*!
     * \property Solid::Block::minor
     */
    Q_PROPERTY(int minor READ deviceMinor)

    /*!
     * \property Solid::Block::device
     */
    Q_PROPERTY(QString device READ device)

    /*!
     * \property Solid::Block::isSystem
     */
    Q_PROPERTY(bool isSystem READ isSystem)

    Q_DECLARE_PRIVATE(Block)
    friend class Device;

private:
    /*!
     * \internal
     *
     * Creates a new Block object.
     *
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     */
    SOLID_NO_EXPORT explicit Block(QObject *backendObject);

public:
    ~Block() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the Block device interface.
     *
     * Returns the Block device interface type
     * \sa Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::Block;
    }

    /*!
     * Retrieves the major number of the node file to interact with
     * the device.
     *
     * Returns the device major number
     */
    int deviceMajor() const;

    /*!
     * Retrieves the minor number of the node file to interact with
     * the device.
     *
     * Returns the device minor number
     */
    int deviceMinor() const;

    /*!
     * Retrieves the absolute path of the special file to interact
     * with the device.
     *
     * Returns the absolute path of the special file to interact with
     * the device
     */
    QString device() const;

    /*!
     * Whether this block is considered a system block,
     * that it requires additional permissions to access (mount/umount)
     *
     * It defaults to false, if the backend does not support it.
     *
     * \since 6.15
     */
    bool isSystem() const;
};
}

#endif
