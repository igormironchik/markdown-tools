/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakebattery.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeBattery::FakeBattery(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeBattery::~FakeBattery()
{
}

bool FakeBattery::isPresent() const
{
    return fakeDevice()->property(QStringLiteral("isPresent")).toBool();
}

Solid::Battery::BatteryType FakeBattery::type() const
{
    QString name = fakeDevice()->property(QStringLiteral("batteryType")).toString();

    if (name == QLatin1String("pda")) {
        return Solid::Battery::PdaBattery;
    } else if (name == QLatin1String("ups")) {
        return Solid::Battery::UpsBattery;
    } else if (name == QLatin1String("primary")) {
        return Solid::Battery::PrimaryBattery;
    } else if (name == QLatin1String("mouse")) {
        return Solid::Battery::MouseBattery;
    } else if (name == QLatin1String("keyboard")) {
        return Solid::Battery::KeyboardBattery;
    } else if (name == QLatin1String("keyboard_mouse")) {
        return Solid::Battery::KeyboardMouseBattery;
    } else if (name == QLatin1String("camera")) {
        return Solid::Battery::CameraBattery;
    } else if (name == QLatin1String("gaminginput")) {
        return Solid::Battery::GamingInputBattery;
    } else if (name == QLatin1String("bluetooth")) {
        return Solid::Battery::BluetoothBattery;
    } else if (name == QLatin1String("tablet")) {
        return Solid::Battery::TabletBattery;
    } else {
        return Solid::Battery::UnknownBattery;
    }
}

int FakeBattery::chargePercent() const
{
    int last_full = fakeDevice()->property(QStringLiteral("lastFullLevel")).toInt();
    int current = fakeDevice()->property(QStringLiteral("currentLevel")).toInt();

    int percent = 0;
    if (last_full > 0) {
        percent = (100 * current) / last_full;
    }

    return percent;
}

int FakeBattery::capacity() const
{
    return fakeDevice()->property(QStringLiteral("capacity")).toInt();
}

int FakeBattery::cycleCount() const
{
    return fakeDevice()->property(QStringLiteral("cycleCount")).toInt();
}

bool FakeBattery::isRechargeable() const
{
    return fakeDevice()->property(QStringLiteral("isRechargeable")).toBool();
}

bool FakeBattery::isPowerSupply() const
{
    return fakeDevice()->property(QStringLiteral("isPowerSupply")).toBool();
}

Solid::Battery::ChargeState FakeBattery::chargeState() const
{
    QString state = fakeDevice()->property(QStringLiteral("chargeState")).toString();

    if (state == QLatin1String("charging")) {
        return Solid::Battery::Charging;
    } else if (state == QLatin1String("discharging")) {
        return Solid::Battery::Discharging;
    } else if (state == QLatin1String("fullyCharged")) {
        return Solid::Battery::FullyCharged;
    } else {
        return Solid::Battery::NoCharge;
    }
}

qlonglong FakeBattery::timeToEmpty() const
{
    return fakeDevice()->property(QStringLiteral("timeToEmpty")).toLongLong();
}

qlonglong FakeBattery::timeToFull() const
{
    return fakeDevice()->property(QStringLiteral("timeToFull")).toLongLong();
}

void FakeBattery::setChargeState(Solid::Battery::ChargeState newState)
{
    QString name;

    switch (newState) {
    case Solid::Battery::Charging:
        name = QStringLiteral("charging");
        break;
    case Solid::Battery::Discharging:
        name = QStringLiteral("discharging");
        break;
    case Solid::Battery::NoCharge:
        name = QStringLiteral("noCharge");
        break;
    case Solid::Battery::FullyCharged:
        name = QStringLiteral("fullyCharged");
        break;
    }

    fakeDevice()->setProperty(QStringLiteral("chargeState"), name);
    Q_EMIT chargeStateChanged(newState, fakeDevice()->udi());
}

void FakeBattery::setChargeLevel(int newLevel)
{
    fakeDevice()->setProperty(QStringLiteral("currentLevel"), newLevel);
    Q_EMIT chargePercentChanged(chargePercent(), fakeDevice()->udi());
}

Solid::Battery::Technology FakeBattery::technology() const
{
    return (Solid::Battery::Technology)fakeDevice()->property(QStringLiteral("technology")).toInt();
}

double FakeBattery::energy() const
{
    return fakeDevice()->property(QStringLiteral("energy")).toDouble();
}

double FakeBattery::energyFull() const
{
    return fakeDevice()->property(QStringLiteral("energyFull")).toDouble();
}

double FakeBattery::energyFullDesign() const
{
    return fakeDevice()->property(QStringLiteral("energyFullDesign")).toDouble();
}

double FakeBattery::energyRate() const
{
    return fakeDevice()->property(QStringLiteral("energyRate")).toDouble();
}

double FakeBattery::voltage() const
{
    return fakeDevice()->property(QStringLiteral("voltage")).toDouble();
}

double FakeBattery::temperature() const
{
    return fakeDevice()->property(QStringLiteral("temperature")).toDouble();
}

QString FakeBattery::serial() const
{
    return fakeDevice()->property(QStringLiteral("serial")).toString();
}

qlonglong FakeBattery::remainingTime() const
{
    return fakeDevice()->property(QStringLiteral("remainingTime")).toLongLong();
}

#include "moc_fakebattery.cpp"
