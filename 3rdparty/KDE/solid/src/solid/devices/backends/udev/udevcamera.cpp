/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevcamera.h"

using namespace Solid::Backends::UDev;

Camera::Camera(UDevDevice *device)
    : DeviceInterface(device)
{
}

Camera::~Camera()
{
}

QStringList Camera::supportedProtocols() const
{
    QString method = m_device->property(QStringLiteral("GPHOTO2_DRIVER")).toString();

    if (!method.isEmpty()) {
        return {method.toLower()};
    }

    return {};
}

QStringList Camera::supportedDrivers(QString /*protocol*/) const
{
    if (!supportedProtocols().isEmpty()) {
        return {QStringLiteral("gphoto")};
    }

    return {};
}

QVariant Camera::driverHandle(const QString &driver) const
{
    if (driver == QLatin1String("gphoto") && m_device->property(QStringLiteral("SUBSYSTEM")).toString() == QLatin1String("usb")) {
        return QVariantList{
            QStringLiteral("usb"),
            m_device->property(QStringLiteral("ID_VENDOR_ID")),
            m_device->property(QStringLiteral("ID_MODEL_ID")),
        };
    }

    return {};
}

#include "moc_udevcamera.cpp"
