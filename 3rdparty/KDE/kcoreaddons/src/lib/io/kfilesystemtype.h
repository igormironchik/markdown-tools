/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#ifndef KFILESYSTEMTYPE_P_H
#define KFILESYSTEMTYPE_P_H

#include <kcoreaddons_export.h>

#include <QString>

/*!
 * \namespace KFileSystemType
 * \inmodule KCoreAddons
 * Provides utility functions for the type of file systems.
 */
namespace KFileSystemType
{
/*!
 * \value Unknown Unknown
 * \value Nfs NFS or other full-featured networked filesystems (autofs, subfs, cachefs, sshfs)
 * \value Smb SMB/CIFS mount (networked but with some FAT-like behavior)
 * \value Fat FAT or similar (msdos, FAT, VFAT)
 * \value Ramfs RAMDISK mount
 * \value Other Ext3, Ext4, ReiserFs, and so on. "Normal" local filesystems.
 * \value [since 5.85] Ntfs NTFS filesystem
 * \value [since 5.86] Exfat ExFat filesystem
 * \value [since 5.100] Fuse (Filesystem in USErspace), this is used for a variety of underlying filesystems.
 */
enum Type {
    Unknown,
    Nfs,
    Smb,
    Fat,
    Ramfs,
    Other,
    Ntfs,
    Exfat,
    Fuse,
};

/*!
 * For a given \a path, returns the filesystem type, one of KFileSystemType::Type
 * values. If the type can't be determined, KFileSystemType::Unknown is returned.
 *
 * \since 5.0
 */
KCOREADDONS_EXPORT Type fileSystemType(const QString &path);

/*!
 * Returns the possibly translated name of a filesystem corresponding to a
 * value from KFileSystemType::Type.
 *
 * \since 5.86
 */
KCOREADDONS_EXPORT QString fileSystemName(KFileSystemType::Type type);
}

#endif
