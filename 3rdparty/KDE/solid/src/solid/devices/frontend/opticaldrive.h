/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_OPTICALDRIVE_H
#define SOLID_OPTICALDRIVE_H

#include <QList>
#include <QVariant>

#include <solid/solid_export.h>
#include <solid/solidnamespace.h>

#include <solid/storagedrive.h>

namespace Solid
{
class OpticalDrivePrivate;
class Device;

/*!
 * \class Solid::OpticalDrive
 * \inheaderfile Solid/OpticalDrive
 * \inmodule Solid
 *
 * \brief This device interface is available on CD-R*,DVD*,Blu-Ray,HD-DVD drives.
 *
 * An OpticalDrive is a storage that can handle optical discs.
 */
class SOLID_EXPORT OpticalDrive : public StorageDrive
{
    Q_OBJECT

    /*!
     * \property Solid::OpticalDrive::supportedMedia
     */
    Q_PROPERTY(MediumTypes supportedMedia READ supportedMedia)

    /*!
     * \property Solid::OpticalDrive::readSpeed
     */
    Q_PROPERTY(int readSpeed READ readSpeed)

    /*!
     * \property Solid::OpticalDrive::writeSpeed
     */
    Q_PROPERTY(int writeSpeed READ writeSpeed)

    /*!
     * \property Solid::OpticalDrive::writeSpeeds
     */
    Q_PROPERTY(QList<int> writeSpeeds READ writeSpeeds)

    Q_DECLARE_PRIVATE(OpticalDrive)
    friend class Device;

public:
    /*!
     * This enum type defines the type of medium an optical drive supports.
     *
     * \value UnknownMediumType
     * \value Cdr A Recordable Compact Disc (CD-R)
     * \value Cdrw A ReWritable Compact Disc (CD-RW)
     * \value Dvd A Digital Versatile Disc (DVD)
     * \value Dvdr A Recordable Digital Versatile Disc (DVD-R)
     * \value Dvdrw A ReWritable Digital Versatile Disc (DVD-RW)
     * \value Dvdram A Random Access Memory Digital Versatile Disc (DVD-RAM)
     * \value Dvdplusr A Recordable Digital Versatile Disc (DVD+R)
     * \value Dvdplusrw A ReWritable Digital Versatile Disc (DVD+RW)
     * \value Dvdplusdl A Dual Layer Digital Versatile Disc (DVD+R DL)
     * \value Dvdplusdlrw A Dual Layer Digital Versatile Disc (DVD+RW DL)
     * \value Bd A Blu-ray Disc (BD)
     * \value Bdr A Blu-ray Disc Recordable (BD-R)
     * \value Bdre A Blu-ray Disc Recordable and Eraseable (BD-RE)
     * \value HdDvd A High Density Digital Versatile Disc (HD DVD)
     * \value HdDvdr A High Density Digital Versatile Disc Recordable (HD DVD-R)
     * \value HdDvdrw A High Density Digital Versatile Disc ReWritable (HD DVD-RW)
     */
    enum MediumType {
        UnknownMediumType = 0x00000,
        Cdr = 0x00001,
        Cdrw = 0x00002,
        Dvd = 0x00004,
        Dvdr = 0x00008,
        Dvdrw = 0x00010,
        Dvdram = 0x00020,
        Dvdplusr = 0x00040,
        Dvdplusrw = 0x00080,
        Dvdplusdl = 0x00100,
        Dvdplusdlrw = 0x00200,
        Bd = 0x00400,
        Bdr = 0x00800,
        Bdre = 0x01000,
        HdDvd = 0x02000,
        HdDvdr = 0x04000,
        HdDvdrw = 0x08000,
    };
    Q_ENUM(MediumType)

    Q_DECLARE_FLAGS(MediumTypes, MediumType)
    Q_FLAG(MediumTypes)

private:
    /*!
     * \internal
     * Creates a new OpticalDrive object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit OpticalDrive(QObject *backendObject);

public:
    ~OpticalDrive() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the OpticalDrive device interface.
     *
     * Returns the OpticalDrive device interface type
     * \sa Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::OpticalDrive;
    }

    /*!
     * Retrieves the medium types this drive supports.
     *
     * Returns the flag set indicating the supported medium types
     */
    MediumTypes supportedMedia() const;

    /*!
     * Retrieves the maximum read speed of this drive in kilobytes per second.
     *
     * Returns the maximum read speed
     */
    int readSpeed() const;

    /*!
     * Retrieves the maximum write speed of this drive in kilobytes per second.
     *
     * Returns the maximum write speed
     */
    int writeSpeed() const;

    /*!
     * Retrieves the list of supported write speeds of this drive in
     * kilobytes per second.
     *
     * Returns the list of supported write speeds
     */
    QList<int> writeSpeeds() const;

    /*!
     * Ejects any disc that could be contained in this drive.
     * If this drive is empty, but has a tray it'll be opened.
     *
     * Returns the status of the eject operation
     */
    bool eject();

Q_SIGNALS:
    /*!
     * This signal is emitted when the eject button is pressed
     * on the drive.
     *
     * Please note that some (broken) drives doesn't report this event.
     *
     * \a udi the UDI of the drive
     */
    void ejectPressed(const QString &udi);

    /*!
     * This signal is emitted when the attempted eject process on this
     * drive is completed. The signal might be spontaneous, i.e.
     * it can be triggered by another process.
     *
     * \a error type of error that occurred, if any
     *
     * \a errorData more information about the error, if any
     *
     * \a udi the UDI of the volume
     */
    void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    /*!
     * This signal is emitted when eject on this drive is
     * requested. The signal might be spontaneous, i.e. it
     * can be triggered by another process.
     *
     * \a udi the UDI of the volume
     */
    void ejectRequested(const QString &udi);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OpticalDrive::MediumTypes)

}

#endif // SOLID_OPTICALDRIVE_H
