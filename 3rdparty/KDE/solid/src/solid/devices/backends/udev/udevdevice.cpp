/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevdevice.h"

#include "cpuinfo.h"
#include "udevblock.h"
#include "udevcamera.h"
#include "udevgenericinterface.h"
#include "udevportablemediaplayer.h"
#include "udevprocessor.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <linux/if_arp.h>

using namespace Solid::Backends::UDev;

namespace
{
// Helper function for three-tier property fallback with hwdb support
QString getUdevPropertyWithFallback(const UdevQt::Device &device, const QString &basePropertyName)
{
    // Try *_FROM_DATABASE first (from hwdb, most readable)
    QString value = device.deviceProperty(basePropertyName + QStringLiteral("_FROM_DATABASE")).toString();
    if (value.isEmpty()) {
        // Fall back to *_ENC (hex-encoded, needs decoding and trimming)
        value = device.decodedDeviceProperty(basePropertyName + QStringLiteral("_ENC")).trimmed();
        if (value.isEmpty()) {
            // Last resort: raw property (may have underscores)
            value = device.deviceProperty(basePropertyName).toString().replace(QLatin1Char('_'), QLatin1Char(' '));
        }
    }
    return value;
}
}

UDevDevice::UDevDevice(const UdevQt::Device device)
    : Solid::Ifaces::Device()
    , m_device(device)
{
}

UDevDevice::~UDevDevice()
{
}

QString UDevDevice::udi() const
{
    return devicePath();
}

QString UDevDevice::parentUdi() const
{
    return QStringLiteral(UDEV_UDI_PREFIX);
}

QString UDevDevice::vendor() const
{
    QString vendor = m_device.sysfsProperty(QStringLiteral("manufacturer")).toString();
    if (vendor.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            vendor = extractCpuVendor(deviceNumber());
        }

        if (vendor.isEmpty()) {
            vendor = getUdevPropertyWithFallback(m_device, QStringLiteral("ID_VENDOR"));
        }
    }
    return vendor;
}

QString UDevDevice::product() const
{
    QString product = m_device.sysfsProperty(QStringLiteral("product")).toString();
    if (product.isEmpty()) {
        if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
            // sysfs doesn't have anything useful here
            product = extractCpuModel(deviceNumber());
        }

        if (product.isEmpty()) {
            product = getUdevPropertyWithFallback(m_device, QStringLiteral("ID_MODEL"));
        }
    }
    return product;
}

QString UDevDevice::displayName() const
{
    QString vendorName = vendor();
    QString productName = product();

    // If both vendor and product are available, check for redundancy
    if (!vendorName.isEmpty() && !productName.isEmpty()) {
        // If product already contains vendor, return product alone to avoid duplication
        if (productName.toLower().contains(vendorName.toLower())) {
            return productName;
        }
        // Otherwise combine them
        return vendorName + QLatin1String(" ") + productName;
    }

    // Fallback: return whichever is available
    if (!productName.isEmpty()) {
        return productName;
    }

    if (!vendorName.isEmpty()) {
        return vendorName;
    }

    // Ultimate fallback: use description
    return description();
}

QString UDevDevice::icon() const
{
    if (parentUdi().isEmpty()) {
        return QLatin1String("computer");
    }

    if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return QLatin1String("cpu");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        // TODO: check out special cases like iPod
        return QLatin1String("multimedia-player");
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return QLatin1String("camera-photo");
    }

    return QString();
}

QStringList UDevDevice::emblems() const
{
    return QStringList();
}

QString UDevDevice::description() const
{
    if (parentUdi().isEmpty()) {
        return tr("Computer");
    }

    if (queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return tr("Processor");
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        /*
         * HACK: As Media player is very generic return the device product instead
         *       until we can return the Name.
         */
        const PortableMediaPlayer *player = new PortableMediaPlayer(const_cast<UDevDevice *>(this));
        if (player->supportedProtocols().contains(QStringLiteral("mtp"))) {
            return product();
        } else {
            // TODO: check out special cases like iPod
            return tr("Portable Media Player");
        }
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return tr("Camera");
    }

    return QString();
}

bool UDevDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;

    case Solid::DeviceInterface::Processor:
        return m_device.subsystem() == QLatin1String("cpu");

    case Solid::DeviceInterface::Camera:
        return m_device.subsystem() == QLatin1String("usb") && property(QStringLiteral("ID_GPHOTO2")).isValid();

    case Solid::DeviceInterface::PortableMediaPlayer:
        return m_device.subsystem() == QLatin1String("usb") && property(QStringLiteral("ID_MEDIA_PLAYER")).isValid();

    case Solid::DeviceInterface::Block:
        return !property(QStringLiteral("MAJOR")).toString().isEmpty();

    default:
        return false;
    }
}

QObject *UDevDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }

    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return new GenericInterface(this);

    case Solid::DeviceInterface::Processor:
        return new Processor(this);

    case Solid::DeviceInterface::Camera:
        return new Camera(this);

    case Solid::DeviceInterface::PortableMediaPlayer:
        return new PortableMediaPlayer(this);

    case Solid::DeviceInterface::Block:
        return new Block(this);

    default:
        qFatal("Shouldn't happen");
        return nullptr;
    }
}

QString UDevDevice::device() const
{
    return devicePath();
}

QVariant UDevDevice::property(const QString &key) const
{
    const QVariant res = m_device.deviceProperty(key);
    if (res.isValid()) {
        return res;
    }
    return m_device.sysfsProperty(key);
}

QMap<QString, QVariant> UDevDevice::allProperties() const
{
    QMap<QString, QVariant> res;
    const QStringList properties = m_device.deviceProperties();
    for (const QString &prop : properties) {
        res[prop] = property(prop);
    }
    return res;
}

bool UDevDevice::propertyExists(const QString &key) const
{
    return m_device.deviceProperties().contains(key);
}

QString UDevDevice::systemAttribute(const char *attribute) const
{
    return m_device.sysfsProperty(QString::fromLatin1(attribute)).toString();
}

QString UDevDevice::deviceName() const
{
    return m_device.sysfsPath();
}

int UDevDevice::deviceNumber() const
{
    return m_device.sysfsNumber();
}

QString UDevDevice::devicePath() const
{
    return QStringLiteral(UDEV_UDI_PREFIX) + deviceName();
}

UdevQt::Device UDevDevice::udevDevice()
{
    return m_device;
}

#include "moc_udevdevice.cpp"
