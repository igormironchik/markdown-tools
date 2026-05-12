/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeopticaldisc.h"

#include <QStringList>
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeOpticalDisc::FakeOpticalDisc(FakeDevice *device)
    : FakeVolume(device)
{
}

FakeOpticalDisc::~FakeOpticalDisc()
{
}

Solid::OpticalDisc::ContentTypes FakeOpticalDisc::availableContent() const
{
    Solid::OpticalDisc::ContentTypes content;

    const QMap<QString, Solid::OpticalDisc::ContentType> map = {
        {QStringLiteral("audio"), Solid::OpticalDisc::Audio},
        {QStringLiteral("data"), Solid::OpticalDisc::Data},
        {QStringLiteral("vcd"), Solid::OpticalDisc::VideoCd},
        {QStringLiteral("svcd"), Solid::OpticalDisc::SuperVideoCd},
        {QStringLiteral("videodvd"), Solid::OpticalDisc::VideoDvd},
    };

    const QStringList content_typelist = fakeDevice()->property(QStringLiteral("availableContent")).toString().split(QLatin1Char(','));

    for (const QString &type : content_typelist) {
        content |= map.value(type, Solid::OpticalDisc::NoContent);
    }

    return content;
}

Solid::OpticalDisc::DiscType FakeOpticalDisc::discType() const
{
    QString type = fakeDevice()->property(QStringLiteral("discType")).toString();

    if (type == QLatin1String("cd_rom")) {
        return Solid::OpticalDisc::CdRom;
    } else if (type == QLatin1String("cd_r")) {
        return Solid::OpticalDisc::CdRecordable;
    } else if (type == QLatin1String("cd_rw")) {
        return Solid::OpticalDisc::CdRewritable;
    } else if (type == QLatin1String("dvd_rom")) {
        return Solid::OpticalDisc::DvdRom;
    } else if (type == QLatin1String("dvd_ram")) {
        return Solid::OpticalDisc::DvdRam;
    } else if (type == QLatin1String("dvd_r")) {
        return Solid::OpticalDisc::DvdRecordable;
    } else if (type == QLatin1String("dvd_rw")) {
        return Solid::OpticalDisc::DvdRewritable;
    } else if (type == QLatin1String("dvd_plus_r")) {
        return Solid::OpticalDisc::DvdPlusRecordable;
    } else if (type == QLatin1String("dvd_plus_rw")) {
        return Solid::OpticalDisc::DvdPlusRewritable;
    } else if (type == QLatin1String("dvd_plus_r_dl")) {
        return Solid::OpticalDisc::DvdPlusRecordableDuallayer;
    } else if (type == QLatin1String("dvd_plus_rw_dl")) {
        return Solid::OpticalDisc::DvdPlusRewritableDuallayer;
    } else if (type == QLatin1String("bd_rom")) {
        return Solid::OpticalDisc::BluRayRom;
    } else if (type == QLatin1String("bd_r")) {
        return Solid::OpticalDisc::BluRayRecordable;
    } else if (type == QLatin1String("bd_re")) {
        return Solid::OpticalDisc::BluRayRewritable;
    } else if (type == QLatin1String("hddvd_rom")) {
        return Solid::OpticalDisc::HdDvdRom;
    } else if (type == QLatin1String("hddvd_r")) {
        return Solid::OpticalDisc::HdDvdRecordable;
    } else if (type == QLatin1String("hddvd_rw")) {
        return Solid::OpticalDisc::HdDvdRewritable;
    } else {
        return Solid::OpticalDisc::UnknownDiscType;
    }
}

bool FakeOpticalDisc::isAppendable() const
{
    return fakeDevice()->property(QStringLiteral("isAppendable")).toBool();
}

bool FakeOpticalDisc::isBlank() const
{
    return fakeDevice()->property(QStringLiteral("isBlank")).toBool();
}

bool FakeOpticalDisc::isRewritable() const
{
    return fakeDevice()->property(QStringLiteral("isRewritable")).toBool();
}

qulonglong FakeOpticalDisc::capacity() const
{
    return fakeDevice()->property(QStringLiteral("capacity")).toULongLong();
}

#include "moc_fakeopticaldisc.cpp"
