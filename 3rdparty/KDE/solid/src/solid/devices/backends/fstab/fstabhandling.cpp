/*
    SPDX-FileCopyrightText: 2006-2010 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fstabhandling.h"
#include "fstab_debug.h"

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include <QThreadStorage>

#include <solid/devices/soliddefs_p.h>

#include <solid/config-solid.h>
#include <stdlib.h>

#if HAVE_LIBMOUNT
#include <libmount.h>
#endif

// This is the *BSD branch
#if HAVE_SYS_MOUNT_H
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif

#define FSTAB "/etc/fstab"

// There are currently two APIs implemented:
// libmount for linux
// getmntinfo + struct statfs&flags (BSD 4.4 and friends)

Q_GLOBAL_STATIC(QThreadStorage<Solid::Backends::Fstab::FstabHandling>, globalFstabCache)

Solid::Backends::Fstab::FstabHandling::FstabHandling()
    : m_fstabCacheValid(false)
    , m_mtabCacheValid(false)
{
}

bool _k_isFstabNetworkFileSystem(const QString &fstype, const QString &devName)
{
    if (fstype == QLatin1String("nfs") //
        || fstype == QLatin1String("nfs4") //
        || fstype == QLatin1String("smbfs") //
        || fstype == QLatin1String("cifs") //
        || fstype == QLatin1String("smb3") //
        || fstype == QLatin1String("fuse.sshfs") //
        || fstype == QLatin1String("fuse.rclone") //
        || devName.startsWith(QLatin1String("//"))) {
        return true;
    }
    return false;
}

bool Solid::Backends::Fstab::FstabHandling::isNetworkFileSystem(const QString &device)
{
    const auto fstype = FstabHandling::fstype(device);
    return _k_isFstabNetworkFileSystem(fstype, device);
}

bool _k_isFstabSupportedLocalFileSystem(const QString &fstype)
{
    if (fstype == QLatin1String("fuse.encfs") //
        || fstype == QLatin1String("fuse.cryfs") //
        || fstype == QLatin1String("fuse.gocryptfs") //
        || fstype == QLatin1String("overlay")) {
        return true;
    }
    return false;
}

QString _k_mntFstype(const QString &orig)
{
    if (orig == QLatin1String("sshfs") || orig == QLatin1String("rclone")) {
        return QStringLiteral("fuse.%1").arg(orig);
    }
    return orig;
}

QString _k_deviceNameForMountpoint(const QString &source, const QString &fstype, const QString &mountpoint)
{
    if (fstype.startsWith(QLatin1String("fuse.")) || fstype == QLatin1String("overlay")) {
        return fstype + mountpoint;
    }
    // A source may be mounted several times, e.g. with different
    // options, often a network share with different credentials
    // for different users. Make sure it is unique by appending the
    // mountpoint (which is unique).

    auto _mountpoint = mountpoint;
    if (fstype == QLatin1String("nfs") || fstype == QLatin1String("nfs4")) {
        if (!mountpoint.startsWith(QLatin1Char('/'))) {
            // making sure mount point starts with /
            _mountpoint.prepend(QLatin1Char('/'));
        }
    }
    return source + QLatin1Char(':') + _mountpoint;
}

void Solid::Backends::Fstab::FstabHandling::_k_updateFstabMountPointsCache()
{
    if (globalFstabCache->localData().m_fstabCacheValid) {
        return;
    }

    globalFstabCache->localData().m_fstabCache.clear();
    globalFstabCache->localData().m_fstabOptionsCache.clear();

#if HAVE_LIBMOUNT

    struct libmnt_table *table = mnt_new_table();
    if (!table) {
        return;
    }

    if (mnt_table_parse_fstab(table, NULL) != 0) {
        mnt_free_table(table);
        return;
    }

    struct libmnt_iter *itr = mnt_new_iter(MNT_ITER_FORWARD);
    struct libmnt_fs *fs;

    while (mnt_table_next_fs(table, itr, &fs) == 0) {
        const QString fstype = _k_mntFstype(QFile::decodeName(mnt_fs_get_fstype(fs)));
        const QString fsname = QFile::decodeName(mnt_fs_get_srcpath(fs));
        if (_k_isFstabNetworkFileSystem(fstype, fsname) || _k_isFstabSupportedLocalFileSystem(fstype)) {
            const QString mountpoint = QFile::decodeName(mnt_fs_get_target(fs));
            const QString device = _k_deviceNameForMountpoint(fsname, fstype, mountpoint);
            char *name = mnt_fs_strdup_options(fs);
            const QStringList options = QFile::decodeName(name).split(QLatin1Char(','));
            free(name);

            globalFstabCache->localData().m_fstabCache.insert(device, mountpoint);
            globalFstabCache->localData().m_fstabFstypeCache.insert(device, fstype);
            for (const auto &optionLine : options) {
                const auto split = optionLine.split(QLatin1Char('='));
                const auto optionName = split[0];
                const auto optionValue = split.size() > 1 ? split[1] : QString{};

                globalFstabCache->localData().m_fstabOptionsCache[device].insert(optionName, optionValue);
            }
        }
    }

    mnt_free_iter(itr);

    mnt_free_table(table);

#else

    QFile fstab(QStringLiteral(FSTAB));
    if (!fstab.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream stream(&fstab);
    QString line;

    while (!stream.atEnd()) {
        line = stream.readLine().simplified();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        // not empty or commented out by '#'
        const QStringList items = line.split(QLatin1Char(' '));
        if (items.count() < 4) {
            continue;
        }

        const QString device = items.at(0);
        const QString fstype = _k_mntFstype(items.at(2));

        // prevent accessing a blocking directory
        if (_k_isFstabNetworkFileSystem(fstype, device) || _k_isFstabSupportedLocalFileSystem(fstype)) {
            QString mountpoint = items.at(1);

            if (fstype == QLatin1String("nfs") || fstype == QLatin1String("nfs4")) {
                if (!mountpoint.startsWith(QLatin1Char('/'))) {
                    // making sure mount point starts with /
                    mountpoint.prepend(QLatin1Char('/'));
                }
            }

            globalFstabCache->localData().m_fstabCache.insert(device, mountpoint);
        }
    }

    fstab.close();
#endif
    globalFstabCache->localData().m_fstabCacheValid = true;
}

QStringList Solid::Backends::Fstab::FstabHandling::deviceList()
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    QStringList devices = globalFstabCache->localData().m_mtabCache.keys();

    // Ensure that regardless an fstab device ends with a slash
    // it will match its eventual mounted device regardless whether or not its path
    // ends with a slash
    for (auto it = globalFstabCache->localData().m_fstabCache.constBegin(), end = globalFstabCache->localData().m_fstabCache.constEnd(); it != end; ++it) {
        auto device = it.key();
        // the device is already known
        if (devices.contains(device)) {
            continue;
        }

        // deviceName will or won't end with / depending if device ended with one
        QString deviceName = device;
        if (deviceName.endsWith(QLatin1Char('/'))) {
            deviceName.chop(1);
        } else {
            deviceName.append(QLatin1Char('/'));
        }
        if (!devices.contains(deviceName)) {
            devices.append(device);
        }
    }
    return devices;
}

QStringList Solid::Backends::Fstab::FstabHandling::mountPoints(const QString &device)
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    QStringList mountpoints = globalFstabCache->localData().m_fstabCache.values(device);
    mountpoints += globalFstabCache->localData().m_mtabCache.values(device);
    mountpoints.removeDuplicates();
    return mountpoints;
}

QHash<QString, QString> Solid::Backends::Fstab::FstabHandling::options(const QString &device)
{
    _k_updateFstabMountPointsCache();
    _k_updateMtabMountPointsCache();

    auto options = globalFstabCache->localData().m_mtabOptionsCache.value(device);

    const auto optionsFstab = globalFstabCache->localData().m_fstabOptionsCache.value(device);
    for (const auto &it : optionsFstab.asKeyValueRange()) {
        if (!options.contains(it.first)) {
            options.insert(it.first, it.second);
        }
    }

    return options;
}

QString Solid::Backends::Fstab::FstabHandling::fstype(const QString &device)
{
    _k_updateFstabMountPointsCache();

    return globalFstabCache->localData().m_fstabFstypeCache.value(device);
}

bool Solid::Backends::Fstab::FstabHandling::callSystemCommand(const QString &commandName,
                                                              const QStringList &args,
                                                              const QObject *receiver,
                                                              std::function<void(QProcess *)> callback)
{
    // search mount and Co. in the normal user environment PATH
    const QString exec = QStandardPaths::findExecutable(commandName);
    if (exec.isEmpty()) {
        qCWarning(FSTAB_LOG) << "Couldn't find executable" << commandName << "in current PATH.";
        return false;
    }

    QProcess *process = new QProcess();
    QObject::connect(process,
                     static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     receiver,
                     [process, callback](int exitCode, QProcess::ExitStatus exitStatus) {
                         Q_UNUSED(exitCode);
                         Q_UNUSED(exitStatus);
                         callback(process);
                         process->deleteLater();
                     });

    process->start(exec, args);
    if (process->waitForStarted()) {
        return true;
    }

    delete process;
    return false;
}

void Solid::Backends::Fstab::FstabHandling::_k_updateMtabMountPointsCache()
{
    if (globalFstabCache->localData().m_mtabCacheValid) {
        return;
    }

    globalFstabCache->localData().m_mtabCache.clear();
    globalFstabCache->localData().m_mtabOptionsCache.clear();

#if HAVE_GETMNTINFO

#if GETMNTINFO_USES_STATVFS
    struct statvfs *mounted;
#else
    struct statfs *mounted;
#endif

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i = 0; i < num_fs; i++) {
        QString type = _k_mntFstype(QFile::decodeName(mounted[i].f_fstypename));
        if (_k_isFstabNetworkFileSystem(type, QString()) || _k_isFstabSupportedLocalFileSystem(type)) {
            const QString fsname = QFile::decodeName(mounted[i].f_mntfromname);
            const QString mountpoint = QFile::decodeName(mounted[i].f_mntonname);
            const QString device = _k_deviceNameForMountpoint(fsname, type, mountpoint);
            globalFstabCache->localData().m_mtabCache.insert(device, mountpoint);
            globalFstabCache->localData().m_fstabFstypeCache.insert(device, type);
        }
    }

#elif HAVE_LIBMOUNT

    struct libmnt_table *table = mnt_new_table();
    if (!table) {
        return;
    }

    if (mnt_table_parse_mtab(table, NULL) != 0) {
        mnt_free_table(table);
        return;
    }

    struct libmnt_iter *itr = mnt_new_iter(MNT_ITER_FORWARD);
    struct libmnt_fs *fs;

    while (mnt_table_next_fs(table, itr, &fs) == 0) {
        const QString fstype = _k_mntFstype(QFile::decodeName(mnt_fs_get_fstype(fs)));
        if (_k_isFstabNetworkFileSystem(fstype, QString{}) || _k_isFstabSupportedLocalFileSystem(fstype)) {
            const QString mountpoint = QFile::decodeName(mnt_fs_get_target(fs));
            const QString fsname = QFile::decodeName(mnt_fs_get_srcpath(fs));
            const QString device = _k_deviceNameForMountpoint(fsname, fstype, mountpoint);
            char *name = mnt_fs_strdup_options(fs);
            const QStringList options = QFile::decodeName(name).split(QLatin1Char(','));
            free(name);

            globalFstabCache->localData().m_mtabCache.insert(device, mountpoint);
            globalFstabCache->localData().m_fstabFstypeCache.insert(device, fstype);
            for (const auto &optionLine : options) {
                const auto split = optionLine.split(QLatin1Char('='));
                const auto optionName = split[0];
                const auto optionValue = split.size() > 1 ? split[1] : QString{};

                globalFstabCache->localData().m_mtabOptionsCache[device].insert(optionName, optionValue);
            }
        }
    }

    mnt_free_iter(itr);

    mnt_free_table(table);

#endif

    globalFstabCache->localData().m_mtabCacheValid = true;
}

QStringList Solid::Backends::Fstab::FstabHandling::currentMountPoints(const QString &device)
{
    _k_updateMtabMountPointsCache();
    return globalFstabCache->localData().m_mtabCache.values(device);
}

void Solid::Backends::Fstab::FstabHandling::flushMtabCache()
{
    globalFstabCache->localData().m_mtabCacheValid = false;
}

void Solid::Backends::Fstab::FstabHandling::flushFstabCache()
{
    globalFstabCache->localData().m_fstabCacheValid = false;
}
