/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2011 Matej Laitl <matej@laitl.cz>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevportablemediaplayer.h"

#include <QChar>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

using namespace Solid::Backends::UDev;

/*!
 * Reads one value from media-player-info ini-like file.
 *
 * @param file file to open. May advance current seek position
 * @param group group name to read from, e.g. "Device" for [Device] group
 * @param key key name, e.g. "AccessProtocol"
 * @return value as a string or an empty string
 */
static QString readMpiValue(QIODevice &file, const QString &group, const QString &key)
{
    QTextStream mpiStream(&file);
    QString line;
    QString currGroup;

    while (!mpiStream.atEnd()) {
        line = mpiStream.readLine().trimmed(); // trimmed is needed for possible indentation
        if (line.isEmpty() || line.startsWith(QLatin1Char(';'))) {
            // skip empty and comment lines
        } else if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            currGroup = line.mid(1, line.length() - 2); // strip [ and ]
        } else if (line.indexOf(QLatin1Char('=')) != -1) {
            int index = line.indexOf(QLatin1Char('='));
            if (currGroup == group && line.left(index) == key) {
                line = line.right(line.length() - index - 1);
                if (line.startsWith(QLatin1Char('"')) && line.endsWith(QLatin1Char('"'))) {
                    line = line.mid(1, line.length() - 2); // strip enclosing double quotes
                }
                return line;
            }
        } else {
            qWarning() << "readMpiValue: cannot parse line:" << line;
        }
    }
    return QString();
}

PortableMediaPlayer::PortableMediaPlayer(UDevDevice *device)
    : DeviceInterface(device)
{
}

PortableMediaPlayer::~PortableMediaPlayer()
{
}

QStringList PortableMediaPlayer::supportedProtocols() const
{
    /* There are multiple packages that set ID_MEDIA_PLAYER:
     *  * gphoto2 sets it to numeric 1 (for _some_ cameras it supports) and it hopefully
     *    means MTP-compatible device.
     *  * libmtp >= 1.0.4 sets it to numeric 1 and this always denotes MTP-compatible player.
     *  * media-player-info sets it to a string that denotes a name of the .mpi file with
     *    additional info.
     */
    if (m_device->property(QStringLiteral("ID_MEDIA_PLAYER")).toInt() == 1) {
        return {QStringLiteral("mtp")};
    }

    QString mpiFileName = mediaPlayerInfoFilePath();
    if (mpiFileName.isEmpty()) {
        return QStringList();
    }
    // we unfornutately cannot use QSettings as it cannot read unquoted valued with semicolons in it
    QFile mpiFile(mpiFileName);
    if (!mpiFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open" << mpiFileName << "for reading."
                   << "Check your media-player-info installation.";
        return QStringList();
    }
    QString value = readMpiValue(mpiFile, QStringLiteral("Device"), QStringLiteral("AccessProtocol"));
    return value.split(QLatin1Char(';'), Qt::SkipEmptyParts);
}

QStringList PortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_UNUSED(protocol)

    if (!supportedProtocols().isEmpty()) {
        return {QStringLiteral("usb")};
    }
    if (m_device->property(QStringLiteral("USBMUX_SUPPORTED")).toBool() == true) {
        return {QStringLiteral("usbmux")};
    }
    return {};
}

QVariant PortableMediaPlayer::driverHandle(const QString &driver) const
{
    if (driver == QLatin1String("mtp") || driver == QLatin1String("usbmux")) {
        return m_device->property(QStringLiteral("ID_SERIAL_SHORT"));
    }

    return QVariant();
}

QString PortableMediaPlayer::mediaPlayerInfoFilePath() const
{
    QString relativeFilename = m_device->property(QStringLiteral("ID_MEDIA_PLAYER")).toString();
    if (relativeFilename.isEmpty()) {
        qWarning() << "We attached PortableMediaPlayer interface to device" << m_device->udi() << "but m_device->property(\"ID_MEDIA_PLAYER\") is empty???";
        return QString();
    }
    relativeFilename.prepend(QStringLiteral("media-player-info/"));
    relativeFilename.append(QStringLiteral(".mpi"));
    QString filename = QStandardPaths::locate(QStandardPaths::GenericDataLocation, relativeFilename);
    if (filename.isEmpty()) {
        qWarning() << "media player info file" << relativeFilename << "not found under user and"
                   << "system XDG data directories. Do you have media-player-info installed?";
    }
    return filename;
}

#include "moc_udevportablemediaplayer.cpp"
