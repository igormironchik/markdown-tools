/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_NETWORKSHARE_H
#define SOLID_NETWORKSHARE_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

#include <QUrl>

namespace Solid
{
class Device;
class NetworkSharePrivate;

/*!
 * \class Solid::NetworkShare
 * \inheaderfile Solid/NetworkShare
 * \inmodule Solid
 *
 * \brief NetworkShare interface.
 *
 * A NetworkShare interface is used to determine the type of
 * network access.
 * \since 4.7
 */
class SOLID_EXPORT NetworkShare : public DeviceInterface
{
    Q_OBJECT

    /*!
     * \property Solid::NetworkShare::type
     */
    Q_PROPERTY(ShareType type READ type)

    /*!
     * \property Solid::NetworkShare::url
     */
    Q_PROPERTY(QUrl url READ url)

    Q_DECLARE_PRIVATE(NetworkShare)
    friend class Device;

private:
    /*!
     * \internal
     *
     * Creates a new NetworkShare object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit NetworkShare(QObject *backendObject);

public:
    ~NetworkShare() override;

    /*!
     * This enum type defines the type of networkShare device can be.
     *
     * \value Unknown a unsupported network protocol
     * \value Nfs nfs protocol
     * \value Cifs samba protocol
     * \value Smb3 samba protocol (version 3)
     */
    enum ShareType { Unknown, Nfs, Cifs, Smb3 };
    Q_ENUM(ShareType)

    /*!
     * Get the Solid::DeviceInterface::Type of the NetworkShare device interface.
     *
     * Returns the NetworkShare device interface type
     * \sa Solid::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::NetworkShare;
    }

    /*!
     * Retrieves the type of network share
     *
     * Returns the type of network share
     */
    ShareType type() const;

    /*!
     * Retrieves the url of network share
     *
     * Returns the url of network share
     */
    QUrl url() const;
};
}

#endif
