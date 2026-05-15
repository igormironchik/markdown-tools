/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_PORTABLEMEDIAPLAYER_H
#define SOLID_PORTABLEMEDIAPLAYER_H

#include <QStringList>
#include <QVariant>

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class PortableMediaPlayerPrivate;
class Device;

/*!
 * \class Solid::PortableMediaPlayer
 * \inheaderfile Solid/PortableMediaPlayer
 * \inmodule Solid
 *
 * \brief This class implements Portable Media Player device interface and represents
 * a portable media player attached to the system.
 *
 * A portable media player is a portable device able to play multimedia files.
 *
 * Some of them have even recording capabilities.
 */
class SOLID_EXPORT PortableMediaPlayer : public DeviceInterface
{
    Q_OBJECT

    /*!
     * \property Solid::PortableMediaPlayer::supportedProtocols
     */
    Q_PROPERTY(QStringList supportedProtocols READ supportedProtocols)

    /*!
     * \property Solid::PortableMediaPlayer::supportedDrivers
     */
    Q_PROPERTY(QStringList supportedDrivers READ supportedDrivers)

    Q_DECLARE_PRIVATE(PortableMediaPlayer)
    friend class Device;

public:
private:
    /*!
     * \internal
     * Creates a new PortableMediaPlayer object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit PortableMediaPlayer(QObject *backendObject);

public:
    ~PortableMediaPlayer() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the PortableMediaPlayer device interface.
     *
     * Returns the PortableMediaPlayer device interface type
     * \sa Solid::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::PortableMediaPlayer;
    }

    /*!
     * Retrieves known protocols this device can speak.  This list may be dependent
     * on installed device driver libraries.
     *
     * Returns a list of known protocols this device can speak
     */
    QStringList supportedProtocols() const;

    /*!
     * Retrieves known installed device drivers that claim to handle this device
     * using the requested protocol.  If protocol is blank, returns a list of
     * all drivers supporting the device.
     *
     * \a protocol The protocol to get drivers for.
     *
     * Returns a list of installed drivers meeting the criteria
     */
    QStringList supportedDrivers(QString protocol = QString()) const;

    /*!
     * Retrieves a \a driver specific string allowing to access the device.
     *
     * For example for the "mtp" driver it will return the serial number
     * of the device.
     *
     * Returns the \a driver specific data
     */
    QVariant driverHandle(const QString &driver) const;
};
}

#endif
