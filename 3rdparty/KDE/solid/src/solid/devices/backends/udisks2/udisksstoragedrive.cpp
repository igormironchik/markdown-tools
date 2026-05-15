/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksstoragedrive.h"

#include <QDebug>

using namespace Solid::Backends::UDisks2;

StorageDrive::StorageDrive(Device *dev)
    : Block(dev)
{
#if UDEV_FOUND
    UdevQt::Client client(this);
    m_udevDevice = client.deviceByDeviceFile(device());
    m_udevDevice.deviceProperties();
#endif
}

StorageDrive::~StorageDrive()
{
}

qulonglong StorageDrive::size() const
{
    return m_device->prop(QStringLiteral("Size")).toULongLong();
}

bool StorageDrive::isHotpluggable() const
{
#if UDEV_FOUND
    const Solid::StorageDrive::Bus _bus = bus();
    /* clang-format off */
    return _bus == Solid::StorageDrive::Usb
        || _bus == Solid::StorageDrive::Ieee1394
        || (m_udevDevice.deviceProperty(QStringLiteral("UDISKS_SYSTEM")).isValid()
            && !m_udevDevice.deviceProperty(QStringLiteral("UDISKS_SYSTEM")).toBool());
    /* clang-format on */
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
    return m_device->prop(QStringLiteral("bsdisks_IsHotpluggable")).toBool();
#else
#error Implement this or stub this out for your platform
#endif
}

bool StorageDrive::isRemovable() const
{
    return m_device->prop(QStringLiteral("MediaRemovable")).toBool() || m_device->prop(QStringLiteral("Removable")).toBool();
}

Solid::StorageDrive::DriveType StorageDrive::driveType() const
{
    const QStringList mediaTypes = m_device->prop(QStringLiteral("MediaCompatibility")).toStringList();

    if (m_device->isOpticalDrive()) { // optical disks
        return Solid::StorageDrive::CdromDrive;
    } else if (mediaTypes.contains(QStringLiteral("floppy"))) {
        return Solid::StorageDrive::Floppy;
    }
#if 0 // TODO add to Solid
    else if (mediaTypes.contains("floppy_jaz")) {
        return Solid::StorageDrive::Jaz;
    } else if (mediaTypes.contains("floppy_zip")) {
        return Solid::StorageDrive::Zip;
    } else if (mediaTypes.contains("flash")) {
        return Solid::StorageDrive::Flash;
    }
#endif
    else if (mediaTypes.contains(QStringLiteral("flash_cf"))) {
        return Solid::StorageDrive::CompactFlash;
    } else if (mediaTypes.contains(QStringLiteral("flash_ms"))) {
        return Solid::StorageDrive::MemoryStick;
    } else if (mediaTypes.contains(QStringLiteral("flash_sm"))) {
        return Solid::StorageDrive::SmartMedia;
    } else if (mediaTypes.contains(QStringLiteral("flash_sd")) //
               || mediaTypes.contains(QStringLiteral("flash_sdhc")) //
               || mediaTypes.contains(QStringLiteral("flash_mmc")) //
               || mediaTypes.contains(QStringLiteral("flash_sdxc"))) {
        return Solid::StorageDrive::SdMmc;
    }
    // FIXME: udisks2 doesn't know about xD cards
    else {
        return Solid::StorageDrive::HardDisk;
    }
}

Solid::StorageDrive::Bus StorageDrive::bus() const
{
    const QString bus = m_device->prop(QStringLiteral("ConnectionBus")).toString();
    const QString udevBus =
#if UDEV_FOUND
        m_udevDevice.deviceProperty(QStringLiteral("ID_BUS")).toString();
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
        m_device->prop(QStringLiteral("bsdisks_ConnectionBus")).toString();
#else
#error Implement this or stub this out for your platform
#endif

    // qDebug() << "bus:" << bus << "udev bus:" << udevBus;

    if (udevBus == QLatin1String("ata")) {
#if UDEV_FOUND
        if (m_udevDevice.deviceProperty(QStringLiteral("ID_ATA_SATA")).toInt() == 1) { // serial ATA
            return Solid::StorageDrive::Sata;
        } else { // parallel (classical) ATA
            return Solid::StorageDrive::Ide;
        }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_OPENBSD)
        if (m_device->prop(QStringLiteral("bsdisks_AtaSata")).toString() == QLatin1String("sata")) { // serial ATA
            return Solid::StorageDrive::Sata;
        } else { // parallel (classical) ATA
            return Solid::StorageDrive::Ide;
        }
#else
#error Implement this or stub this out for your platform
#endif
    } else if (bus == QLatin1String("usb")) {
        return Solid::StorageDrive::Usb;
    } else if (bus == QLatin1String("ieee1394")) {
        return Solid::StorageDrive::Ieee1394;
    } else if (udevBus == QLatin1String("scsi")) {
        return Solid::StorageDrive::Scsi;
    }
#if 0 // TODO add these to Solid
    else if (bus == "sdio") {
        return Solid::StorageDrive::SDIO;
    } else if (bus == "virtual") {
        return Solid::StorageDrive::Virtual;
    }
#endif
    else {
        return Solid::StorageDrive::Platform;
    }
}

QDateTime StorageDrive::timeDetected() const
{
    bool conversionValid;
    const qulonglong microSecondsSinceEpoch = m_device->prop(QStringLiteral("TimeDetected")).toULongLong(&conversionValid);
    if (!conversionValid) {
        return QDateTime();
    }
    return QDateTime::fromMSecsSinceEpoch(microSecondsSinceEpoch / 1000);
}

QDateTime StorageDrive::timeMediaDetected() const
{
    bool conversionValid;
    const qulonglong microSecondsSinceEpoch = m_device->prop(QStringLiteral("TimeMediaDetected")).toULongLong(&conversionValid);
    if (!conversionValid) {
        return QDateTime();
    }
    return QDateTime::fromMSecsSinceEpoch(microSecondsSinceEpoch / 1000);
}

#include "moc_udisksstoragedrive.cpp"
