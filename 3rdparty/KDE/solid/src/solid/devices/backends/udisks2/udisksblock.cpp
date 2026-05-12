/*
    SPDX-FileCopyrightText: 2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksblock.h"

#if defined(Q_OS_LINUX)
#include <linux/kdev_t.h>
#else
// taken from linux/kdev_t.h
#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)
#define MAJOR(dev) ((unsigned int)((dev) >> MINORBITS))
#define MINOR(dev) ((unsigned int)((dev)&MINORMASK))
#endif

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDomDocument>
#include <QFile>

#include "udisks_debug.h"

using namespace Solid::Backends::UDisks2;

Block::Block(Device *dev)
    : DeviceInterface(dev)
    , m_devNum(m_device->prop(QStringLiteral("DeviceNumber")).toULongLong())
    , m_devFile(QFile::decodeName(m_device->prop(QStringLiteral("Device")).toByteArray()))
{
    // we have a drive (non-block device for udisks), so let's find the corresponding (real) block device
    if (m_devNum == 0 || m_devFile.isEmpty()) {
        QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral(UD2_DBUS_SERVICE),
                                                           QStringLiteral(UD2_DBUS_PATH_BLOCKDEVICES),
                                                           QStringLiteral(DBUS_INTERFACE_INTROSPECT),
                                                           QStringLiteral("Introspect"));
        QDBusPendingReply<QString> reply = QDBusConnection::systemBus().asyncCall(call);
        reply.waitForFinished();

        if (reply.isValid()) {
            QDomDocument dom;
            dom.setContent(reply.value());
            QDomNodeList nodeList = dom.documentElement().elementsByTagName(QStringLiteral("node"));
            for (int i = 0; i < nodeList.count(); i++) {
                QDomElement nodeElem = nodeList.item(i).toElement();
                if (!nodeElem.isNull() && nodeElem.hasAttribute(QStringLiteral("name"))) {
                    const QString udi = QStringLiteral(UD2_DBUS_PATH_BLOCKDEVICES) + QLatin1Char('/') + nodeElem.attribute(QStringLiteral("name"));

                    Device device(dev->manager(), udi);
                    if (device.drivePath() == dev->udi()) {
                        m_devNum = device.prop(QStringLiteral("DeviceNumber")).toULongLong();
                        m_devFile = QFile::decodeName(device.prop(QStringLiteral("Device")).toByteArray());
                        m_hintSystem = device.prop(QStringLiteral("HintSystem")).toBool();
                        break;
                    }
                }
            }
        } else {
            qCWarning(UDISKS2) << "Failed enumerating UDisks2 objects:" << reply.error().name() << QStringLiteral("\n") << reply.error().message();
        }
    }

    // qDebug() << "devnum:" << m_devNum << "dev file:" << m_devFile;
}

Block::~Block()
{
}

QString Block::device() const
{
    return m_devFile;
}

int Block::deviceMinor() const
{
    return MINOR(m_devNum);
}

int Block::deviceMajor() const
{
    return MAJOR(m_devNum);
}

bool Block::isSystem() const
{
    return m_hintSystem;
}

#include "moc_udisksblock.cpp"
