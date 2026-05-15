/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeblock.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeBlock::FakeBlock(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeBlock::~FakeBlock()
{
}

int FakeBlock::deviceMajor() const
{
    return fakeDevice()->property(QStringLiteral("major")).toInt();
}

int FakeBlock::deviceMinor() const
{
    return fakeDevice()->property(QStringLiteral("minor")).toInt();
}

QString FakeBlock::device() const
{
    return fakeDevice()->property(QStringLiteral("device")).toString();
}

#include "moc_fakeblock.cpp"
