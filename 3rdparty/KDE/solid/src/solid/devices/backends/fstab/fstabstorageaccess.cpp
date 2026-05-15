/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fstabstorageaccess.h"
#include "fstabwatcher.h"
#include <QStringList>
#include <solid/devices/backends/fstab/fstabdevice.h>
#include <solid/devices/backends/fstab/fstabhandling.h>
#include <solid/devices/backends/fstab/fstabservice.h>

#include <QDir>
#include <QProcess>
#include <QTimer>

#include <errno.h>

using namespace Solid::Backends::Fstab;

namespace
{
bool isNetworkDeviceAccessibleToUser(const QString &device, const QString &filePath)
{
    // Limit the number of calls to `QDir::isReadable` since it does a call to `stat` which is blocking for the thread.
    // Network mounts like fuse.sshfs (used in KDE Connect for example) could still block when the phone disconnects, so filter these out too
    const auto fstype = FstabHandling::fstype(device);
    if (!FstabHandling::isNetworkFileSystem(device) || fstype.startsWith(QStringLiteral("fuse."))) {
        return true;
    }

    return QDir(filePath).isReadable();
}
}

FstabStorageAccess::FstabStorageAccess(Solid::Backends::Fstab::FstabDevice *device)
    : QObject(device)
    , m_fstabDevice(device)
{
    QStringList currentMountPoints = FstabHandling::currentMountPoints(device->device());
    if (currentMountPoints.isEmpty()) {
        QStringList mountPoints = FstabHandling::mountPoints(device->device());
        m_filePath = mountPoints.isEmpty() ? QString() : mountPoints.first();
        m_isAccessible = false;
    } else {
        m_filePath = currentMountPoints.first();
        m_isAccessible = true;
    }

    const bool inUserPath =
        m_filePath.startsWith(QLatin1String("/media/")) || m_filePath.startsWith(QLatin1String("/run/media/")) || m_filePath.startsWith(QDir::homePath());

    const bool gvfsHidden = FstabHandling::options(device->device()).contains(QLatin1String("x-gvfs-hide"));
    const bool fsIsOverlay = FstabHandling::fstype(device->device()) == QLatin1String("overlay");

    m_isIgnored = gvfsHidden ||
        // ignore overlay fs not pointing to / or seemingly mounted by user
        (fsIsOverlay && m_filePath != QLatin1String("/") && !inUserPath) ||
        // Ignore network devices where the user is missing read rights
        !::isNetworkDeviceAccessibleToUser(device->device(), m_filePath);

    connect(device, &FstabDevice::mtabChanged, this, &FstabStorageAccess::onMtabChanged);
    QTimer::singleShot(0, this, SLOT(connectDBusSignals()));
}

FstabStorageAccess::~FstabStorageAccess()
{
}

void FstabStorageAccess::connectDBusSignals()
{
    m_fstabDevice->registerAction(QStringLiteral("setup"), this, SLOT(slotSetupRequested()), SLOT(slotSetupDone(int, QString)));

    m_fstabDevice->registerAction(QStringLiteral("teardown"), this, SLOT(slotTeardownRequested()), SLOT(slotTeardownDone(int, QString)));
}

const Solid::Backends::Fstab::FstabDevice *FstabStorageAccess::fstabDevice() const
{
    return m_fstabDevice;
}

bool FstabStorageAccess::isAccessible() const
{
    return m_isAccessible;
}

QString FstabStorageAccess::filePath() const
{
    return m_filePath;
}

bool FstabStorageAccess::isIgnored() const
{
    return m_isIgnored;
}

bool FstabStorageAccess::isEncrypted() const
{
    return m_fstabDevice->isEncrypted();
}

bool FstabStorageAccess::setup()
{
    if (filePath().isEmpty()) {
        return false;
    }
    m_fstabDevice->broadcastActionRequested(QStringLiteral("setup"));
    return FstabHandling::callSystemCommand(QStringLiteral("mount"), {filePath()}, this, [this](QProcess *process) {
        if (process->exitCode() == 0) {
            m_fstabDevice->broadcastActionDone(QStringLiteral("setup"), Solid::NoError, QString());
        } else {
            m_fstabDevice->broadcastActionDone(QStringLiteral("setup"),
                                               Solid::UnauthorizedOperation,
                                               QString::fromUtf8(process->readAllStandardError().trimmed()));
        }
    });
}

void FstabStorageAccess::slotSetupRequested()
{
    Q_EMIT setupRequested(m_fstabDevice->udi());
}

bool FstabStorageAccess::teardown()
{
    if (filePath().isEmpty()) {
        return false;
    }
    m_fstabDevice->broadcastActionRequested(QStringLiteral("teardown"));
    return FstabHandling::callSystemCommand(QStringLiteral("umount"), {filePath()}, this, [this](QProcess *process) {
        if (process->exitCode() == 0) {
            m_fstabDevice->broadcastActionDone(QStringLiteral("teardown"), Solid::NoError);
        } else if (process->exitCode() == EBUSY) {
            m_fstabDevice->broadcastActionDone(QStringLiteral("teardown"), Solid::DeviceBusy);
        } else if (process->exitCode() == EPERM) {
            m_fstabDevice->broadcastActionDone(QStringLiteral("teardown"),
                                               Solid::UnauthorizedOperation,
                                               QString::fromUtf8(process->readAllStandardError().trimmed()));
        } else {
            m_fstabDevice->broadcastActionDone(QStringLiteral("teardown"),
                                               Solid::OperationFailed,
                                               QString::fromUtf8(process->readAllStandardError().trimmed()));
        }
    });
}

void FstabStorageAccess::slotTeardownRequested()
{
    Q_EMIT teardownRequested(m_fstabDevice->udi());
}

void FstabStorageAccess::slotSetupDone(int error, const QString &errorString)
{
    Q_EMIT setupDone(static_cast<Solid::ErrorType>(error), errorString, m_fstabDevice->udi());
}

void FstabStorageAccess::slotTeardownDone(int error, const QString &errorString)
{
    Q_EMIT teardownDone(static_cast<Solid::ErrorType>(error), errorString, m_fstabDevice->udi());
}

void FstabStorageAccess::onMtabChanged(const QString &device)
{
    QStringList currentMountPoints = FstabHandling::currentMountPoints(device);
    if (currentMountPoints.isEmpty()) {
        // device umounted
        m_filePath = FstabHandling::mountPoints(device).first();
        if (m_isAccessible) {
            m_isAccessible = false;
            Q_EMIT accessibilityChanged(false, QStringLiteral(FSTAB_UDI_PREFIX "/%1").arg(device));
        }
    } else {
        // device added
        m_filePath = currentMountPoints.first();
        if (!m_isAccessible) {
            m_isAccessible = true;
            Q_EMIT accessibilityChanged(true, QStringLiteral(FSTAB_UDI_PREFIX "/%1").arg(device));
        }
    }
}

#include "moc_fstabstorageaccess.cpp"
