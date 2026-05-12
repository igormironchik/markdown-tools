/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakecamera.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeCamera::FakeCamera(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeCamera::~FakeCamera()
{
}

QStringList FakeCamera::supportedProtocols() const
{
    QString method = fakeDevice()->property(QStringLiteral("accessMethod")).toString();

    return {method};
}

QStringList FakeCamera::supportedDrivers(QString /*protocol*/) const
{
    if (fakeDevice()->property(QStringLiteral("gphotoSupport")).toBool()) {
        return {QStringLiteral("gphoto")};
    }

    return {};
}

QVariant Solid::Backends::Fake::FakeCamera::driverHandle(const QString &driver) const
{
    Q_UNUSED(driver);
    return QVariant();
}

#include "moc_fakecamera.cpp"
