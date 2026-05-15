/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1998 Sven Radej <sven@lisa.exp.univie.ac.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef _KDIRWATCH_H
#define _KDIRWATCH_H

#include <QDateTime>
#include <QObject>
#include <QString>

#include <kcoreaddons_export.h>

class KDirWatchPrivate;

/*!
 * \class KDirWatch
 * \inmodule KCoreAddons
 *
 * \brief Class for watching directory and file changes.
 *
 * Watch directories and files for changes.
 * The watched directories or files don't have to exist yet.
 *
 * When a watched directory is changed, i.e. when files therein are
 * created or deleted, KDirWatch will emit the signal dirty().
 *
 * When a watched, but previously not existing directory gets created,
 * KDirWatch will emit the signal created().
 *
 * When a watched directory gets deleted, KDirWatch will emit the
 * signal deleted(). The directory is still watched for new
 * creation.
 *
 * When a watched file is changed, i.e. attributes changed or written
 * to, KDirWatch will emit the signal dirty().
 *
 * Scanning of particular directories or files can be stopped temporarily
 * and restarted. The whole class can be stopped and restarted.
 * Directories and files can be added/removed from the list in any state.
 *
 * The implementation uses the INOTIFY functionality on LINUX.
 * As a last resort, a regular polling for change of modification times
 * is done; the polling interval is a global config option:
 * DirWatch/PollInterval and DirWatch/NFSPollInterval for NFS mounted
 * directories.
 * The choice of implementation can be adjusted by the user, with the key
 * [DirWatch] PreferredMethod={Stat|QFSWatch|inotify}
 *
 */
class KCOREADDONS_EXPORT KDirWatch : public QObject
{
    Q_OBJECT

public:
    /*!
     * Available watch modes for directory monitoring
     *
     * \value WatchDirOnly Watch just the specified directory
     * \value WatchFiles Watch also all files contained by the directory
     * \value WatchSubDirs Watch also all the subdirs contained by the directory
     *
     **/
    enum WatchMode {
        WatchDirOnly = 0,
        WatchFiles = 0x01,
        WatchSubDirs = 0x02,
    };
    Q_DECLARE_FLAGS(WatchModes, WatchMode)

    /*!
     * Constructor.
     *
     * Scanning begins immediately when a dir/file watch
     * is added.
     *
     * \a parent the parent of the QObject (or \c nullptr for parent-less KDataTools)
     */
    explicit KDirWatch(QObject *parent = nullptr);

    /*!
     * Destructor.
     *
     * Stops scanning and cleans up.
     */
    ~KDirWatch() override;

    /*!
     * Adds a directory to be watched.
     *
     * The directory does not have to exist. When \a watchModes is set to
     * WatchDirOnly (the default), the signals dirty(), created(), deleted()
     * can be emitted, all for the watched directory.
     * When \a watchModes is set to WatchFiles, all files in the watched
     * directory are watched for changes, too. Thus, the signals dirty(),
     * created(), deleted() can be emitted.
     * When \a watchModes is set to WatchSubDirs, all subdirs are watched using
     * the same flags specified in \a watchModes (symlinks aren't followed).
     * If the \a path points to a symlink to a directory, the target directory
     * is watched instead. If you want to watch the link, use \a addFile().
     *
     * \a path the path to watch
     *
     * \a watchModes watch modes
     *
     * \sa  KDirWatch::WatchMode
     */
    void addDir(const QString &path, WatchModes watchModes = WatchDirOnly);

    /*!
     * Adds a file to be watched.
     * If it's a symlink to a directory, it watches the symlink itself.
     *
     * \a file the file to watch
     */
    void addFile(const QString &file);

    /*!
     * Returns the time the directory/file was last changed.
     *
     * \a path the file to check
     *
     * Returns the date of the last modification
     */
    QDateTime ctime(const QString &path) const;

    /*!
     * Removes a directory from the list of scanned directories.
     *
     * If specified path is not in the list this does nothing.
     *
     * \a path the path of the dir to be removed from the list
     */
    void removeDir(const QString &path);

    /*!
     * Removes a file from the list of watched files.
     *
     * If specified path is not in the list this does nothing.
     *
     * \a file the file to be removed from the list
     */
    void removeFile(const QString &file);

    /*!
     * Stops scanning the specified path.
     *
     * The \a path is not deleted from the internal list, it is just skipped.
     * Call this function when you perform an huge operation
     * on this directory (copy/move big files or many files). When finished,
     * call restartDirScan(path).
     *
     * \a path the path to skip
     *
     * Returns true if the \a path is being watched, otherwise false
     * \sa restartDirScan()
     */
    bool stopDirScan(const QString &path);

    /*!
     * Restarts scanning for specified path.
     *
     * It doesn't notify about the changes (by emitting a signal).
     * The ctime value is reset.
     *
     * Call it when you are finished with big operations on that path,
     * \e and when \e you have refreshed that path.
     *
     * \a path the path to restart scanning
     *
     * Returns \c true if the \a path is being watched, otherwise false
     * \sa stopDirScan()
     */
    bool restartDirScan(const QString &path);

    /*!
     * Starts scanning of all dirs in list.
     *
     * \a notify If true, all changed directories (since
     * stopScan() call) will be notified for refresh. If notify is
     * false, all ctimes will be reset (except those who are stopped,
     * but only if \a skippedToo is false) and changed dirs won't be
     * notified. You can start scanning even if the list is
     * empty. First call should be called with \a false or else all
     * directories
     * in list will be notified.
     *
     * \a skippedToo if true, the skipped directories (scanning of which was
     * stopped with stopDirScan() ) will be reset and notified
     * for change. Otherwise, stopped directories will continue to be
     * unnotified.
     */
    void startScan(bool notify = false, bool skippedToo = false);

    /*!
     * Stops scanning of all directories in internal list.
     *
     * The timer is stopped, but the list is not cleared.
     */
    void stopScan();

    /*!
     * Is scanning stopped?
     * After creation of a KDirWatch instance, this is false.
     * Returns true when scanning stopped
     */
    bool isStopped();

    /*!
     * Check if a directory is being watched by this KDirWatch instance
     * \a path the directory to check
     * Returns true if the directory is being watched
     */
    bool contains(const QString &path) const;

    /*!
        \enum KDirWatch::Method

        \value INotify INotify
        \value Stat Stat
        \value QFSWatch QFileSystemWatcher
     */
    enum Method {
        INotify,
        Stat,
        QFSWatch,
    };
    /*!
     * Returns the preferred internal method to
     * watch for changes.
     */
    Method internalMethod() const;

    /*!
     * The KDirWatch instance usually globally used in an application.
     * It is automatically deleted when the application exits.
     *
     * However, you can create an arbitrary number of KDirWatch instances
     * aside from this one - for those you have to take care of memory management.
     *
     * This function returns an instance of KDirWatch. If there is none, it
     * will be created.
     *
     * Returns a KDirWatch instance
     */
    static KDirWatch *self();
    /*!
     * Returns true if there is an instance of KDirWatch.
     * \sa KDirWatch::self()
     */
    static bool exists();

    bool event(QEvent *event) override;

public Q_SLOTS:

    /*!
     * Emits created().
     *
     * \a path the path of the file or directory
     */
    void setCreated(const QString &path);

    /*!
     * Emits dirty().
     *
     * \a path the path of the file or directory
     */
    void setDirty(const QString &path);

    /*!
     * Emits deleted().
     *
     * \a path the path of the file or directory
     */
    void setDeleted(const QString &path);

Q_SIGNALS:

    /*!
     * Emitted when a watched object is changed.
     * For a directory this signal is emitted when files
     * therein are created or deleted.
     * For a file this signal is emitted when its size or attributes change.
     *
     * When you watch a directory, changes in the size or attributes of
     * contained files may or may not trigger this signal to be emitted
     * depending on which backend is used by KDirWatch.
     *
     * The new ctime is set before the signal is emitted.
     *
     * \a path the path of the file or directory
     */
    void dirty(const QString &path);

    /*!
     * Emitted when a file or directory (being watched explicitly) is created.
     * This is not emitted when creating a file is created in a watched directory.
     *
     * \a path the path of the file or directory
     */
    void created(const QString &path);

    /*!
     * Emitted when a file or directory is deleted.
     *
     * The object is still watched for new creation.
     *
     * \a path the path of the file or directory
     */
    void deleted(const QString &path);

private:
    KDirWatchPrivate *d;
    friend class KDirWatchPrivate;
    friend class KDirWatch_UnitTest;
};

/*!
 * Dump debug information about the KDirWatch::self() instance.
 * This checks for consistency, too.
 * \relates KDirWatch
 */
KCOREADDONS_EXPORT QDebug operator<<(QDebug debug, const KDirWatch &watch);

Q_DECLARE_OPERATORS_FOR_FLAGS(KDirWatch::WatchModes)

#endif
