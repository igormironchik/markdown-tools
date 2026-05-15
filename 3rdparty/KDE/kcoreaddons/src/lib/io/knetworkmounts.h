/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KNETWORKMOUNTS_H
#define KNETWORKMOUNTS_H

#include <memory>

#include <QObject>
#include <kcoreaddons_export.h>

/*!
 * \class KNetworkMounts
 * \inmodule KCoreAddons
 *
 * Performance control on network mounts.
 *
 * This class provides methods for deciding whether operations
 * on slow network mounts should be performed or not.
 *
 * Configuration is read from a configuration file network_mounts in
 * the user's QStandardPaths::ConfigLocation. This file can be filled by using
 * the network mounts performance configuration module or directly via setEnabled,
 * setPaths, addPath and setOption
 * \code
 *   KNetworkMounts::self()->setEnabled(true);
 *   KNetworkMounts::self()->setOption(KNetworkMounts::LowSideEffectsOptimizations, true);
 *   KNetworkMounts::self()->addPath(path1, KNetworkMounts::NfsPaths);
 *   KNetworkMounts::self()->addPath(path2, KNetworkMounts::NfsPaths);
 *   KNetworkMounts::self()->setPaths(listOfPaths, KNetworkMounts::SmbPaths);
 * \endcode
 *
 * Use KNetworkMounts like this to check if the given url is on a
 * configured slow path and the KNetworkMountOption LowSideEffectsOptimizations
 * is enabled:
 * \code
 *   if (KNetworkMounts::self()->isOptionEnabledForPath(url.toLocalFile(),
 *       KNetworkMounts::LowSideEffectsOptimizations))
 *   {
 *       // skip operations which are slow on the given url if
 *       // KNetworkMountOption LowSideEffectsOptimizations is enabled
 *   } else {
 *       // given url is not configured being slow or the KNetworkMountOption
 *       // LowSideEffectsOptimizations is not enabled
 *   }
 * \endcode
 *
 * If called for the first time, this creates a singleton instance and reads
 * the config file. Subsequent calls just use this instance without reading
 * the config file again.
 *
 * \since 5.85
 **/
class KCOREADDONS_EXPORT KNetworkMounts : public QObject
{
    Q_OBJECT

public:
    /*!
     * Returns (and creates if necessary) the singleton instance
     */
    static KNetworkMounts *self();

    /*!
     * The KNetworkMountOption enum
     *
     * \value LowSideEffectsOptimizations
     * Don't run KDiskFreeSpaceInfo if slow path.
     * Don't check for manually mounted drives.
     * Don't check with QFileInfo::isWritable if it is writable, if not yet known, return true.
     * Don't check with QFileInfo::isReadable if it is readable, return false.
     * Don't check for desktop files just return false.
     * Ignore .hidden files on slow paths.
     * Don't read mime comment from .desktop or .directory files.
     * Don't get the size with QFileInfo::size, just return 0, if not yet known.
     * Don't determine mime type from file content, use file extension.
     * Don't check for desktop files just return false.
     * Don't call KFileSystemType::fileSystemType to check if the filesystem is slow, just return true.
     * Don't count files/directories in subdirectories.
     * Don't calculate sizes of subdirectories.
     * Avoid check for dir at Kate startup.
     * \value MediumSideEffectsOptimizations
     * Don't return project for dir, avoid QFileInfo().absoluteDir().
     * Don't search for .kateconfig recursively
     * Ignore recent files on slow paths
     * \value StrongSideEffectsOptimizations
     * Turn off symbolic link resolution
     * \value KDirWatchDontAddWatches
     * Disables dir watching completely for slow paths, avoids stat() calls on added dirs and subdirs
     * \value SymlinkPathsUseCache
     * Cache resolved symlink paths
     */
    enum KNetworkMountOption {
        LowSideEffectsOptimizations,
        MediumSideEffectsOptimizations,
        StrongSideEffectsOptimizations,
        KDirWatchDontAddWatches,
        SymlinkPathsUseCache
    };
    Q_ENUM(KNetworkMountOption)

    /*!
     * The KNetworkMountsType enum
     *
     * \value NfsPaths NFS paths
     * \value SmbPaths SMB paths
     * \value SymlinkDirectory Paths to directories which contain symbolic links to network mounts
     * \value SymlinkToNetworkMount Paths which are symbolic links to network mounts
     * \value Any Any slow path type. Do not use with setPaths or addPath
     */
    enum KNetworkMountsType { NfsPaths, SmbPaths, SymlinkDirectory, SymlinkToNetworkMount, Any };
    Q_ENUM(KNetworkMountsType)

    /*!
     * Query if \a path is configured to be a slow path of type \a type
     *
     * \a path the path to query
     *
     * \a type the type to query. If omitted, any type matches
     *
     * Returns \c true if \a path is a configured slow path of type \a type
     *
     * This function is also used to determine the filesystem type in KFileSystemType::fileSystemType
     * (KFileSystemType::Smb or KFileSystemType::Nfs) without an expensive call to stafs(). For this
     * to work the types of paths need to be correctly assigned in setPath or addPath
     */
    bool isSlowPath(const QString &path, KNetworkMountsType type = Any);

    /*!
     * Query if \a path is configured to be a slow path and \a option is enabled
     *
     * \a path the path to query
     *
     * \a option the option to query
     *
     * Returns \c true if \a path is a configured slow path and option \a option is enabled
     */
    bool isOptionEnabledForPath(const QString &path, KNetworkMountOption option);

    /*!
     * Query if the performance optimizations are switched on
     *
     * Returns \c true if on, \c false otherwise
     */
    bool isEnabled() const;

    /*!
     * Switch the performance optimizations on or off
     *
     * \a value the value to set
     */
    void setEnabled(bool value);

    /*!
     * Query a performance option
     *
     * \a option the option to query
     *
     * \a defaultValue the value to return if the option is not configured
     *
     * Returns \c true if option is on, \c false if not
     * \sa KNetworkMountOption
     */
    bool isOptionEnabled(const KNetworkMountOption option, const bool defaultValue = false) const;

    /*!
     * Switch a performance option on or off
     *
     * \a option the option to change
     *
     * \a value the value to set
     *
     * \sa KNetworkMountOption
     */
    void setOption(const KNetworkMountOption option, const bool value);

    /*!
     * Query the configured paths for which optimizations are to take place
     *
     * Returns a list of paths
     */
    QStringList paths(KNetworkMountsType type = Any) const;

    /*!
     * Set the paths for which optimizations are to take place
     *
     * \a paths the paths to set
     *
     * \a type the type of paths. Do not use Any
     *
     * \sa KNetworkMountsType
     */
    void setPaths(const QStringList &paths, KNetworkMountsType type);

    /*!
     * Add a path for which optimizations are to take place
     *
     * \a path the path to add
     *
     * \a type the type of the path. Do not use Any
     *
     * \sa KNetworkMountsType
     */
    void addPath(const QString &path, KNetworkMountsType type);

    /*!
     * Resolves a \a path that may contain symbolic links to mounted network shares.
     *
     * A symlink path is either a directory which contains symbolic links to slow network mounts
     * (SymlinkDirectory) or a direct symbolic link to a slow network mount (     * SymlinkToNfsOrSmbPaths).
     *
     * Example:
     * There are some Samba shares mounted below /mnt. These are paths of type SmbPaths
     * \code
     * /mnt/server1/share1
     * /mnt/server1/share2
     * /mnt/server2/share3
     * \endcode
     *
     * A (logged in) user may have symbolic links to them in his home directory below netshares. The
     * directory /home/user/netshares is a SymlinkDirectory:
     * \code
     * /home/user/netshares/share1 -> /mnt/server1/share1
     * /home/user/netshares/share2 -> /mnt/server1/share2
     * /home/user/netshares/share3 -> /mnt/server2/share3
     * \endcode
     *
     * There is a direct symbolic link from /home/user/share1 to /mnt/server1/share1. This is of type
     * SymlinkToNfsOrSmbPaths:
     * \code
     * /home/user/share1 -> /mnt/server1/share1
     * \endcode
     *
     * Both types of symbolic links from symlink paths to the real mounted shares are resolved even if
     * KNetworkMountOption StrongSideEffectsOptimizations is enabled.
     *
     * If the setup is like above a \a path
     * \code
     * /home/user/netshares/share1/Documents/file.txt
     * \endcode
     *
     * would be resolved to
     * \code
     * /mnt/server1/share1/Documents/file.txt
     * \endcode
     *
     * and a \a path
     * \code
     * /home/user/share1/Documents/file.txt
     * \endcode
     *
     * would also be resolved to
     * \code
     * /mnt/server1/share1/Documents/file.txt
     * \endcode
     *
     * Resolved paths are cached in a hash.
     *
     * \a path the path to resolve
     *
     * Returns the resolved path or \a path if \a path is not a symlink path or no symlink found
     * \sa KNetworkMountsType
     * \sa clearCache
     * \sa isSlowPath
     */
    QString canonicalSymlinkPath(const QString &path);

    /*!
     * Clears the canonical symlink path cache
     *
     * Call this if directory structures on mounted network drives changed. Don't enable the
     * cache (SymlinkPathsUseCache) if this happens often and the drives are usually accessed
     * via the symlinks. This method exists mainly for the KCM.
     * \sa canonicalSymlinkPath
     */
    void clearCache();

    /*!
     * Synchronizes to config file
     *
     * QSettings synchronization also takes place automatically at regular intervals and from
     * QSettings destructor, see QSettings::sync() documentation.
     *
     * Calls QSettings::sync()
     */
    void sync();

private:
    /// Creates a new KNetworkMounts object
    KCOREADDONS_NO_EXPORT KNetworkMounts();

    /// Destructor
    KCOREADDONS_NO_EXPORT ~KNetworkMounts() override;

    std::unique_ptr<class KNetworkMountsPrivate> const d;
};

#endif // KNETWORKMOUNTS_H
