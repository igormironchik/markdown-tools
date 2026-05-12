/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_PROCESSOR_H
#define SOLID_PROCESSOR_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class ProcessorPrivate;
class Device;

/*!
 * \class Solid::Processor
 * \inheaderfile Solid/Processor
 * \inmodule Solid
 *
 * \brief This device interface is available on processors.
 */
class SOLID_EXPORT Processor : public DeviceInterface
{
    Q_OBJECT

    /*!
     * \property Solid::Processor::number
     */
    Q_PROPERTY(int number READ number)

    /*!
     * \property Solid::Processor::maxSpeed
     */
    Q_PROPERTY(qulonglong maxSpeed READ maxSpeed)

    /*!
     * \property Solid::Processor::canChangeFrequency
     */
    Q_PROPERTY(bool canChangeFrequency READ canChangeFrequency)

    /*!
     * \property Solid::Processor::instructionSets
     */
    Q_PROPERTY(InstructionSets instructionSets READ instructionSets)

    Q_DECLARE_PRIVATE(Processor)
    friend class Device;

private:
    /*!
     * \internal
     * Creates a new Processor object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit Processor(QObject *backendObject);

public:
    /*!
     * This enum contains the list of architecture extensions you
     * can query.
     *
     * \value NoExtensions
     * \value IntelMmx
     * \value IntelSse
     * \value IntelSse2
     * \value IntelSse3
     * \value IntelSsse3
     * \value IntelSse4
     * \value IntelSse41
     * \value IntelSse42
     * \value Amd3DNow
     * \value AltiVec
     */
    enum InstructionSet {
        NoExtensions = 0x0,
        IntelMmx = 0x1,
        IntelSse = 0x2,
        IntelSse2 = 0x4,
        IntelSse3 = 0x8,
        IntelSsse3 = 0x80,
        IntelSse4 = 0x10,
        IntelSse41 = 0x10,
        IntelSse42 = 0x100,
        Amd3DNow = 0x20,
        AltiVec = 0x40,
    };
    Q_ENUM(InstructionSet)

    Q_DECLARE_FLAGS(InstructionSets, InstructionSet)
    Q_FLAG(InstructionSets)

    ~Processor() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the Processor device interface.
     *
     * Returns the Processor device interface type
     * \sa Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::Processor;
    }

    /*!
     * Retrieves the processor number in the system.
     *
     * Returns the internal processor number in the system, starting from zero
     */
    int number() const;

    /*!
     * Retrieves the maximum speed of the processor.
     *
     * Returns the maximum speed in MHz, or 0 if the device can't be queried for this
     * information.
     */
    int maxSpeed() const;

    /*!
     * Indicates if the processor can change the CPU frequency.
     *
     * True if a processor is able to change its own CPU frequency.
     *  (generally for power management).
     *
     * Returns true if the processor can change CPU frequency, false otherwise
     */
    bool canChangeFrequency() const;

    /*!
     * Queries the instructions set extensions of the CPU.
     *
     * Returns the extensions supported by the CPU
     * \sa Solid::Processor::InstructionSet
     */
    InstructionSets instructionSets() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Processor::InstructionSets)

}

#endif
