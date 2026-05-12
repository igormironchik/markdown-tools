/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakestorageaccess.h"

using namespace Solid::Backends::Fake;

FakeStorageAccess::FakeStorageAccess(FakeDevice *device)
    : FakeDeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(QMap<QString, int>)), this, SLOT(onPropertyChanged(QMap<QString, int>)));
}

FakeStorageAccess::~FakeStorageAccess()
{
}

bool FakeStorageAccess::isAccessible() const
{
    return fakeDevice()->property(QStringLiteral("isMounted")).toBool();
}

QString FakeStorageAccess::filePath() const
{
    return fakeDevice()->property(QStringLiteral("mountPoint")).toString();
}

bool FakeStorageAccess::isIgnored() const
{
    return fakeDevice()->property(QStringLiteral("isIgnored")).toBool();
}

bool FakeStorageAccess::isEncrypted() const
{
    return fakeDevice()->property(QStringLiteral("isEncrypted")).toBool();
}

bool FakeStorageAccess::setup()
{
    if (fakeDevice()->isBroken() || isAccessible()) {
        return false;
    } else {
        fakeDevice()->setProperty(QStringLiteral("isMounted"), true);
        return true;
    }
}

bool FakeStorageAccess::teardown()
{
    if (fakeDevice()->isBroken() || !isAccessible()) {
        return false;
    } else {
        fakeDevice()->setProperty(QStringLiteral("isMounted"), false);
        return true;
    }
}

void Solid::Backends::Fake::FakeStorageAccess::onPropertyChanged(const QMap<QString, int> &changes)
{
    for (auto it = changes.cbegin(); it != changes.cend(); ++it) {
        if (it.key() == QLatin1String(QLatin1String("isMounted"))) {
            Q_EMIT accessibilityChanged(fakeDevice()->property(QStringLiteral("isMounted")).toBool(), fakeDevice()->udi());
        }
    }
}

#include "moc_fakestorageaccess.cpp"
