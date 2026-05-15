/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_GENERICINTERFACE_H
#define SOLID_GENERICINTERFACE_H

#include <QMap>
#include <QVariant>

#include <solid/deviceinterface.h>
#include <solid/solid_export.h>

namespace Solid
{
class GenericInterfacePrivate;
class Device;

/*!
 * \class Solid::GenericInterface
 * \inheaderfile Solid/GenericInterface
 * \inmodule Solid
 *
 * \brief Generic interface to deal with a device. It exposes a set of properties
 * and is organized as a key/value set.
 *
 * \warning Using this class could expose some backend specific details
 * and lead to non portable code. Use it at your own risk, or during
 * transitional phases when the provided device interfaces don't
 * provide the necessary methods.
 */
class SOLID_EXPORT GenericInterface : public DeviceInterface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GenericInterface)
    friend class Device;

public:
    /*!
     * This enum type defines the type of change that can occur to a GenericInterface
     * property.
     *
     * \value PropertyModified A property value has changed in the device
     * \value PropertyAdded A new property has been added to the device
     * \value PropertyRemoved A property has been removed from the device
     */
    enum PropertyChange {
        PropertyModified,
        PropertyAdded,
        PropertyRemoved,
    };
    Q_ENUM(PropertyChange)

private:
    /*!
     * \internal
     *
     * Creates a new GenericInterface object.
     *
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     *
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit GenericInterface(QObject *backendObject);

public:
    ~GenericInterface() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the GenericInterface device interface.
     *
     * Returns the Processor device interface type
     * \sa Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::GenericInterface;
    }

    /*!
     * Retrieves a property of the device.
     *
     * \warning Using this method could expose some backend specific details
     * and lead to non portable code. Use it at your own risk, or during
     * transitional phases when the provided device interfaces don't
     * provide the necessary methods.
     *
     * \a key the property key
     *
     * Returns the actual value of the property, or QVariant() if the
     * property is unknown
     */
    QVariant property(const QString &key) const;

    /*!
     * Retrieves a key/value map of all the known properties for the device.
     *
     * \warning Using this method could expose some backend specific details
     * and lead to non portable code. Use it at your own risk, or during
     * transitional phases when the provided device interfaces don't
     * provide the necessary methods.
     *
     * Returns all the properties of the device
     */
    QMap<QString, QVariant> allProperties() const;

    /*!
     * Tests if a property exist in the device.
     *
     * \warning Using this method could expose some backend specific details
     * and lead to non portable code. Use it at your own risk, or during
     * transitional phases when the provided device interfaces don't
     * provide the necessary methods.
     *
     * \a key the property key
     *
     * Returns true if the property is available in the device, false
     * otherwise
     */
    bool propertyExists(const QString &key) const;

Q_SIGNALS:
    /*!
     * This signal is emitted when a property is changed in the device.
     *
     * \a changes the map describing the property changes that
     * occurred in the device, keys are property name and values
     * describe the kind of change done on the device property
     * (added/removed/modified), it's one of the type Solid::Device::PropertyChange
     */
    void propertyChanged(const QMap<QString, int> &changes);

    /*!
     * This signal is emitted when an event occurred in the device.
     * For example when a button is pressed.
     *
     * \a condition the condition name
     *
     * \a reason a message explaining why the condition has been raised
     */
    void conditionRaised(const QString &condition, const QString &reason);
};
}

#endif
