/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "fakehardwaretest.h"

// Qt includes
#include <QTest>

// Solid includes
#include <solid/devices/ifaces/device.h>
#include <solid/devices/ifaces/deviceinterface.h>
#include <solid/devices/ifaces/processor.h>

// Local includes
#include "solid/devices/backends/fakehw/fakedevice.h"
#include "solid/devices/backends/fakehw/fakemanager.h"

QTEST_MAIN(FakeHardwareTest)

void FakeHardwareTest::testFakeBackend()
{
    Solid::Backends::Fake::FakeManager *fakeManager = new Solid::Backends::Fake::FakeManager(nullptr, QStringLiteral(TEST_DATA));

    QVERIFY(!fakeManager->allDevices().isEmpty());
    auto computer = fakeManager->createDevice(QStringLiteral("/org/kde/solid/fakehw/computer"));
    QVERIFY(computer != nullptr);
    QVERIFY(fakeManager->createDevice(QStringLiteral("/com/helloworld/troll/compiutor")) == nullptr);

    auto d = fakeManager->createDevice(QStringLiteral("/org/kde/solid/fakehw/acpi_CPU0"));
    auto *device = static_cast<Solid::Backends::Fake::FakeDevice *>(d.get());

    QCOMPARE(device->udi(), QStringLiteral("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(device->parentUdi(), QStringLiteral("/org/kde/solid/fakehw/computer"));
    QCOMPARE(device->vendor(), QStringLiteral("Acme Corporation"));
    QCOMPARE(device->product(), QStringLiteral("Solid Processor #0"));

    QCOMPARE(device->property(QStringLiteral("number")).toString(), QStringLiteral("0"));
    QVERIFY(device->propertyExists(QStringLiteral("number")));
    QVERIFY(!device->propertyExists(QStringLiteral("youstfuqewerrernoob")));

    QVERIFY(device->queryDeviceInterface(Solid::DeviceInterface::Processor));

    QObject *interface = device->createDeviceInterface(Solid::DeviceInterface::Processor);
    Solid::Ifaces::Processor *processor = qobject_cast<Solid::Ifaces::Processor *>(interface);

    QCOMPARE(processor->number(), 0);
    QCOMPARE(processor->canChangeFrequency(), true);
    QCOMPARE((int)processor->maxSpeed(), 3200);

    Solid::Processor::InstructionSets instructionsets;
    instructionsets |= Solid::Processor::IntelMmx;
    instructionsets |= Solid::Processor::IntelSse;
    QCOMPARE(processor->instructionSets(), instructionsets);

    delete processor;
    delete fakeManager;
}

#include "moc_fakehardwaretest.cpp"
