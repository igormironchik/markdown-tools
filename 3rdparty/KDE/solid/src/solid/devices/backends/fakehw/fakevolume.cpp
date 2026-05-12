/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakevolume.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeVolume::FakeVolume(FakeDevice *device)
    : FakeBlock(device)
{
}

FakeVolume::~FakeVolume()
{
}

bool FakeVolume::isIgnored() const
{
    return fakeDevice()->property(QStringLiteral("isIgnored")).toBool();
}

Solid::StorageVolume::UsageType FakeVolume::usage() const
{
    QString usage = fakeDevice()->property(QStringLiteral("usage")).toString();

    if (usage == QLatin1String("filesystem")) {
        return Solid::StorageVolume::FileSystem;
    } else if (usage == QLatin1String("partitiontable")) {
        return Solid::StorageVolume::PartitionTable;
    } else if (usage == QLatin1String("raid")) {
        return Solid::StorageVolume::Raid;
    } else if (usage == QLatin1String("unused")) {
        return Solid::StorageVolume::Unused;
    } else if (usage == QLatin1String("encrypted")) {
        return Solid::StorageVolume::Encrypted;
    } else {
        return Solid::StorageVolume::Other;
    }
}

QString FakeVolume::fsType() const
{
    return fakeDevice()->property(QStringLiteral("fsType")).toString();
}

QString FakeVolume::label() const
{
    return fakeDevice()->property(QStringLiteral("label")).toString();
}

QString FakeVolume::uuid() const
{
    return fakeDevice()->property(QStringLiteral("uuid")).toString();
}

qulonglong FakeVolume::size() const
{
    return fakeDevice()->property(QStringLiteral("size")).toULongLong();
}

QString Solid::Backends::Fake::FakeVolume::encryptedContainerUdi() const
{
    return QString();
}

#include "moc_fakevolume.cpp"
