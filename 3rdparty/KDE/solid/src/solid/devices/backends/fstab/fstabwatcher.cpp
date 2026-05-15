/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fstabwatcher.h"
#include "fstab_debug.h"
#include "soliddefs_p.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileSystemWatcher>
#include <QSocketNotifier>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
Q_GLOBAL_STATIC(FstabWatcher, globalFstabWatcher)

static const QString s_mtabFile = QStringLiteral("/etc/mtab");
static const QString s_fstabFile = QStringLiteral("/etc/fstab");
static const QString s_fstabPath = QStringLiteral("/etc");

FstabWatcher::FstabWatcher()
{
#ifdef Q_OS_LINUX
    auto mountMonitor = mnt_new_monitor();

    if (!mountMonitor) {
        qCCritical(FSTAB_LOG) << "could not start mount monitor";
        return;
    }
    m_mountMonitor = mountMonitor;

    auto r = mnt_monitor_enable_kernel(m_mountMonitor, true);
    if (r < 0) {
        mnt_unref_monitor(m_mountMonitor);
        qCCritical(FSTAB_LOG) << "Failed to enable watching of kernel mount events:" << strerror(errno);
    }

    r = mnt_monitor_enable_userspace(m_mountMonitor, true, NULL);
    if (r < 0) {
        mnt_unref_monitor(m_mountMonitor);
        qCCritical(FSTAB_LOG) << "Failed to enable watching of userspace mount events:" << strerror(errno);
    }

    auto fd = mnt_monitor_get_fd(m_mountMonitor);
    if (fd < 0) {
        mnt_unref_monitor(m_mountMonitor);
        qCCritical(FSTAB_LOG) << "Failed to acquire watch file descriptor" << strerror(errno);
        m_mountMonitor = nullptr;
        return;
    }

    m_socketNotifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &FstabWatcher::onMountChanged);

    if (qApp) {
        connect(qApp, &QCoreApplication::aboutToQuit, this, &FstabWatcher::onQuit);
    }
#else
    m_isRoutineInstalled = false;
    m_fileSystemWatcher = new QFileSystemWatcher(this);

#ifndef Q_OS_OPENBSD
    m_mtabFile = new QFile(s_mtabFile, this);
    if (m_mtabFile && m_mtabFile->symLinkTarget().startsWith(QLatin1String("/proc/")) && m_mtabFile->open(QIODevice::ReadOnly)) {
        m_socketNotifier = new QSocketNotifier(m_mtabFile->handle(), QSocketNotifier::Exception, this);
        connect(m_socketNotifier, &QSocketNotifier::activated, this, &FstabWatcher::mtabChanged);
    } else {
        m_fileSystemWatcher->addPath(s_mtabFile);
    }
#endif

    m_fileSystemWatcher->addPath(s_fstabPath);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &) {
        if (!m_isFstabWatched) {
            m_isFstabWatched = m_fileSystemWatcher->addPath(s_fstabFile);
            if (m_isFstabWatched) {
                qCDebug(FSTAB_LOG) << "Re-added" << s_fstabFile;
                Q_EMIT onFileChanged(s_fstabFile);
            }
        }
    });

    m_isFstabWatched = m_fileSystemWatcher->addPath(s_fstabFile);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &FstabWatcher::onFileChanged);
#endif
}

FstabWatcher::~FstabWatcher()
{
#ifdef Q_OS_LINUX
    if (m_mountMonitor) {
        mnt_unref_monitor(m_mountMonitor);
    }
#else
    // The QFileSystemWatcher doesn't work correctly in a singleton
    // The solution so far was to destroy the QFileSystemWatcher when the application quits
    // But we have some crash with this solution.
    // For the moment to workaround the problem, we detach the QFileSystemWatcher from the parent
    // effectively leaking it on purpose.
    m_fileSystemWatcher->setParent(nullptr);
#endif
}

void FstabWatcher::onQuit()
{
#ifndef Q_OS_LINUX
    m_fileSystemWatcher->setParent(nullptr);
#endif
}

FstabWatcher *FstabWatcher::instance()
{
#if 0
    FstabWatcher *fstabWatcher = globalFstabWatcher;

    if (fstabWatcher && !fstabWatcher->m_isRoutineInstalled) {
        qAddPostRoutine(globalFstabWatcher.destroy);
        fstabWatcher->m_isRoutineInstalled = true;
    }
    return fstabWatcher;
#else
    return globalFstabWatcher;
#endif
}

#ifdef Q_OS_LINUX
void FstabWatcher::onMountChanged()
{
    auto mtab = false;
    auto fstab = false;
    const char *filename;
    while (mnt_monitor_next_change(m_mountMonitor, &filename, NULL) == 0) {
        if (!mtab && ((strcmp(filename, "/proc/self/mountinfo") == 0) || (strcmp(filename, "/run/mount/utab") == 0))) {
            mtab = true;
        }
        if (!fstab && (strcmp(filename, "/etc/fstab") == 0)) {
            fstab = true;
        }
    }

    if (mtab) {
        Q_EMIT mtabChanged();
    }
    if (fstab) {
        Q_EMIT fstabChanged();
    }
}
#else
void FstabWatcher::onFileChanged(const QString &path)
{
    if (path == s_mtabFile) {
        Q_EMIT mtabChanged();
        if (!m_fileSystemWatcher->files().contains(s_mtabFile)) {
            m_fileSystemWatcher->addPath(s_mtabFile);
        }
    }
    if (path == s_fstabFile) {
        Q_EMIT fstabChanged();
        if (!m_fileSystemWatcher->files().contains(s_fstabFile)) {
            m_isFstabWatched = m_fileSystemWatcher->addPath(s_fstabFile);
            qCDebug(FSTAB_LOG) << "Fstab removed, re-added:" << m_isFstabWatched;
        }
    }
}
#endif
}
}
} // namespace Solid:Backends::Fstab

#include "moc_fstabwatcher.cpp"
