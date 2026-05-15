/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WINDEVICE_H
#define WINDEVICE_H

#include "windevicemanager.h"
#include <solid/devices/ifaces/device.h>

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinDevice : public Solid::Ifaces::Device
{
    Q_OBJECT
public:
    WinDevice(const QString &udi);

    QString udi() const override;

    QString parentUdi() const override;
    QString vendor() const override;
    QString product() const override;

    QString icon() const override;

    QStringList emblems() const override;

    QString description() const override;

    bool queryDeviceInterface(const Solid::DeviceInterface::Type &type) const override;
    QObject *createDeviceInterface(const Solid::DeviceInterface::Type &type) override;

    Solid::DeviceInterface::Type type() const;

private:
    QString m_udi;
    QString m_parentUdi;
    QString m_vendor;
    QString m_product;
    QString m_description;
    Solid::DeviceInterface::Type m_type;

    void initStorageDevice();
    void initBatteryDevice();
    void initCpuDevice();
};

}
}
}

#endif // WINDEVICE_H
