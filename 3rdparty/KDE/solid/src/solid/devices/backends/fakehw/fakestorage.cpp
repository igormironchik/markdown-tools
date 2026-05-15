/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakestorage.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeStorage::FakeStorage(FakeDevice *device)
    : FakeBlock(device)
{
}

FakeStorage::~FakeStorage()
{
}

Solid::StorageDrive::Bus FakeStorage::bus() const
{
    QString bus = fakeDevice()->property(QStringLiteral("bus")).toString();

    if (bus == QLatin1String("ide")) {
        return Solid::StorageDrive::Ide;
    } else if (bus == QLatin1String("usb")) {
        return Solid::StorageDrive::Usb;
    } else if (bus == QLatin1String("ieee1394")) {
        return Solid::StorageDrive::Ieee1394;
    } else if (bus == QLatin1String("scsi")) {
        return Solid::StorageDrive::Scsi;
    } else if (bus == QLatin1String("sata")) {
        return Solid::StorageDrive::Sata;
    } else {
        return Solid::StorageDrive::Platform;
    }
}

Solid::StorageDrive::DriveType FakeStorage::driveType() const
{
    QString type = fakeDevice()->property(QStringLiteral("major")).toString();

    if (type == QLatin1String("disk")) {
        return Solid::StorageDrive::HardDisk;
    } else if (type == QLatin1String("cdrom")) {
        return Solid::StorageDrive::CdromDrive;
    } else if (type == QLatin1String("floppy")) {
        return Solid::StorageDrive::Floppy;
    } else if (type == QLatin1String("tape")) {
        return Solid::StorageDrive::Tape;
    } else if (type == QLatin1String("compact_flash")) {
        return Solid::StorageDrive::CompactFlash;
    } else if (type == QLatin1String("memory_stick")) {
        return Solid::StorageDrive::MemoryStick;
    } else if (type == QLatin1String("smart_media")) {
        return Solid::StorageDrive::SmartMedia;
    } else if (type == QLatin1String("sd_mmc")) {
        return Solid::StorageDrive::SdMmc;
    } else {
        return Solid::StorageDrive::HardDisk;
    }
}

bool FakeStorage::isRemovable() const
{
    return fakeDevice()->property(QStringLiteral("isRemovable")).toBool();
}

bool FakeStorage::isHotpluggable() const
{
    return fakeDevice()->property(QStringLiteral("isHotpluggable")).toBool();
}

qulonglong FakeStorage::size() const
{
    return fakeDevice()->property(QStringLiteral("size")).toULongLong();
}

QDateTime FakeStorage::timeDetected() const
{
    return fakeDevice()->property(QStringLiteral("timeDetected")).toDateTime();
}

QDateTime FakeStorage::timeMediaDetected() const
{
    return fakeDevice()->property(QStringLiteral("timeMediaDetected")).toDateTime();
}

#include "moc_fakestorage.cpp"
