/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGBASE_H
#define KCONFIGBASE_H

#include <kconfigcore_export.h>

#include <QStringList>
#include <QtGlobal>

class KConfigGroup;
class KConfigBasePrivate;

/*!
 * \class KConfigBase
 * \inmodule KConfigCore
 * \brief Interface to interact with configuration.
 *
 * KConfigBase allows a component of an application to persists its configuration
 * without the component knowing if it is storing the configuration into a top
 * level KConfig or a KConfigGroup inside a KConfig instance.
 */
class KCONFIGCORE_EXPORT KConfigBase
{
public:
    /*!
     * Flags to control write entry
     *
     * \value Persistent
     *        Save this entry when saving the config object.
     * \value Global
     *        Save the entry to the global KDE config file instead of the application specific config file.
     * \value Localized
     *        Add the locale tag to the key when writing it.
     * \value[since 5.51] Notify
     *        Notifies remote KConfigWatchers of changes (requires DBus support). Implies Persistent.
     * \value Normal
     *        Save the entry to the application specific config file without a locale tag. This is the default.
     * \sa KConfigGroup
     * \sa KConfigWatcher
     * \sa KConfigSkeletonItem::setWriteFlags()
     */
    enum WriteConfigFlag {
        Persistent = 0x01,
        Global = 0x02,
        Localized = 0x04,
        Notify = 0x08 | Persistent,
        Normal = Persistent,
    };
    Q_DECLARE_FLAGS(WriteConfigFlags, WriteConfigFlag)

    virtual ~KConfigBase();

    /*!
     * Returns a list of groups that are known about.
     **/
    virtual QStringList groupList() const = 0;

    /*!
     * Returns \c true if the specified group is known about.
     *
     * \a group name of group to search for
     */
    bool hasGroup(const QString &group) const;

    /*!
     * Returns an object for the named subgroup.
     *
     * \a group the group to open. Pass an empty string here to the KConfig
     *   object to obtain a handle on the root group.
     * Returns config group object for the given group name.
     */
    KConfigGroup group(const QString &group);

    /*!
     * Const overload for group(const QString&)
     * \overload
     */
    const KConfigGroup group(const QString &group) const;

    /*!
     * Delete \a group.
     *
     * This marks \a group as deleted in the config object. This effectively
     * removes any cascaded values from config files earlier in the stack.
     */
    void deleteGroup(const QString &group, WriteConfigFlags flags = Normal);

    /*!
     * Syncs the configuration object that this group belongs to.
     *
     * Unrelated concurrent changes to the same file are merged and thus
     * not overwritten. Note however, that this object is not automatically
     * updated with those changes.
     */
    virtual bool sync() = 0;

    /*!
     * Reset the dirty flags of all entries in the entry map, so the
     * values will not be written to disk on a later call to sync().
     */
    virtual void markAsClean() = 0;

    /*!
     * Possible return values for accessMode().
     * \value NoAccess
     * \value ReadOnly
     * \value ReadWrite
     */
    enum AccessMode {
        NoAccess,
        ReadOnly,
        ReadWrite,
    };

    /*!
     * Returns the access mode of the app-config object.
     *
     * Possible return values
     * are NoAccess (the application-specific config file could not be
     * opened neither read-write nor read-only), ReadOnly (the
     * application-specific config file is opened read-only, but not
     * read-write) and ReadWrite (the application-specific config
     * file is opened read-write).
     */
    virtual AccessMode accessMode() const = 0;

    /*!
     * Checks whether this configuration object can be modified.
     */
    virtual bool isImmutable() const = 0;

    /*!
     * Can changes be made to the entries in \a group?
     *
     * \a group The group to check for immutability.
     *
     * Returns \c false if the entries in \a group can be modified, otherwise \c true
     */
    bool isGroupImmutable(const QString &group) const;

protected:
    KConfigBase();

    virtual bool hasGroupImpl(const QString &groupName) const = 0;
    virtual KConfigGroup groupImpl(const QString &groupName) = 0;
    virtual const KConfigGroup groupImpl(const QString &groupName) const = 0;
    virtual void deleteGroupImpl(const QString &groupName, WriteConfigFlags flags = Normal) = 0;
    virtual bool isGroupImmutableImpl(const QString &groupName) const = 0;

    /*
     * Virtual hook, used to add new "virtual" functions while maintaining
     * binary compatibility. Unused in this class.
     */
    virtual void virtual_hook(int id, void *data);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigBase::WriteConfigFlags)

#endif // KCONFIG_H
