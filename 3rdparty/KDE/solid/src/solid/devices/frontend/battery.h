/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BATTERY_H
#define SOLID_BATTERY_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
class BatteryPrivate;
class Device;

/*!
 * \class Solid::Battery
 * \inheaderfile Solid/Battery
 * \inmodule Solid
 *
 * \brief This device interface is available on batteries.
 */
class SOLID_EXPORT Battery : public DeviceInterface
{
    Q_OBJECT

    /*!
     * \property Solid::Battery::present
     */
    Q_PROPERTY(bool present READ isPresent NOTIFY presentStateChanged)

    /*!
     * \property Solid::Battery::type
     */
    Q_PROPERTY(BatteryType type READ type CONSTANT)

    /*!
     * \property Solid::Battery::chargePercent
     */
    Q_PROPERTY(int chargePercent READ chargePercent NOTIFY chargePercentChanged)

    /*!
     * \property Solid::Battery::capacity
     */
    Q_PROPERTY(int capacity READ capacity NOTIFY capacityChanged)

    /*!
     * \property Solid::Battery::cycleCount
     */
    Q_PROPERTY(int cycleCount READ cycleCount NOTIFY cycleCountChanged)

    /*!
     * \property Solid::Battery::rechargeable
     */
    Q_PROPERTY(bool rechargeable READ isRechargeable CONSTANT)

    /*!
     * \property Solid::Battery::powerSupply
     */
    Q_PROPERTY(bool powerSupply READ isPowerSupply NOTIFY powerSupplyStateChanged)

    /*!
     * \property Solid::Battery::chargeState
     */
    Q_PROPERTY(ChargeState chargeState READ chargeState NOTIFY chargeStateChanged)

    /*!
     * \property Solid::Battery::timeToEmpty
     */
    Q_PROPERTY(qlonglong timeToEmpty READ timeToEmpty NOTIFY timeToEmptyChanged)

    /*!
     * \property Solid::Battery::timeToFull
     */
    Q_PROPERTY(qlonglong timeToFull READ timeToFull NOTIFY timeToFullChanged)

    /*!
     * \property Solid::Battery::energy
     */
    Q_PROPERTY(double energy READ energy NOTIFY energyChanged)

    /*!
     * \property Solid::Battery::energyFull
     */
    Q_PROPERTY(double energyFull READ energyFull NOTIFY energyFullChanged)

    /*!
     * \property Solid::Battery::energyFullDesign
     */
    Q_PROPERTY(double energyFullDesign READ energyFullDesign NOTIFY energyFullDesignChanged)

    /*!
     * \property Solid::Battery::energyRate
     */
    Q_PROPERTY(double energyRate READ energyRate NOTIFY energyRateChanged)

    /*!
     * \property Solid::Battery::voltage
     */
    Q_PROPERTY(double voltage READ voltage NOTIFY voltageChanged)

    /*!
     * \property Solid::Battery::temperature
     */
    Q_PROPERTY(double temperature READ temperature NOTIFY temperatureChanged)

    /*!
     * \property Solid::Battery::technology
     */
    Q_PROPERTY(Technology technology READ technology CONSTANT)

    /*!
     * \property Solid::Battery::serial
     */
    Q_PROPERTY(QString serial READ serial CONSTANT)

    /*!
     * \property Solid::Battery::remainingTime
     */
    Q_PROPERTY(qlonglong remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    Q_DECLARE_PRIVATE(Battery)
    friend class Device;

public:
    /*!
     * This enum type defines the type of the device holding the battery
     *
     * \value PdaBattery A battery in a Personal Digital Assistant
     * \value UpsBattery A battery in an Uninterruptible Power Supply
     * \value PrimaryBattery A primary battery for the system (for example laptop battery)
     * \value MouseBattery A battery in a mouse
     * \value KeyboardBattery A battery in a keyboard
     * \value KeyboardMouseBattery A battery in a combined keyboard and mouse
     * \value CameraBattery A battery in a camera
     * \value PhoneBattery A battery in a phone
     * \value MonitorBattery A battery in a monitor
     * \value GamingInputBattery A battery in a gaming input device (for example a wireless game pad)
     * \value[since 5.54] BluetoothBattery A generic Bluetooth device battery (if its type isn't known, a Bluetooth mouse would normally show up as a
     * MouseBattery)
     * \value[since 5.88] TabletBattery A battery in a graphics tablet or pen
     * \value[since 6.0] HeadphoneBattery A battery in a headphone
     * \value[since 6.0] HeadsetBattery A battery in a headset
     * \value[since 6.0] TouchpadBattery A battery in a touchpad. This is how the Dualsense Wireless Controller is categorized
     * \value[since 6.21] WearableBattery A battery for a wearable device, watch...
     * \value UnknownBattery A battery in an unknown device
     */
    enum BatteryType {
        UnknownBattery,
        PdaBattery,
        UpsBattery,
        PrimaryBattery,
        MouseBattery,
        KeyboardBattery,
        KeyboardMouseBattery,
        CameraBattery,
        PhoneBattery,
        MonitorBattery,
        GamingInputBattery,
        BluetoothBattery,
        TabletBattery,
        HeadphoneBattery,
        HeadsetBattery,
        TouchpadBattery,
        WearableBattery,
    };
    Q_ENUM(BatteryType)

    /*!
     * This enum type defines charge state of a battery
     *
     * \value NoCharge Battery charge is stable, not charging or discharging or the state is Unknown
     * \value Charging Battery is charging
     * \value Discharging Battery is discharging
     * \value FullyCharged The battery is fully charged; a battery not necessarily charges up to 100%
     */
    enum ChargeState { NoCharge, Charging, Discharging, FullyCharged };
    Q_ENUM(ChargeState)

    /*!
     * Technology used in the battery
     *
     * \value UnknownTechnology Unknown
     * \value LithiumIon Lithium ion
     * \value LithiumPolymer Lithium polymer
     * \value LithiumIronPhosphate Lithium iron phosphate
     * \value LeadAcid Lead acid
     * \value NickelCadmium Nickel cadmium
     * \value NickelMetalHydride Nickel metal hydride
     */
    enum Technology {
        UnknownTechnology = 0,
        LithiumIon,
        LithiumPolymer,
        LithiumIronPhosphate,
        LeadAcid,
        NickelCadmium,
        NickelMetalHydride,
    };
    Q_ENUM(Technology)

private:
    /*!
     * Creates a new Battery object.
     *
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit Battery(QObject *backendObject);

public:
    ~Battery() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the Battery device interface.
     * \sa Solid::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::Battery;
    }

    /*!
     * Indicates if this battery is currently present in its bay.
     *
     * Returns \c true if the battery is present, \c false otherwise
     */
    bool isPresent() const;

    /*!
     * Returns the type of device holding this battery.
     *
     * \sa Solid::Battery::BatteryType
     */
    Solid::Battery::BatteryType type() const;

    /*!
     * Returns the current charge level of the battery normalised
     * to percent.
     */
    int chargePercent() const;

    /*!
     * Returns the battery capacity normalised to percent,
     * meaning how much energy can it hold compared to what it is designed to.
     *
     * The capacity of the battery will reduce with age.
     * A capacity value less than 75% is usually a sign that you should renew your battery.
     *
     * \since 4.11
     */
    int capacity() const;

    /*!
     * Retrieves the number of charge cycles this battery has experienced so far,
     * or -1 if this information is unavailable.
     *
     * Returns the number of charge cycles
     * \since 6.9
     */
    int cycleCount() const;

    /*!
     * Indicates if the battery is rechargeable.
     */
    bool isRechargeable() const;

    /*!
     * Indicates if the battery is powering the machine.
     */
    bool isPowerSupply() const;

    /*!
     * Retrieves the current charge state of the battery. It can be in a stable
     * state (no charge), charging or discharging.
     *
     * \sa Solid::Battery::ChargeState
     */
    Solid::Battery::ChargeState chargeState() const;

    /*!
     * Time (in seconds) until the battery is empty.
     *
     * \since 5.0
     */
    qlonglong timeToEmpty() const;

    /*!
     * Time (in seconds) until the battery is full.
     *
     * \since 5.0
     */
    qlonglong timeToFull() const;

    /*!
     * Retrieves the technology used to manufacture the battery.
     *
     * \sa Solid::Battery::Technology
     */
    Solid::Battery::Technology technology() const;

    /*!
     * Amount of energy (measured in Wh) currently available in the power source.
     */
    double energy() const;

    /*!
     * Amount of energy (measured in Wh) the battery has when it is full.
     * \since 5.7
     */
    double energyFull() const;

    /*!
     * Amount of energy (measured in Wh) the battery should have by design hen it is full.
     *
     * Returns amount of battery energy when full by design in Wh
     * \since 5.7
     */
    double energyFullDesign() const;

    /*!
     * Amount of energy being drained from the source, measured in W.
     * If positive, the source is being discharged, if negative it's being charged.
     */
    double energyRate() const;

    /*!
     * Voltage in the Cell or being recorded by the meter.
     */
    double voltage() const;

    /*!
     * The temperature of the battery in degrees Celsius.
     * \since 5.0
     */
    double temperature() const;

    /*!
     * The serial number of the battery
     * \since 5.0
     */
    QString serial() const;

    /*!
     * Retrieves the current estimated remaining time (in seconds) of the system batteries
     * \since 5.8
     */
    qlonglong remainingTime() const;

Q_SIGNALS:
    /*!
     * This signal is emitted if the battery gets plugged in/out of the
     * battery bay.
     *
     * \a newState the new plugging state of the battery
     *
     * \a udi the UDI of the battery with thew new plugging state
     */
    void presentStateChanged(bool newState, const QString &udi);

    /*!
     * This signal is emitted when the charge percent value of this
     * battery has changed.
     *
     * \a value the new charge percent value of the battery
     *
     * \a udi the UDI of the battery with the new charge percent
     */
    void chargePercentChanged(int value, const QString &udi);

    /*!
     * This signal is emitted when the capacity of this battery has changed.
     *
     * \a value the new capacity of the battery
     *
     * \a udi the UDI of the battery with the new capacity
     * \since 4.11
     */
    void capacityChanged(int value, const QString &udi);

    /*!
     * This signal is emitted when the number of charge cycles of the
     * battery has changed.
     *
     * \a value the new number of charge cycles of the battery
     *
     * \a udi the UDI of the battery with the new number of charge cycles
     *
     * \since 6.9
     */
    void cycleCountChanged(int value, const QString &udi);

    /*!
     * This signal is emitted when the power supply state of the battery
     * changes.
     *
     * \a newState the new power supply state
     *
     * \a udi the UDI of the battery with the new power supply state
     * \since 4.11
     */
    void powerSupplyStateChanged(bool newState, const QString &udi);

    /*!
     * This signal is emitted when the charge state of this battery
     * has changed.
     *
     * \a newState the new charge state of the battery
     *
     * \a udi the UDI of the battery with the new charge state
     */
    void chargeStateChanged(int newState, const QString &udi = QString());

    /*!
     * This signal is emitted when the time until the battery is empty
     * has changed.
     *
     * \a time the new remaining time
     *
     * \a udi the UDI of the battery with the new remaining time
     * \since 5.0
     */
    void timeToEmptyChanged(qlonglong time, const QString &udi);

    /*!
     * This signal is emitted when the time until the battery is full
     * has changed.
     *
     * \a time the new remaining time
     *
     * \a udi the UDI of the battery with the new remaining time
     * \since 5.0
     */
    void timeToFullChanged(qlonglong time, const QString &udi);

    /*!
     * This signal is emitted when the energy value of this
     * battery has changed.
     *
     * \a energy the new energy value of the battery
     *
     * \a udi the UDI of the battery with the new energy value
     */
    void energyChanged(double energy, const QString &udi);

    /*!
     * This signal is emitted when the energy full value of this
     * battery has changed.
     *
     * \a energy the new energy full value of the battery
     *
     * \a udi the UDI of the battery with the new energy full value
     */
    void energyFullChanged(double energy, const QString &udi);

    /*!
     * This signal is emitted when the energy full design value of this
     * battery has changed.
     *
     * \a energy the new energy full design value of the battery
     *
     * \a udi the UDI of the battery with the new energy full design value
     */
    void energyFullDesignChanged(double energy, const QString &udi);

    /*!
     * This signal is emitted when the energy rate value of this
     * battery has changed.
     *
     * If positive, the source is being discharged, if negative it's being charged.
     *
     * \a energyRate the new energy rate value of the battery
     *
     * \a udi the UDI of the battery with the new charge percent
     */
    void energyRateChanged(double energyRate, const QString &udi);

    /*!
     * This signal is emitted when the voltage in the cell has changed.
     *
     * \a voltage the new voltage of the cell
     *
     * \a udi the UDI of the battery with the new voltage
     * \since 5.0
     */
    void voltageChanged(double voltage, const QString &udi);

    /*!
     * This signal is emitted when the battery temperature has changed.
     *
     * \a temperature the new temperature of the battery in degrees Celsius
     *
     * \a udi the UDI of the battery with the new temperature
     * \since 5.0
     */
    void temperatureChanged(double temperature, const QString &udi);

    /*!
     * This signal is emitted when the estimated battery remaining time changes.
     *
     * \a time the new remaining time
     *
     * \a udi the UDI of the battery with the new remaining time
     * \since 5.8
     */
    void remainingTimeChanged(qlonglong time, const QString &udi);
};
}

#endif
