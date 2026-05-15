/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksdevice.h"
#include "udisks_debug.h"
#include "udisksblock.h"
#include "udisksdeviceinterface.h"
#include "udisksgenericinterface.h"
#include "udisksmanager.h"
#include "udisksopticaldisc.h"
#include "udisksopticaldrive.h"
#include "udisksstorageaccess.h"
#include "udisksstoragevolume.h"

#include <solid/device.h>
#include <solid/genericinterface.h>

#include <QLocale>
#include <QMimeDatabase>

using namespace Solid::Backends::UDisks2;

// Adapted from KLocale as Solid needs to be Qt-only
static QString formatByteSize(double size)
{
    // Per IEC 60027-2

    // Binary prefixes
    // Tebi-byte             TiB             2^40    1,099,511,627,776 bytes
    // Gibi-byte             GiB             2^30    1,073,741,824 bytes
    // Mebi-byte             MiB             2^20    1,048,576 bytes
    // Kibi-byte             KiB             2^10    1,024 bytes

    QString s;
    // Gibi-byte
    if (size >= 1073741824.0) {
        size /= 1073741824.0;
        if (size > 1024) { // Tebi-byte
            s = QCoreApplication::translate("udisksdevice", "%1 TiB").arg(QLocale().toString(size / 1024.0, 'f', 1));
        } else {
            s = QCoreApplication::translate("udisksdevice", "%1 GiB").arg(QLocale().toString(size, 'f', 1));
        }
    }
    // Mebi-byte
    else if (size >= 1048576.0) {
        size /= 1048576.0;
        s = QCoreApplication::translate("udisksdevice", "%1 MiB").arg(QLocale().toString(size, 'f', 1));
    }
    // Kibi-byte
    else if (size >= 1024.0) {
        size /= 1024.0;
        s = QCoreApplication::translate("udisksdevice", "%1 KiB").arg(QLocale().toString(size, 'f', 1));
    }
    // Just byte
    else if (size > 0) {
        s = QCoreApplication::translate("udisksdevice", "%1 B").arg(QLocale().toString(size, 'f', 1));
    }
    // Nothing
    else {
        s = QCoreApplication::translate("udisksdevice", "0 B");
    }
    return s;
}

static QString concatBlockDeviceDescription(const QString &name, qulonglong size, bool isExternal)
{
    QString description;
    if (size > 0) {
        const QString sizeStr = formatByteSize(size);
        if (isExternal) {
            description = QObject::tr("%1 External Drive (%2)", "%1 is the size, %2 is the block device name e.g. sda, sda1").arg(sizeStr, name);
        } else {
            description = QObject::tr("%1 Internal Drive (%2)", "%1 is the size, %2 is the block device name e.g. sda, sda1").arg(sizeStr, name);
        }
    } else {
        if (isExternal) {
            description = QObject::tr("External Drive (%1)", "%1 is the block device name e.g. sda, sda1").arg(name);
        } else {
            description = QObject::tr("Internal Drive (%1)", "%1 is the block device name e.g. sda, sda1").arg(name);
        }
    }

    return description;
}

Device::Device(Manager *manager, const QString &udi)
    : Solid::Ifaces::Device()
    , m_manager(manager)
    , m_udi(udi)
{
    connect(m_manager, &Manager::propertyChanged, this, [this](const QString &udi, const QMap<QString, int> &changes) {
        if (udi == m_udi) {
            Q_EMIT propertyChanged(changes);
            Q_EMIT changed();
        }
    });
}

Device::~Device()
{
}

QString Device::udi() const
{
    return m_udi;
}

QVariant Device::prop(const QString &key) const
{
    return m_manager->deviceProperty(m_udi, key);
}

bool Device::propertyExists(const QString &key) const
{
    return m_manager->deviceProperty(m_udi, key, Manager::CachedOnly).isValid();
}

QVariantMap Device::allProperties() const
{
    QVariantMap flattened;

    const auto interfaces = m_manager->deviceProperties(m_udi);

    // Flatten per-interface properties into a single map.
    // We iterate the interfaces in reverse since Manager::prop() returns the *first*
    // property found, so we'll override any other properties that way.
    // FIXME actually reverse it lol, no rbegin on QMap :(
    for (const auto &props : interfaces) {
        flattened.insert(props);
    }

    return flattened;
}

bool Device::hasInterface(const QString &name) const
{
    return m_manager->hasInterface(m_udi, name);
}

Manager *Device::manager() const
{
    return m_manager;
}

QObject *Device::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
        return nullptr;
    }

    DeviceInterface *iface = nullptr;
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        iface = new GenericInterface(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new Block(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new StorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new StorageDrive(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new OpticalDrive(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new StorageVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new OpticalDisc(this);
        break;
    default:
        break;
    }
    return iface;
}

bool Device::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;
    case Solid::DeviceInterface::Block:
        return isBlock() || isDrive();
    case Solid::DeviceInterface::StorageVolume:
        return isStorageVolume();
    case Solid::DeviceInterface::StorageAccess:
        return isStorageAccess();
    case Solid::DeviceInterface::StorageDrive:
        return isDrive();
    case Solid::DeviceInterface::OpticalDrive:
        return isOpticalDrive();
    case Solid::DeviceInterface::OpticalDisc:
        return isOpticalDisc();
    default:
        return false;
    }
}

QStringList Device::emblems() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        const UDisks2::StorageAccess accessIface(const_cast<Device *>(this));
        if (accessIface.isAccessible()) {
            if (isEncryptedContainer()) {
                return {QStringLiteral("emblem-encrypted-unlocked")};
            }
        } else {
            if (isEncryptedContainer()) {
                return {QStringLiteral("emblem-encrypted-locked")};
            } else {
                return {QStringLiteral("emblem-unmounted")};
            }
        }
    }

    return {};
}

QString Device::description() const
{
    const QString hintName = property("HintName").toString(); // non-cached
    if (!hintName.isEmpty()) {
        return hintName;
    }

    if (isLoop()) {
        return loopDescription();
    } else if (isSwap()) {
        return tr("Swap Space");
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        return storageDescription();
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume)) {
        return volumeDescription();
    } else {
        return product();
    }
}

QString Device::loopDescription() const
{
    const QString label = prop(QStringLiteral("IdLabel")).toString();
    if (!label.isEmpty()) {
        return label;
    }

    const QString backingFile = prop(QStringLiteral("BackingFile")).toString();
    if (!backingFile.isEmpty()) {
        return backingFile.section(QLatin1Char('/'), -1);
    }

    return tr("Loop Device");
}

QString Device::storageDescription() const
{
    QString description;
    const UDisks2::StorageDrive storageDrive(const_cast<Device *>(this));
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();
    const bool drive_is_hotpluggable = storageDrive.isHotpluggable();

    if (drive_type == Solid::StorageDrive::CdromDrive) {
        const UDisks2::OpticalDrive opticalDrive(const_cast<Device *>(this));
        Solid::OpticalDrive::MediumTypes mediumTypes = opticalDrive.supportedMedia();
        QString first;
        QString second;

        first = tr("CD-ROM", "First item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Cdr) {
            first = tr("CD-R", "First item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Cdrw) {
            first = tr("CD-RW", "First item of %1%2 Drive sentence");
        }

        if (mediumTypes & Solid::OpticalDrive::Dvd) {
            second = tr("/DVD-ROM", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Dvdplusr) {
            second = tr("/DVD+R", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Dvdplusrw) {
            second = tr("/DVD+RW", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Dvdr) {
            second = tr("/DVD-R", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Dvdrw) {
            second = tr("/DVD-RW", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Dvdram) {
            second = tr("/DVD-RAM", "Second item of %1%2 Drive sentence");
        }
        if ((mediumTypes & Solid::OpticalDrive::Dvdr) && (mediumTypes & Solid::OpticalDrive::Dvdplusr)) {
            if (mediumTypes & Solid::OpticalDrive::Dvdplusdl) {
                second = tr("/DVD±R DL", "Second item of %1%2 Drive sentence");
            } else {
                second = tr("/DVD±R", "Second item of %1%2 Drive sentence");
            }
        }
        if ((mediumTypes & Solid::OpticalDrive::Dvdrw) && (mediumTypes & Solid::OpticalDrive::Dvdplusrw)) {
            if ((mediumTypes & Solid::OpticalDrive::Dvdplusdl) || (mediumTypes & Solid::OpticalDrive::Dvdplusdlrw)) {
                second = tr("/DVD±RW DL", "Second item of %1%2 Drive sentence");
            } else {
                second = tr("/DVD±RW", "Second item of %1%2 Drive sentence");
            }
        }
        if (mediumTypes & Solid::OpticalDrive::Bd) {
            second = tr("/BD-ROM", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Bdr) {
            second = tr("/BD-R", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Bdre) {
            second = tr("/BD-RE", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::HdDvd) {
            second = tr("/HD DVD-ROM", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::HdDvdr) {
            second = tr("/HD DVD-R", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::HdDvdrw) {
            second = tr("/HD DVD-RW", "Second item of %1%2 Drive sentence");
        }

        if (drive_is_hotpluggable) {
            description = tr("External %1%2 Drive", "%1 is CD-ROM/CD-R/etc; %2 is '/DVD-ROM'/'/DVD-R'/etc (with leading slash)").arg(first, second);
        } else {
            description = tr("%1%2 Drive", "%1 is CD-ROM/CD-R/etc; %2 is '/DVD-ROM'/'/DVD-R'/etc (with leading slash)").arg(first, second);
        }

        return description;
    }

    if (drive_type == Solid::StorageDrive::Floppy) {
        if (drive_is_hotpluggable) {
            description = tr("External Floppy Drive");
        } else {
            description = tr("Floppy Drive");
        }

        return description;
    }

    const bool drive_is_removable = storageDrive.isRemovable();

    if (drive_type == Solid::StorageDrive::HardDisk && !drive_is_removable) {
        QString devName = storageDrive.device();
        devName.remove(QLatin1String("/dev/"));
        description = concatBlockDeviceDescription(devName, storageDrive.size(), drive_is_hotpluggable);

        return description;
    }

    QString vendormodel_str;
    QString model = product();
    QString vendor_str = vendor();

    if (vendor_str.isEmpty()) {
        if (!model.isEmpty()) {
            vendormodel_str = model;
        }
    } else {
        if (model.isEmpty()) {
            vendormodel_str = vendor_str;
        } else {
            if (model.startsWith(vendor_str)) {
                // e.g. vendor is "Nokia" and model is "Nokia N950" we do not want "Nokia Nokia N950" as description
                vendormodel_str = model;
            } else {
                vendormodel_str = tr("%1 %2", "%1 is the vendor, %2 is the model of the device").arg(vendor_str, model);
            }
        }
    }

    if (vendormodel_str.isEmpty()) {
        description = tr("Drive");
    } else {
        description = vendormodel_str;
    }

    return description;
}

QString Device::volumeDescription() const
{
    QString description;
    const UDisks2::StorageVolume storageVolume(const_cast<Device *>(this));
    QString volume_label = prop(QStringLiteral("IdLabel")).toString();
    if (volume_label.isEmpty()) {
        volume_label = prop(QStringLiteral("Name")).toString();
    }
    if (!volume_label.isEmpty()) {
        return volume_label;
    }

    UDisks2::Device storageDevice(manager(), drivePath());
    const UDisks2::StorageDrive storageDrive(&storageDevice);
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();

    // Handle media in optical drives
    if (drive_type == Solid::StorageDrive::CdromDrive) {
        const UDisks2::OpticalDisc disc(const_cast<Device *>(this));
        switch (disc.discType()) {
        case Solid::OpticalDisc::UnknownDiscType:
        case Solid::OpticalDisc::CdRom:
            description = tr("CD-ROM");
            break;

        case Solid::OpticalDisc::CdRecordable:
            if (disc.isBlank()) {
                description = tr("Blank CD-R");
            } else {
                description = tr("CD-R");
            }
            break;

        case Solid::OpticalDisc::CdRewritable:
            if (disc.isBlank()) {
                description = tr("Blank CD-RW");
            } else {
                description = tr("CD-RW");
            }
            break;

        case Solid::OpticalDisc::DvdRom:
            description = tr("DVD-ROM");
            break;

        case Solid::OpticalDisc::DvdRam:
            if (disc.isBlank()) {
                description = tr("Blank DVD-RAM");
            } else {
                description = tr("DVD-RAM");
            }
            break;

        case Solid::OpticalDisc::DvdRecordable:
            if (disc.isBlank()) {
                description = tr("Blank DVD-R");
            } else {
                description = tr("DVD-R");
            }
            break;

        case Solid::OpticalDisc::DvdPlusRecordableDuallayer:
            if (disc.isBlank()) {
                description = tr("Blank DVD+R Dual-Layer");
            } else {
                description = tr("DVD+R Dual-Layer");
            }
            break;

        case Solid::OpticalDisc::DvdRewritable:
            if (disc.isBlank()) {
                description = tr("Blank DVD-RW");
            } else {
                description = tr("DVD-RW");
            }
            break;

        case Solid::OpticalDisc::DvdPlusRecordable:
            if (disc.isBlank()) {
                description = tr("Blank DVD+R");
            } else {
                description = tr("DVD+R");
            }
            break;

        case Solid::OpticalDisc::DvdPlusRewritable:
            if (disc.isBlank()) {
                description = tr("Blank DVD+RW");
            } else {
                description = tr("DVD+RW");
            }
            break;

        case Solid::OpticalDisc::DvdPlusRewritableDuallayer:
            if (disc.isBlank()) {
                description = tr("Blank DVD+RW Dual-Layer");
            } else {
                description = tr("DVD+RW Dual-Layer");
            }
            break;

        case Solid::OpticalDisc::BluRayRom:
            description = tr("BD-ROM");
            break;

        case Solid::OpticalDisc::BluRayRecordable:
            if (disc.isBlank()) {
                description = tr("Blank BD-R");
            } else {
                description = tr("BD-R");
            }
            break;

        case Solid::OpticalDisc::BluRayRewritable:
            if (disc.isBlank()) {
                description = tr("Blank BD-RE");
            } else {
                description = tr("BD-RE");
            }
            break;

        case Solid::OpticalDisc::HdDvdRom:
            description = tr("HD DVD-ROM");
            break;

        case Solid::OpticalDisc::HdDvdRecordable:
            if (disc.isBlank()) {
                description = tr("Blank HD DVD-R");
            } else {
                description = tr("HD DVD-R");
            }
            break;

        case Solid::OpticalDisc::HdDvdRewritable:
            if (disc.isBlank()) {
                description = tr("Blank HD DVD-RW");
            } else {
                description = tr("HD DVD-RW");
            }
            break;
        }

        // Special case for pure audio disc
        if (disc.availableContent() == Solid::OpticalDisc::Audio) {
            description = tr("Audio CD");
        }

        return description;
    }

    const bool drive_is_removable = storageDrive.isRemovable();

    QString size_str = formatByteSize(storageVolume.size());
    QString volumeName = storageVolume.device();
    volumeName.remove(QLatin1String("/dev/"));
    if (isEncryptedContainer()) {
        if (storageVolume.size() > 0) {
            description = tr("%1 Encrypted Drive", "%1 is the size").arg(size_str);
        } else {
            description = tr("Encrypted Drive");
        }
    } else if (drive_type == Solid::StorageDrive::HardDisk && !drive_is_removable) {
        description = concatBlockDeviceDescription(volumeName, storageVolume.size(), storageDrive.isHotpluggable());
    } else if (drive_type == Solid::StorageDrive::Floppy) {
        description = tr("Floppy Disk");
    } else {
        if (drive_is_removable) {
            if (storageVolume.size() > 0) {
                description = tr("%1 Removable Media", "%1 is the size").arg(size_str);
            } else {
                description = tr("Removable Media");
            }
        } else {
            if (storageVolume.size() > 0) {
                description = tr("%1 Media", "%1 is the size").arg(size_str);
            } else {
                description = tr("Storage Media");
            }
        }
    }

    return description;
}

QString Device::icon() const
{
    QString iconName = property("HintIconName").toString(); // non-cached

    if (!iconName.isEmpty()) {
        return iconName;
    } else if (isRoot()) {
        return QStringLiteral("drive-harddisk-root");
    } else if (isLoop()) {
        const QString backingFile = prop(QStringLiteral("BackingFile")).toString();
        if (!backingFile.isEmpty()) {
            QMimeType type = QMimeDatabase().mimeTypeForFile(backingFile);
            if (!type.isDefault()) {
                return type.iconName();
            }
        }
        return QStringLiteral("drive-harddisk");
    } else if (isSwap()) {
        return QStringLiteral("drive-harddisk");
    } else if (isDrive()) {
        const bool isRemovable = prop(QStringLiteral("Removable")).toBool();
        const QString conn = prop(QStringLiteral("ConnectionBus")).toString();

        if (isOpticalDrive()) {
            return QStringLiteral("drive-optical");
        } else if (isRemovable && !prop(QStringLiteral("Optical")).toBool()) {
            if (conn == QLatin1String("usb")) {
                return QStringLiteral("drive-removable-media-usb");
            } else {
                return QStringLiteral("drive-removable-media");
            }
        }
    } else if (isBlock()) {
        const QString drv = drivePath();
        if (drv.isEmpty() || drv == QLatin1String("/")) {
            return QStringLiteral("drive-harddisk"); // stuff like loop devices or swap which don't have the Drive prop set
        }

        Device drive(manager(), drv);

        // handle media
        const QString media = drive.prop(QStringLiteral("Media")).toString();

        if (!media.isEmpty()) {
            if (drive.prop(QStringLiteral("Optical")).toBool()) { // optical stuff
                bool isWritable = drive.prop(QStringLiteral("OpticalBlank")).toBool();

                const UDisks2::OpticalDisc disc(const_cast<Device *>(this));
                Solid::OpticalDisc::ContentTypes availContent = disc.availableContent();

                if (availContent & Solid::OpticalDisc::VideoDvd) { // Video DVD
                    return QStringLiteral("media-optical-dvd-video");
                } else if ((availContent & Solid::OpticalDisc::VideoCd) || (availContent & Solid::OpticalDisc::SuperVideoCd)) { // Video CD
                    return QStringLiteral("media-optical-video");
                } else if ((availContent & Solid::OpticalDisc::Data) && (availContent & Solid::OpticalDisc::Audio)) { // Mixed CD
                    return QStringLiteral("media-optical-mixed-cd");
                } else if (availContent & Solid::OpticalDisc::Audio) { // Audio CD
                    return QStringLiteral("media-optical-audio");
                } else if (availContent & Solid::OpticalDisc::Data) { // Data CD
                    return QStringLiteral("media-optical-data");
                } else if (isWritable) {
                    return QStringLiteral("media-optical-recordable");
                } else {
                    if (media.startsWith(QStringLiteral("optical_dvd")) || media.startsWith(QStringLiteral("optical_hddvd"))) { // DVD
                        return QStringLiteral("media-optical-dvd");
                    } else if (media.startsWith(QStringLiteral("optical_bd"))) { // BluRay
                        return QStringLiteral("media-optical-blu-ray");
                    }
                }

                // fallback for every other optical disc
                return QStringLiteral("media-optical");
            }

            if (media == QLatin1String("flash_ms")) { // Flash & Co.
                return QStringLiteral("media-flash-memory-stick");
            } else if (media == QLatin1String("flash_sd") //
                       || media == QLatin1String("flash_sdhc") //
                       || media == QLatin1String("flash_sdxc") //
                       || media == QLatin1String("flash_mmc")) {
                return QStringLiteral("media-flash-sd-mmc");
            } else if (media == QLatin1String("flash_sm")) {
                return QStringLiteral("media-flash-smart-media");
            } else if (media == QLatin1String("thumb")) {
                return QStringLiteral("drive-removable-media-usb-pendrive");
            } else if (media.startsWith(QStringLiteral("flash"))) {
                return QStringLiteral("media-flash");
            } else if (media == QLatin1String("floppy")) { // the good ol' floppy
                return QStringLiteral("media-floppy");
            }
        }

        if (drive.prop(QStringLiteral("ConnectionBus")).toString() == QLatin1String("sdio")) { // hack for SD cards connected thru sdio bus
            return QStringLiteral("media-flash-sd-mmc");
        }

        return drive.icon();
    }

    return QStringLiteral("drive-harddisk"); // general fallback
}

QString Device::product() const
{
    if (!isDrive()) {
        Device drive(manager(), drivePath());
        return drive.prop(QStringLiteral("Model")).toString();
    }

    return prop(QStringLiteral("Model")).toString();
}

QString Device::vendor() const
{
    if (!isDrive()) {
        Device drive(manager(), drivePath());
        return drive.prop(QStringLiteral("Vendor")).toString();
    }

    return prop(QStringLiteral("Vendor")).toString();
}

QString Device::parentUdi() const
{
    QString parent;

    if (propertyExists(QStringLiteral("Drive"))) { // block
        parent = drivePath();
    } else if (propertyExists(QStringLiteral("Table"))) { // partition
        parent = prop(QStringLiteral("Table")).value<QDBusObjectPath>().path();
    } else if (parent.isEmpty() || parent == QLatin1String("/")) {
        parent = QStringLiteral(UD2_UDI_DISKS_PREFIX);
    }
    return parent;
}

QString Device::errorToString(const QString &error) const
{
    if (error == QLatin1String(UD2_ERROR_UNAUTHORIZED) || error == QLatin1String(UD2_ERROR_NOT_AUTHORIZED)) {
        return tr("You are not authorized to perform this operation");
    } else if (error == QLatin1String(UD2_ERROR_BUSY)) {
        return tr("The device is currently busy");
    } else if (error == QLatin1String(UD2_ERROR_FAILED)) {
        return tr("The requested operation has failed");
    } else if (error == QLatin1String(UD2_ERROR_CANCELED)) {
        return tr("The requested operation has been canceled");
    } else if (error == QLatin1String(UD2_ERROR_INVALID_OPTION)) {
        return tr("An invalid or malformed option has been given");
    } else if (error == QLatin1String(UD2_ERROR_MISSING_DRIVER)) {
        return tr("The kernel driver for this filesystem type is not available");
    } else if (error == QLatin1String(UD2_ERROR_ALREADY_MOUNTED)) {
        return tr("The device is already mounted");
    } else if (error == QLatin1String(UD2_ERROR_NOT_MOUNTED)) {
        return tr("The device is not mounted");
    } else if (error == QLatin1String(UD2_ERROR_MOUNTED_BY_OTHER_USER)) {
        return tr("The device is mounted by another user");
    } else if (error == QLatin1String(UD2_ERROR_ALREADY_UNMOUNTING)) {
        return tr("The device is already unmounting");
    } else if (error == QLatin1String(UD2_ERROR_TIMED_OUT)) {
        return tr("The operation timed out");
    } else if (error == QLatin1String(UD2_ERROR_WOULD_WAKEUP)) {
        return tr("The operation would wake up a disk that is in a deep-sleep state");
    } else if (error == QLatin1String(UD2_ERROR_ALREADY_CANCELLED)) {
        return tr("The operation has already been canceled");
    } else if (error == QLatin1String(UD2_ERROR_NOT_AUTHORIZED_CAN_OBTAIN)) {
        return tr("Cannot request authentication for this action. The PolicyKit authentication system appears to be not available.");
    } else if (error == QLatin1String(UD2_ERROR_NOT_AUTHORIZED_DISMISSED)) {
        return tr("The authentication prompt was canceled");
    } else {
        return tr("An unspecified error has occurred");
    }
}

Solid::ErrorType Device::errorToSolidError(const QString &error) const
{
    if (error == QLatin1String(UD2_ERROR_BUSY)) {
        return Solid::DeviceBusy;
    } else if (error == QLatin1String(UD2_ERROR_FAILED)) {
        return Solid::OperationFailed;
    } else if (error == QLatin1String(UD2_ERROR_CANCELED)) {
        return Solid::UserCanceled;
    } else if (error == QLatin1String(UD2_ERROR_INVALID_OPTION)) {
        return Solid::InvalidOption;
    } else if (error == QLatin1String(UD2_ERROR_MISSING_DRIVER)) {
        return Solid::MissingDriver;
    } else if (error == QLatin1String(UD2_ERROR_NOT_AUTHORIZED_DISMISSED)) {
        return Solid::UserCanceled;
    } else {
        return Solid::UnauthorizedOperation;
    }
}

bool Device::isBlock() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_BLOCK));
}

bool Device::isPartition() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_PARTITION));
}

bool Device::isPartitionTable() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_PARTITIONTABLE));
}

bool Device::isStorageVolume() const
{
    return isPartition() || isPartitionTable() || isStorageAccess() || isOpticalDisc();
}

bool Device::isStorageAccess() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_FILESYSTEM)) || isEncryptedContainer();
}

bool Device::isDrive() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_DRIVE));
}

bool Device::isOpticalDrive() const
{
    return isDrive() && !prop(QStringLiteral("MediaCompatibility")).toStringList().filter(QStringLiteral("optical_")).isEmpty();
}

bool Device::isOpticalDisc() const
{
    const QString drv = drivePath();
    if (drv.isEmpty() || drv == QLatin1String("/")) {
        return false;
    }

    Device drive(manager(), drv);
    return drive.prop(QStringLiteral("Optical")).toBool();
}

bool Device::mightBeOpticalDisc() const
{
    const QString drv = drivePath();
    if (drv.isEmpty() || drv == QLatin1String("/")) {
        return false;
    }

    Device drive(manager(), drv);
    return drive.isOpticalDrive();
}

bool Device::isMounted() const
{
    QVariant mountPoints = prop(QStringLiteral("MountPoints"));
    return mountPoints.isValid() && !qdbus_cast<QByteArrayList>(mountPoints).isEmpty();
}

bool Device::isEncryptedContainer() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_ENCRYPTED));
}

bool Device::isEncryptedCleartext() const
{
    const QString holderDevice = prop(QStringLiteral("CryptoBackingDevice")).value<QDBusObjectPath>().path();
    if (holderDevice.isEmpty() || holderDevice == QLatin1String("/")) {
        return false;
    } else {
        return true;
    }
}

bool Device::isRoot() const
{
    if (isStorageAccess()) {
        const UDisks2::StorageAccess accessIface(const_cast<Device *>(this));
        return accessIface.filePath() == QLatin1String("/");
    }
    return false;
}

bool Device::isSwap() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_SWAP));
}

bool Device::isLoop() const
{
    return hasInterface(QStringLiteral(UD2_DBUS_INTERFACE_LOOP));
}

QString Device::drivePath() const
{
    return prop(QStringLiteral("Drive")).value<QDBusObjectPath>().path();
}

#include "moc_udisksdevice.cpp"
