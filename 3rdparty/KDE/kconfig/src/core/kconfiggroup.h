/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGGROUP_H
#define KCONFIGGROUP_H

#include "kconfigbase.h"

#include <kconfigcore_export.h>

#include <QExplicitlySharedDataPointer>
#include <QStringList>
#include <QVariant>

class KConfig;
class KConfigGroupPrivate;
class KSharedConfig;

/*!
 * \class KConfigGroup
 * \inmodule KConfigCore
 *
 * \brief A class for one specific group in a KConfig object.
 *
 * If you want to access the top-level entries of a KConfig
 * object which are not associated with any group, use an
 * empty group name. For example:
 *
 * \code
 * auto config = KSharedConfig::openConfig();
 * auto globalGroup = KConfigGroup(config, QString());
 * \endcode
 *
 * A KConfigGroup will be read-only if it is constructed from a const config object or from another read-only group.
 */
class KCONFIGCORE_EXPORT KConfigGroup : public KConfigBase
{
public:
    /*!
     * Constructs an invalid group.
     *
     * \sa isValid
     */
    KConfigGroup();

    /*!
     * Constructs a config group corresponding to \a group in \a master.
     *
     * This allows the creation of subgroups by passing another
     * group as \a master. For example:
     *
     * \code
     * auto generalGroup = KConfigGroup(config, "General");
     * auto preferencesGroup = KConfigGroup(&generalGroup, "Preferences");
     * preferencesGroup.writeEntry("SomeKey", true);
     * \endcode
     *
     * Will render the following in the configuration file:
     *
     * \badcode
     * [General][Preferences]
     * SomeKey=true
     * \endcode
     */
    KConfigGroup(KConfigBase *master, const QString &group);

    // TODO: Actually enforce silently ignoring attempts to write.
    /*!
     * Constructs a read-only config group by passing a const config object.
     *
     * A read-only group will silently ignore any attempts to write to it.
     *
     * This allows the creation of subgroups by passing an existing group
     * as \a master.
     */
    KConfigGroup(const KConfigBase *master, const QString &group);

    /*!
     * \overload KConfigGroup(const KConfigBase*,const QString&)
     */
    KConfigGroup(const QExplicitlySharedDataPointer<KSharedConfig> &master, const QString &group);

    KConfigGroup(const KConfigGroup &);
    KConfigGroup &operator=(const KConfigGroup &);

    ~KConfigGroup() override;

    /*!
     * Returns whether the group is valid.
     *
     * A group is invalid if it was constructed without arguments.
     *
     * You should not call any functions on an invalid group.
     */
    bool isValid() const;

    /*!
     * The name of this group.
     *
     * The root group is named "<default>".
     */
    QString name() const;

    /*!
     * Returns whether the containing KConfig object actually contains a
     * group with this name.
     */
    bool exists() const;

    bool sync() override;

    void markAsClean() override;

    AccessMode accessMode() const override;

    /*!
     * Returns the config object that this group belongs to.
     */
    KConfig *config();
    /*!
     * Returns the read-only config object that this group belongs to.
     */
    const KConfig *config() const;

    /*!
     * Copies the entries in this group to \a other configuration object with the given \a pFlags.
     * \note The \a other config object can be either another group or a different file.
     * \since 4.1
     */
    void copyTo(KConfigBase *other, WriteConfigFlags pFlags = Normal) const;

    /*!
     * Changes the configuration object that this group belongs to to \a parent with the given \a pFlags.
     * \note If \a parent is already the parent of this group, this method will have
     * no effect.
     * \since 4.1
     */
    void reparent(KConfigBase *parent, WriteConfigFlags pFlags = Normal);

    /*!
     * Moves the key-value pairs with specific \a keys from one config group to the \a other with the given \a pFlags.
     *
     * In case the entries do not exist the key is ignored.
     * \since 5.88
     */
    void moveValuesTo(const QList<const char *> &keys, KConfigGroup &other, WriteConfigFlags pFlags = Normal);

    /*!
     * Moves the key-value pairs from one config group to the \a other with the given \a pFlags.
     * \since 6.3
     */
    void moveValuesTo(KConfigGroup &other, WriteConfigFlags pFlags = Normal);

    /*!
     * Returns the parent group that this group belongs to,
     * or an invalid group if this is a top-level group.
     * \since 4.1
     */
    KConfigGroup parent() const;

    QStringList groupList() const override;

    /*!
     * Returns a list of keys this group contains.
     */
    QStringList keyList() const;

    /*!
     * Delete all entries in the entire group with the given \a pFlags.
     * \sa deleteEntry()
     */
    void deleteGroup(WriteConfigFlags pFlags = Normal);
    using KConfigBase::deleteGroup;

    /*!
     * Reads the value of an entry specified by \a key in the current group, returning \a aDefault value if the key is not found.
     *
     * This template method makes it possible to write:
     * \code
     * QString foo = readEntry("...", QString("default"));
     * \endcode
     * and the same with all other types supported by QVariant.
     *
     * The return type of the method is simply the same as the type of the default value.
     *
     * \note readEntry("...", Qt::white) will not compile because Qt::white is an enum.
     * You must turn it into readEntry("...", QColor(Qt::white)).
     *
     * \note Only the following QVariant types are allowed : String,
     * StringList, List, Font, Point, PointF, Rect, RectF, Size, SizeF, Color, Int, UInt, Bool,
     * Double, LongLong, ULongLong, DateTime and Date.
     *
     * \sa writeEntry(), deleteEntry(), hasKey()
     */
    template<typename T>
    T readEntry(const QString &key, const T &aDefault) const
    {
        return readEntry(key.toUtf8().constData(), aDefault);
    }
    /*!
     * \overload readEntry<T>(const QString&, const T&) const
     * The \a key name will be encoded in UTF-8.
     */
    template<typename T>
    T readEntry(const char *key, const T &aDefault) const;

    /*!
     * Reads the value of an entry specified by \a key in the current group, returning \a aDefault value if the key is not found.
     * \sa writeEntry(), deleteEntry(), hasKey()
     */
    QVariant readEntry(const QString &key, const QVariant &aDefault) const;
    /*!
     * \overload readEntry(const QString&, const QVariant&) const
     * The \a key name will be encoded in UTF-8.
     */
    QVariant readEntry(const char *key, const QVariant &aDefault) const;

    /*!
     * Reads the string value of an entry specified by \a key in the current group, returning \a aDefault value if the key is not found.
     *
     * If you want to read a path, use readPathEntry().
     * \sa readPathEntry(), writeEntry(), deleteEntry(), hasKey()
     */
    QString readEntry(const QString &key, const QString &aDefault) const;
    /*!
     * \overload readEntry(const QString&, const QString&) const
     * The \a key name will be encoded in UTF-8.
     */
    QString readEntry(const char *key, const QString &aDefault) const;

    /*!
     * \overload readEntry(const QString&, const QString&) const
     */
    QString readEntry(const QString &key, const char *aDefault = nullptr) const;
    /*!
     * \overload readEntry(const QString&, const QString&) const
     * The \a key name will be encoded in UTF-8.
     */
    QString readEntry(const char *key, const char *aDefault = nullptr) const;

    /*!
     * \overload
     * \sa readEntry()
     *
     * \warning This function doesn't convert the items returned
     *          to any type. It's actually a list of QVariant strings. If you
     *          want the items converted to a specific type use
     *          readEntry(const char*, const QList<T>&) const
     */
    QVariantList readEntry(const QString &key, const QVariantList &aDefault) const;
    /*!
     * \overload readEntry(const QString&, const QVariantList&) const
     * The \a key name will be encoded in UTF-8.
     */
    QVariantList readEntry(const char *key, const QVariantList &aDefault) const;

    /*!
     * Reads the string list values of an entry specified by \a key in the current group, returning \a aDefault value if the key is not found.
     * \sa readXdgListEntry(), writeEntry(), deleteEntry(), hasKey()
     */
    QStringList readEntry(const QString &key, const QStringList &aDefault) const;
    /*!
     * \overload readEntry(const QString&, const QStringList&) const
     * The \a key name will be encoded in UTF-8.
     */
    QStringList readEntry(const char *key, const QStringList &aDefault) const;

    /*!
     * Reads a list of values of an entry specified by \a key in the current group, returning \a aDefault value if the key is not found.
     * \sa readXdgListEntry(), writeEntry(), deleteEntry(), hasKey()
     */
    template<typename T>
    QList<T> readEntry(const QString &key, const QList<T> &aDefault) const
    {
        return readEntry(key.toUtf8().constData(), aDefault);
    }
    /*!
     * \overload readEntry<T>(const QString&, const QList<T>&) const
     * The \a key name will be encoded in UTF-8.
     */
    template<typename T>
    QList<T> readEntry(const char *key, const QList<T> &aDefault) const;

    /*!
     * \overload readEntry(const QString&, std::chrono::duration<Rep, Period>)
     * The \a key name will be encoded in UTF-8.
     * \since 6.22
     */
    template<typename Rep, typename Period>
    std::chrono::duration<Rep, Period> readEntry(const char *key, std::chrono::duration<Rep, Period> defaultValue) const;

    /*!
     * Reads a std::chrono duration of an entry specified by \a key in the current group, returning the \a defaultValue if the key is not found.
     * \since 6.22
     */
    template<typename Rep, typename Period>
    std::chrono::duration<Rep, Period> readEntry(const QString &key, std::chrono::duration<Rep, Period> defaultValue) const;

    /*!
     * Reads the list of strings of an entry specified by \a pKey in the current group, returning \a aDefault value if the key is not found.
     * \sa readEntry(const QString&, const QStringList&)
     */
    QStringList readXdgListEntry(const QString &pKey, const QStringList &aDefault = QStringList()) const;
    /*!
     *\overload readXdgListEntry(const QString&, const QStringList&) const
     * The \a key name will be encoded in UTF-8.
     */
    QStringList readXdgListEntry(const char *key, const QStringList &aDefault = QStringList()) const;

    /*!
     * Reads the path value of an entry specified by \a pKey in the current group, returning \a aDefault value if the key is not found. If \a aDefault is null, an empty QString() is returned.
     *
     * Dollar expansion is activated for this value, so that for example $HOME gets expanded.
     */
    QString readPathEntry(const QString &pKey, const QString &aDefault) const;
    /*!
     * \overload readPathEntry(const QString&, const QString&) const
     * The \a key name will be encoded in UTF-8.
     */
    QString readPathEntry(const char *key, const QString &aDefault) const;

    /*!
     * Reads the list of paths of an entry specified by \a pKey in the current group, returning \a aDefault value if the key is not found.
     *
     * Dollar expansion is activated for this value, so that for example $HOME gets expanded.
     */
    QStringList readPathEntry(const QString &pKey, const QStringList &aDefault) const;
    /*!
     * \overload readPathEntry(const QString&, const QStringList&) const
     * The \a key name will be encoded in UTF-8.
     */
    QStringList readPathEntry(const char *key, const QStringList &aDefault) const;

    /*!
     * Reads the untranslated string for an entry specified by \a pKey in the current group, returning \a aDefault value if the key is not found.
     */
    QString readEntryUntranslated(const QString &pKey, const QString &aDefault = QString()) const;
    /*!
     * \overload readEntryUntranslated(const QString&, const QString&) const
     * The \a key name will be encoded in UTF-8.
     */
    QString readEntryUntranslated(const char *key, const QString &aDefault = QString()) const;

    /*!
     * Writes \a key and \a value to the configuration object with the given \a pFlags.
     *
     * Changes to the file will only be synced with
     * KConfig::reparseConfiguration() or KConfigBase::sync().
     * \sa readEntry(), writeXdgListEntry(), deleteEntry()
     */
    void writeEntry(const QString &key, const QVariant &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const char *key, const QVariant &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const QString &key, const QString &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const char *key, const QString &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QByteArray &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const char *key, const QByteArray &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const char *key, const char *value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    template<typename T>
    void writeEntry(const char *key, const T &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    template<typename T>
    void writeEntry(const QString &key, const T &value, WriteConfigFlags pFlags = Normal)
    {
        writeEntry(key.toUtf8().constData(), value, pFlags);
    }

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QStringList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const char *key, const QStringList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    void writeEntry(const QString &key, const QVariantList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeEntry(const char *key, const QVariantList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     */
    template<typename T>
    void writeEntry(const QString &key, const QList<T> &value, WriteConfigFlags pFlags = Normal)
    {
        writeEntry(key.toUtf8().constData(), value, pFlags);
    }
    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    template<typename T>
    void writeEntry(const char *key, const QList<T> &value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * \since 6.22
     */
    template<typename Rep, typename Period>
    void writeEntry(const QString &key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags = Normal);

    /*!
     * \overload writeEntry(const QString&, const QVariant&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     * \since 6.22
     */
    template<typename Rep, typename Period>
    void writeEntry(const char *key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags = Normal);

    /*!
     * Writes \a pKey and \a value to the configuration object with the given \a pFlags,
     * where the \a value matches a list of strings following the XDG desktop entry specification.
     * \sa writeEntry(), readXdgListEntry()
     */
    void writeXdgListEntry(const QString &pKey, const QStringList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writeXdgListEntry(const QString&, const QStringList&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writeXdgListEntry(const char *key, const QStringList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * Writes \a pKey and \a value to the configuration object with the given \a pFlags,
     * where the \a value matches a file path.
     *
     * If the path includes the absolute name for $HOME, the user's home directory
     * is replaced with "$HOME" in the persistent storage.
     *
     * The path should therefore be read back with readPathEntry().
     * \sa writeEntry(), readPathEntry()
     */
    void writePathEntry(const QString &pKey, const QString &path, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writePathEntry(const QString&, const QString&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writePathEntry(const char *Key, const QString &path, WriteConfigFlags pFlags = Normal);

    /*!
     * Writes \a pKey and \a value to the configuration object with the given \a pFlags,
     * where the \a value matches a file path.
     *
     * If the path includes the absolute name for $HOME, the user's home directory
     * is replaced with "$HOME" in the persistent storage.
     *
     * The path should therefore be read back with readPathEntry().
     * \sa writeEntry(), readPathEntry()
     */
    void writePathEntry(const QString &pKey, const QStringList &value, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload writePathEntry(const QString&, const QStringList&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void writePathEntry(const char *key, const QStringList &value, WriteConfigFlags pFlags = Normal);

    /*!
     * Deletes the entry specified by \a pKey in the current group with the given \a pFlags.
     *
     * This also hides system wide defaults.
     * \sa deleteGroup(), readEntry(), writeEntry()
     */
    void deleteEntry(const QString &pKey, WriteConfigFlags pFlags = Normal);
    /*!
     * \overload deleteEntry(const QString&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void deleteEntry(const char *key, WriteConfigFlags pFlags = Normal);

    /*!
     * Returns whether the \a key exists in this group in any of the configuration sources.
     *
     * If this returns \c false for a key, readEntry() (and its variants)
     * will return the default value passed to them.
     * \sa readEntry()
     */
    bool hasKey(const QString &key) const;
    /*!
     * \overload hasKey(const QString&) const
     * The \a key will be encoded in UTF-8.
     */
    bool hasKey(const char *key) const;

    /*!
     * Returns whether this group may be changed.
     * \sa KAuthorized
     * \sa {https://develop.kde.org/docs/administration/kiosk/}{Kiosk - Simple configuration management for large deployment}
     */
    bool isImmutable() const override;

    /*!
     * Returns whether it is possible to change the given entry \a key.
     *
     * If isImmutable() returns \c true, then this method will return
     * \c true for all inputs.
     * \sa KAuthorized
     * \sa {https://develop.kde.org/docs/administration/kiosk/}{Kiosk - Simple configuration management for large deployment}
     */
    bool isEntryImmutable(const QString &key) const;
    /*!
     * \overload isEntryImmutable(const QString&) const
     * The \a key will be encoded in UTF-8.
     */
    bool isEntryImmutable(const char *key) const;

    /*!
     * Reverts an entry \a key to the default settings with the given \a pFlag in the current group.
     *
     * The default settings will be either the system wide (default)
     * value or the value specified in the global KDE config file.
     *
     * To revert entries in the global KDE config file, the global KDE config
     * file should be opened \e explicitly in a \e separate config object.
     *
     * \note This is not the same as deleting the key, as instead the
     * global setting will be copied to the configuration file that this
     * object manipulates.
     */
    void revertToDefault(const QString &key, WriteConfigFlags pFlag = WriteConfigFlags());

    /*!
     * \overload revertToDefault(const QString&, WriteConfigFlags)
     * The \a key will be encoded in UTF-8.
     */
    void revertToDefault(const char *key, WriteConfigFlags pFlag = WriteConfigFlags());

    /*!
     * Returns whether a default is specified for an entry \a key in either the
     * system wide configuration file or the global KDE config file.
     *
     * If an application computes a default value at runtime for
     * a certain entry, like:
     * \code
     * QColor computedDefault = qApp->palette().color(QPalette::Active, QPalette::Text);
     * QColor color = group.readEntry(key, computedDefault);
     * \endcode
     * then it may wish to make the following check before
     * writing back changes:
     * \code
     * if ( (value == computedDefault) && !group.hasDefault(key) )
     *    group.revertToDefault(key);
     * else
     *    group.writeEntry(key, value);
     * \endcode
     *
     * This ensures that as long as the entry is not modified to differ from
     * the computed default, the application will keep using the computed default
     * and will follow changes the computed default makes over time.
     */
    bool hasDefault(const QString &key) const;
    /*!
     * \overload hasDefault(const QString&) const
     * The \a key will be encoded in UTF-8.
     */
    bool hasDefault(const char *key) const;

    /*!
     * Returns a map (tree) of entries for all entries in this group.
     *
     * Only the actual entry string is returned, none of the
     * other internal data should be included.
     */
    QMap<QString, QString> entryMap() const;

protected:
    bool hasGroupImpl(const QString &groupName) const override;
    KConfigGroup groupImpl(const QString &groupName) override;
    const KConfigGroup groupImpl(const QString &groupName) const override;
    void deleteGroupImpl(const QString &groupName, WriteConfigFlags flags) override;
    bool isGroupImmutableImpl(const QString &groupName) const override;

private:
    QExplicitlySharedDataPointer<KConfigGroupPrivate> d;

    friend class KConfigGroupPrivate;

    /*!
     * \internal
     *
     * Return the data in \a value converted to a QVariant
     *
     * \a pKey the name of the entry being converted, this is only used for error
     * reporting
     *
     * \a value the UTF-8 data to be converted
     *
     * \a aDefault the default value if \a pKey is not found
     *
     * Returns \a value converted to QVariant, or \a aDefault if \a value is invalid or cannot be converted.
     */
    static QVariant convertToQVariant(const char *pKey, const QByteArray &value, const QVariant &aDefault);

    KCONFIGCORE_NO_EXPORT void moveValue(const char *key, KConfigGroup &other, WriteConfigFlags pFlags);

    // exported for usage by KServices' KService & KServiceAction
    friend class KServicePrivate; // XXX yeah, ugly^5
    friend class KServiceAction;
};

Q_DECLARE_TYPEINFO(KConfigGroup, Q_RELOCATABLE_TYPE);

#define KCONFIGGROUP_ENUMERATOR_ERROR(ENUM) "The Qt MetaObject system does not seem to know about \"" ENUM "\" please use Q_ENUM or Q_FLAG to register it."

/*!
 * \macro KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)
 * \relates KConfigGroup
 * To add support for your own enum \a Enum in KConfig, you can declare them with Q_ENUM()
 * in a QObject subclass \a Class (which will make moc generate the code to turn the
 * enum into a string and vice-versa), and then (in the cpp code)
 * use the macro
 * \code
 * KCONFIGGROUP_DECLARE_ENUM_QOBJECT(MyClass, MyEnum)
 * \endcode
 *
 */
#define KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)                                                                                                         \
    template<>                                                                                                                                                 \
    Class::Enum KConfigGroup::readEntry(const char *key, const Class::Enum &def) const                                                                         \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Enum);                                                                                                   \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Enum));                                                                                                      \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        const QByteArray M_data = readEntry(key, QByteArray(M_enum.valueToKey(def)));                                                                          \
        return static_cast<Class::Enum>(M_enum.keyToValue(M_data.constData()));                                                                                \
    }                                                                                                                                                          \
    template<>                                                                                                                                                 \
    void KConfigGroup::writeEntry(const char *key, const Class::Enum &value, KConfigBase::WriteConfigFlags flags)                                              \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Enum);                                                                                                   \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Enum));                                                                                                      \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        writeEntry(key, QByteArray(M_enum.valueToKey(value)), flags);                                                                                          \
    }

/*!
 * \macro KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(Class, Flags)
 * \relates KConfigGroup
 * Similar to KCONFIGGROUP_DECLARE_ENUM_QOBJECT for adding your own \a Flags in KConfig but for flags declared with Q_FLAG() for a QObject subclass \a Class.
 * (where multiple values can be set at the same time)
 */
#define KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(Class, Flags)                                                                                                       \
    template<>                                                                                                                                                 \
    Class::Flags KConfigGroup::readEntry(const char *key, const Class::Flags &def) const                                                                       \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Flags);                                                                                                  \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Flags));                                                                                                     \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        const QByteArray M_data = readEntry(key, QByteArray(M_enum.valueToKeys(def)));                                                                         \
        return static_cast<Class::Flags>(M_enum.keysToValue(M_data.constData()));                                                                              \
    }                                                                                                                                                          \
    template<>                                                                                                                                                 \
    void KConfigGroup::writeEntry(const char *key, const Class::Flags &value, KConfigBase::WriteConfigFlags flags)                                             \
    {                                                                                                                                                          \
        const QMetaObject *M_obj = &Class::staticMetaObject;                                                                                                   \
        const int M_index = M_obj->indexOfEnumerator(#Flags);                                                                                                  \
        if (M_index == -1)                                                                                                                                     \
            qFatal(KCONFIGGROUP_ENUMERATOR_ERROR(#Flags));                                                                                                     \
        const QMetaEnum M_enum = M_obj->enumerator(M_index);                                                                                                   \
        writeEntry(key, QByteArray(M_enum.valueToKeys(value)), flags);                                                                                         \
    }

#include "kconfigconversioncheck_p.h"

template<typename T>
T KConfigGroup::readEntry(const char *key, const T &defaultValue) const
{
    KConfigConversionCheck::to_QVariant<T>();
    return qvariant_cast<T>(readEntry(key, QVariant::fromValue(defaultValue)));
}

template<typename T>
QList<T> KConfigGroup::readEntry(const char *key, const QList<T> &defaultValue) const
{
    KConfigConversionCheck::to_QVariant<T>();
    KConfigConversionCheck::to_QString<T>();

    QVariantList data;

    for (const T &value : defaultValue) {
        data.append(QVariant::fromValue(value));
    }

    QList<T> list;
    const auto variantList = readEntry<QVariantList>(key, data);
    for (const QVariant &value : variantList) {
        Q_ASSERT(value.canConvert<T>());
        list.append(qvariant_cast<T>(value));
    }

    return list;
}

template<typename Rep, typename Period>
std::chrono::duration<Rep, Period> KConfigGroup::readEntry(const char *key, std::chrono::duration<Rep, Period> value) const
{
    return std::chrono::duration<Rep, Period>(readEntry(key, value.count()));
}

template<typename Rep, typename Period>
std::chrono::duration<Rep, Period> KConfigGroup::readEntry(const QString &key, std::chrono::duration<Rep, Period> value) const
{
    return std::chrono::duration<Rep, Period>(readEntry(key, value.count()));
}

template<typename T>
void KConfigGroup::writeEntry(const char *key, const T &value, WriteConfigFlags pFlags)
{
    KConfigConversionCheck::to_QVariant<T>();
    writeEntry(key, QVariant::fromValue(value), pFlags);
}

template<typename T>
void KConfigGroup::writeEntry(const char *key, const QList<T> &list, WriteConfigFlags pFlags)
{
    KConfigConversionCheck::to_QVariant<T>();
    KConfigConversionCheck::to_QString<T>();
    QVariantList data;
    for (const T &value : list) {
        data.append(QVariant::fromValue(value));
    }

    writeEntry(key, data, pFlags);
}

template<typename Rep, typename Period>
void KConfigGroup::writeEntry(const QString &key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags)
{
    writeEntry(key, value.count(), pFlags);
}

template<typename Rep, typename Period>
void KConfigGroup::writeEntry(const char *key, std::chrono::duration<Rep, Period> value, WriteConfigFlags pFlags)
{
    writeEntry(key, value.count(), pFlags);
}

#endif // KCONFIGGROUP_H
