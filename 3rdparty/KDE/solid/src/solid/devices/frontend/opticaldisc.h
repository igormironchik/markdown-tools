/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_OPTICALDISC_H
#define SOLID_OPTICALDISC_H

#include <solid/solid_export.h>

#include <solid/storagevolume.h>

namespace Solid
{
class OpticalDiscPrivate;
class Device;

/*!
 * \class Solid::OpticalDisc
 * \inheaderfile Solid/OpticalDisc
 * \inmodule Solid
 *
 * \brief This device interface is available on optical discs.
 *
 * An optical disc is a volume that can be inserted in CD-R*,DVD*,Blu-Ray,HD-DVD drives.
 */
class SOLID_EXPORT OpticalDisc : public StorageVolume
{
    Q_OBJECT

    /*!
     * \property Solid::OpticalDisc::availableContent
     */
    Q_PROPERTY(ContentTypes availableContent READ availableContent)

    /*!
     * \property Solid::OpticalDisc::discType
     */
    Q_PROPERTY(DiscType discType READ discType)

    /*!
     * \property Solid::OpticalDisc::appendable
     */
    Q_PROPERTY(bool appendable READ isAppendable)

    /*!
     * \property Solid::OpticalDisc::blank
     */
    Q_PROPERTY(bool blank READ isBlank)

    /*!
     * \property Solid::OpticalDisc::rewritable
     */
    Q_PROPERTY(bool rewritable READ isRewritable)

    /*!
     * \property Solid::OpticalDisc::capacity
     */
    Q_PROPERTY(qulonglong capacity READ capacity)

    Q_DECLARE_PRIVATE(OpticalDisc)
    friend class Device;

public:
    /*!
     * This enum type defines the type of content available in an optical disc.
     *
     * \value NoContent
     * \value Audio A disc containing audio
     * \value Data A disc containing data
     * \value VideoCd A Video Compact Disc (VCD)
     * \value SuperVideoCd A Super Video Compact Disc (SVCD)
     * \value VideoDvd A Video Digital Versatile Disc (DVD-Video)
     * \value VideoBluRay
     */
    enum ContentType {
        NoContent = 0x00,
        Audio = 0x01,
        Data = 0x02,
        VideoCd = 0x04,
        SuperVideoCd = 0x08,
        VideoDvd = 0x10,
        VideoBluRay = 0x20,
    };
    Q_ENUM(ContentType)

    Q_DECLARE_FLAGS(ContentTypes, ContentType)
    Q_FLAG(ContentTypes)

    /*!
     * This enum type defines the type of optical disc it can be.
     *
     * \value UnknownDiscType An undetermined disc type
     * \value CdRom A Compact Disc Read-Only Memory (CD-ROM)
     * \value CdRecordable A Compact Disc Recordable (CD-R)
     * \value CdRewritable A Compact Disc ReWritable (CD-RW)
     * \value DvdRom A Digital Versatile Disc Read-Only Memory (DVD-ROM)
     * \value DvdRam A Digital Versatile Disc Random Access Memory (DVD-RAM)
     * \value DvdRecordable A Digital Versatile Disc Recordable (DVD-R)
     * \value DvdRewritable A Digital Versatile Disc ReWritable (DVD-RW)
     * \value DvdPlusRecordable A Digital Versatile Disc Recordable (DVD+R)
     * \value DvdPlusRewritable A Digital Versatile Disc ReWritable (DVD+RW)
     * \value DvdPlusRecordableDuallayer A Digital Versatile Disc Recordable Dual-Layer (DVD+R DL)
     * \value DvdPlusRewritableDuallayer A Digital Versatile Disc ReWritable Dual-Layer (DVD+RW DL)
     * \value BluRayRom A Blu-ray Disc (BD)
     * \value BluRayRecordable A Blu-ray Disc Recordable (BD-R)
     * \value BluRayRewritable A Blu-ray Disc (BD-RE)
     * \value HdDvdRom: A High Density Digital Versatile Disc (HD DVD)
     * \value HdDvdRecordable A High Density Digital Versatile Disc Recordable (HD DVD-R)
     * \value HdDvdRewritable A High Density Digital Versatile Disc ReWritable (HD DVD-RW)
     */
    enum DiscType {
        UnknownDiscType = -1,
        CdRom,
        CdRecordable,
        CdRewritable,
        DvdRom,
        DvdRam,
        DvdRecordable,
        DvdRewritable,
        DvdPlusRecordable,
        DvdPlusRewritable,
        DvdPlusRecordableDuallayer,
        DvdPlusRewritableDuallayer,
        BluRayRom,
        BluRayRecordable,
        BluRayRewritable,
        HdDvdRom,
        HdDvdRecordable,
        HdDvdRewritable,
    };
    Q_ENUM(DiscType)

private:
    /*!
     * \internal
     * Creates a new OpticalDisc object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * \a backendObject the device interface object provided by the backend
     * \sa Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit OpticalDisc(QObject *backendObject);

public:
    ~OpticalDisc() override;

    /*!
     * Get the Solid::DeviceInterface::Type of the OpticalDisc device interface.
     *
     * Returns the OpticalDisc device interface type
     * \sa Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::OpticalDisc;
    }

    /*!
     * Retrieves the content types this disc contains (audio, video,
     * data...).
     *
     * Returns the flag set indicating the available contents
     * \sa Solid::OpticalDisc::ContentType
     */
    ContentTypes availableContent() const;

    /*!
     * Retrieves the disc type (cdr, cdrw...).
     *
     * Returns the disc type
     */
    DiscType discType() const;

    /*!
     * Indicates if it's possible to write additional data to the disc.
     *
     * Returns true if the disc is appendable, false otherwise
     */
    bool isAppendable() const;

    /*!
     * Indicates if the disc is blank.
     *
     * Returns true if the disc is blank, false otherwise
     */
    bool isBlank() const;

    /*!
     * Indicates if the disc is rewritable.
     *
     * A disc is rewritable if you can write on it several times.
     *
     * Returns true if the disc is rewritable, false otherwise
     */
    bool isRewritable() const;

    /*!
     * Retrieves the disc capacity (that is the maximum size of a
     * volume could have on this disc).
     *
     * Returns the capacity of the disc in bytes
     */
    qulonglong capacity() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(OpticalDisc::ContentTypes)

}

#endif
