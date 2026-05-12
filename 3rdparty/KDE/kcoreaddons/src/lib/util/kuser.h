/*
    KUser - represent a user/account

    SPDX-FileCopyrightText: 2002-2003 Tim Jansen <tim@tjansen.de>
    SPDX-FileCopyrightText: 2003 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2004 Jan Schaefer <j_schaef@informatik.uni-kl.de>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KUSER_H
#define KUSER_H

#include <kcoreaddons_export.h>

#include <QSharedDataPointer>
#include <QStringList>
#include <QVariant>
#include <qcontainerfwd.h>

class KUserGroup;
class QString;

#ifdef Q_OS_WIN
typedef void *K_UID;
typedef void *K_GID;
struct WindowsSIDWrapper;
#else
#include <sys/types.h>
typedef uid_t K_UID;
typedef gid_t K_GID;
struct passwd;
struct group;
#endif

// The following is to avoid compile errors using msvc, and it is done
// using a common #define to avoid helpful people accidentally cleaning this
// not quite pretty thing and breaking it for people on windows.
// See https://git.reviewboard.kde.org/r/127598/ for details
#define KCOREADDONS_UINT_MAX (std::numeric_limits<uint>::max)()

/* A platform independent user or group ID.
 *
 *
 * This struct is required since Windows does not have an integer uid_t/gid_t type
 * but instead uses an opaque binary blob (SID) which must free allocated memory.
 * On UNIX this is simply a uid_t/gid_t and all operations are inline, so there is
 * no runtime overhead over using the uid_t/gid_t directly. On Windows this is an implicitly
 * shared class that frees the underlying SID once no more references remain.
 *
 * Unlike KUser/KUserGroup this does not query additional information, it is simply
 * an abstraction over the native user/group ID type. If more information is necessary, a
 * KUser or KUserGroup instance can be constructed from this ID
 *
 */
template<typename T>
struct KCOREADDONS_EXPORT KUserOrGroupId {
    typedef T NativeType;

protected:
    /* Creates an invalid KUserOrGroupId */
    KUserOrGroupId();
    /* Creates a KUserOrGroupId from a native user/group ID. On windows this will not take
     * ownership over the passed SID, a copy will be created instead.
     */
    explicit KUserOrGroupId(NativeType nativeId);
    /* Copy constructor. This is very fast, objects can be passed by value */
    KUserOrGroupId(const KUserOrGroupId<T> &other);
    KUserOrGroupId &operator=(const KUserOrGroupId<T> &other);
    ~KUserOrGroupId();

public:
    /* Returns true if this object references a valid user/group ID.
     *
ote If this returns true it doesn't necessarily mean that the referenced user/group exists,
     * it only checks whether this value could be a valid user/group ID.
     */
    bool isValid() const;
    /*
     * Returns A user/group ID that can be used in operating system specific functions
     *
ote On Windows the returned pointer will be freed once the last KUserOrGroupId referencing
     * this user/group ID is deleted. Make sure that the KUserOrGroupId object remains valid as
     * long as the native pointer is needed.
     */
    NativeType nativeId() const;
    /* Returns A string representation of this user ID, not the name of the user
     * On UNIX this is a simple integer, e.g. "0" for root. On Windows this is a string
     * like e.g. "S-1-5-32-544" for the Administrators group
     */
    QString toString() const;
    /* Returns whether this KUserOrGroupId is equal to \a other */
    bool operator==(const KUserOrGroupId &other) const;
    /* Returns whether this KUserOrGroupId is not equal to \a other */
    bool operator!=(const KUserOrGroupId &other) const;

private:
#ifdef Q_OS_WIN
    QExplicitlySharedDataPointer<WindowsSIDWrapper> data;
#else
    NativeType id;
#endif
};

#ifdef Q_OS_WIN
template<>
KUserOrGroupId<void *>::KUserOrGroupId();
template<>
KUserOrGroupId<void *>::~KUserOrGroupId();
template<>
KUserOrGroupId<void *>::KUserOrGroupId(KUserOrGroupId::NativeType nativeId);
template<>
KUserOrGroupId<void *>::KUserOrGroupId(const KUserOrGroupId &other);
template<>
KUserOrGroupId<void *> &KUserOrGroupId<void *>::operator=(const KUserOrGroupId &other);
template<>
bool KUserOrGroupId<void *>::isValid() const;
template<>
KUserOrGroupId<void *>::NativeType KUserOrGroupId<void *>::nativeId() const;
template<>
QString KUserOrGroupId<void *>::toString() const;
template<>
bool KUserOrGroupId<void *>::operator==(const KUserOrGroupId &other) const;
template<>
bool KUserOrGroupId<void *>::operator!=(const KUserOrGroupId &other) const;
#endif

/* A platform independent user ID.
 */
struct KCOREADDONS_EXPORT KUserId : public KUserOrGroupId<K_UID> {
    /* Creates an invalid KUserId */
    KUserId()
    {
    }
    /* Creates an KUserId from the native user ID type */
    explicit KUserId(K_UID uid)
        : KUserOrGroupId(uid)
    {
    }
    /* Returns a KUserId for the user with name \a name, or an invalid KUserId if no
     * user with this name was found on the system
     */
    static KUserId fromName(const QString &name);
    /* Returns a KUserId for the current user. This is like ::getuid() on UNIX. */
    static KUserId currentUserId();
    /* Returns a KUserId for the current effective user. This is like ::geteuid() on UNIX.
     *
ote Windows does not have setuid binaries, so on Windows this will always be the same
     * as KUserId::currentUserId()
     */
    static KUserId currentEffectiveUserId();
};

/* A platform independent group ID.
 */
struct KCOREADDONS_EXPORT KGroupId : public KUserOrGroupId<K_GID> {
    /* Creates an invalid KGroupId */
    KGroupId()
    {
    }
    /* Creates an KGroupId from the native group ID type */
    explicit KGroupId(K_GID gid)
        : KUserOrGroupId(gid)
    {
    }
    /* Returns A KGroupId for the user with name \a name, or an invalid KGroupId if no
     * user with this name was found on the system
     */
    static KGroupId fromName(const QString &name);
    /* Returns a KGroupId for the current user. This is like ::getgid() on UNIX. */
    static KGroupId currentGroupId();
    /* Returns a KGroupId for the current effective user. This is like ::getegid() on UNIX.
     *
ote Windows does not have setuid binaries, so on Windows this will always be the same
     * as KGroupId::currentGroupId()
     */
    static KGroupId currentEffectiveGroupId();
};

#ifndef Q_OS_WIN
inline size_t qHash(const KUserId &id, size_t seed = 0)
{
    return qHash(id.nativeId(), seed);
}
inline size_t qHash(const KGroupId &id, size_t seed = 0)
{
    return qHash(id.nativeId(), seed);
}
#else
// can't be inline on windows, because we would need to include windows.h (which can break code)
KCOREADDONS_EXPORT size_t qHash(const KUserId &id, size_t seed = 0);
KCOREADDONS_EXPORT size_t qHash(const KGroupId &id, size_t seed = 0);
#endif

/*!
 * \class KUser
 * \inmodule KCoreAddons
 *
 * \brief Represents a user on your system.
 *
 * This class represents a user on your system. You can either get
 * information about the current user, or fetch information about
 * a user on the system. Instances of this class will be explicitly shared,
 * so copying objects is very cheap and you can safely pass objects by value.
 *
 */
class KCOREADDONS_EXPORT KUser
{
public:
    /*!
     * \value UseEffectiveUID Use the effective user id
     * \value UseRealUserID Use the real user id
     */
    enum UIDMode {
        UseEffectiveUID,
        UseRealUserID,
    };

    /*!
     * Creates an object that contains information about the current user.
     * (as returned by getuid(2) or geteuid(2), taking $LOGNAME/$USER into
     * account).
     *
     * \a mode if #UseEffectiveUID is passed the effective
     *             user is returned.
     *        If #UseRealUserID is passed the real user will be
     *        returned.
     *        The real UID will be different than the effective UID in setuid
     *        programs; in
     *        such a case use the effective UID for checking permissions, and
     *        the real UID for displaying information about the user.
     */
    explicit KUser(UIDMode mode = UseEffectiveUID);

    /*!
     * Creates an object for the user with the given user id.
     * If the user does not exist isValid() will return false.
     *
     * \a uid the user id
     */
    explicit KUser(K_UID uid);

    /*!
     * Creates an object for the user with the given user id.
     * If the KUserId object is invalid this one will be, too.
     *
     * \a uid the user id
     */
    explicit KUser(KUserId uid);

    /*!
     * Creates an object that contains information about the user with the given
     * name. If the user does not exist isValid() will return false.
     *
     * \a name the name of the user
     */
    explicit KUser(const QString &name);

    /*!
     * Creates an object that contains information about the user with the given
     * name. If the user does not exist isValid() will return false.
     *
     * \a name the name of the user
     */
    explicit KUser(const char *name);

#ifndef Q_OS_WIN
    /*!
     * Creates an object from a passwd structure.
     * If the pointer is null isValid() will return false.
     *
     * \a p the passwd structure to create the user from
     */
    explicit KUser(const passwd *p);
#endif

    /*!
     * Creates an object from another KUser object
     *
     * \a user the user to create the new object from
     */
    KUser(const KUser &user);

    /*!
     * Copies a user
     *
     * \a user the user to copy
     *
     * Returns this object
     */
    KUser &operator=(const KUser &user);

    /*!
     * Two KUser objects are equal if the userId() are identical.
     * Invalid users never compare equal.
     */
    bool operator==(const KUser &user) const;

    /*!
     * Two KUser objects are not equal if userId() are not identical.
     * Invalid users always compare unequal.
     */
    bool operator!=(const KUser &user) const;

    /*!
     * Returns true if the user is valid. A KUser object can be invalid if
     * you created it with an non-existing uid or name.
     * Returns true if the user is valid
     */
    bool isValid() const;

    /*! Returns the native user id of the user. */
    KUserId userId() const;

    /*! Returns the native user id of the user. */
    KGroupId groupId() const;

    /*!
     * Checks whether the user is the super user (root).
     * Returns true if the user is root
     */
    bool isSuperUser() const;

    /*!
     * The login name of the user.
     * Returns the login name of the user or QString() if user is invalid
     */
    QString loginName() const;

    /*!
     * The path to the user's home directory.
     * Returns the home directory of the user or QString() if the
     *         user is invalid
     */
    QString homeDir() const;

    /*!
     * The path to the user's face file.
     * Returns the path to the user's face file or QString() if no
     *         face has been set
     */
    QString faceIconPath() const;

    /*!
     * The path to the user's login shell.
     * Returns the login shell of the user or QString() if the
     *         user is invalid
     */
    QString shell() const;

    /*!
     * \a maxCount the maximum number of groups to return
     *
     * Returns all groups of the user
     */
    QList<KUserGroup> groups(uint maxCount = KCOREADDONS_UINT_MAX) const;

    /*!
     * \a maxCount the maximum number of groups to return
     *
     * Returns all group names of the user
     */
    QStringList groupNames(uint maxCount = KCOREADDONS_UINT_MAX) const;

    /*!
     * \value FullName
     * \value RoomNumber
     * \value WorkPhone
     * \value HomePhone
     */
    enum UserProperty {
        FullName,
        RoomNumber,
        WorkPhone,
        HomePhone,
    };

    /*!
     * Returns an extended property given by \a which.
     *
     * Under Windows, RoomNumber, WorkPhone and HomePhone are unsupported.
     *
     * Returns a QVariant with the value of the property or an invalid QVariant,
     *         if the property is not set
     */
    QVariant property(UserProperty which) const;

    ~KUser();

    /*!
     * \a maxCount the maximum number of users to return
     *
     * Returns all users of the system.
     */
    static QList<KUser> allUsers(uint maxCount = KCOREADDONS_UINT_MAX);

    /*!
     * \a maxCount the maximum number of users to return
     *
     * Returns all user names of the system.
     */
    static QStringList allUserNames(uint maxCount = KCOREADDONS_UINT_MAX);

private:
    QExplicitlySharedDataPointer<class KUserPrivate> d;
};

Q_DECLARE_TYPEINFO(KUser, Q_RELOCATABLE_TYPE);

/*!
 * \class KUserGroup
 * \inheaderfile KUser
 * \inmodule KCoreAddons
 *
 * \brief Represents a group on your system.
 *
 * This class represents a group on your system. You can either get
 * information about the group of the current user, of fetch information about
 * a group on the system. Instances of this class will be explicitly shared,
 * so copying objects is very cheap and you can safely pass objects by value.
 *
 */
class KCOREADDONS_EXPORT KUserGroup
{
public:
    /*!
     * Create an object from a group name.
     * If the group does not exist, isValid() will return false.
     *
     * \a name the name of the group
     */
    explicit KUserGroup(const QString &name);

    /*!
     * Create an object from a group name.
     * If the group does not exist, isValid() will return false.
     *
     * \a name the name of the group
     */
    explicit KUserGroup(const char *name);

    /*!
     * Creates an object for the group with the given group id.
     * If the KGroupId object is invalid this one will be, too.
     *
     * \a gid the group id
     */
    explicit KUserGroup(KGroupId gid);

    /*!
     * Create an object from the group of the current user.
     *
     * \a mode if #KUser::UseEffectiveUID is passed the effective user
     *        will be used. If #KUser::UseRealUserID is passed the real user
     *        will be used.
     *        The real UID will be different than the effective UID in setuid
     *        programs; in  such a case use the effective UID for checking
     *        permissions, and the real UID for displaying information about
     *        the group associated with the user.
     */
    explicit KUserGroup(KUser::UIDMode mode = KUser::UseEffectiveUID);

    /*!
     * Create an object from a group id.
     * If the group does not exist, isValid() will return false.
     *
     * \a gid the group id
     */
    explicit KUserGroup(K_GID gid);

#ifndef Q_OS_WIN
    /*!
     * Creates an object from a group structure.
     * If the pointer is null, isValid() will return false.
     *
     * \a g the group structure to create the group from.
     */
    explicit KUserGroup(const group *g);
#endif

    /*!
     * Creates a new KUserGroup instance from another KUserGroup object
     *
     * \a group the KUserGroup to copy
     */
    KUserGroup(const KUserGroup &group);

    /*!
     * Copies a group
     *
     * \a group the group that should be copied
     *
     * Returns this group
     */
    KUserGroup &operator=(const KUserGroup &group);

    /*!
     * Returns true if this group's gid() is identical to the one from \a group.
     * Invalid groups never compare equal.
     */
    bool operator==(const KUserGroup &group) const;

    /*!
     * Two KUserGroup objects are not equal if their gid()s are not identical.
     * Invalid groups always compare unequal.
     * Returns true if the groups are not identical
     */
    bool operator!=(const KUserGroup &group) const;

    /*!
     * Returns whether the group is valid.
     * A KUserGroup object can be invalid if it is
     * created with a non-existing gid or name.
     * Returns true if the group is valid
     */
    bool isValid() const;

    /*! Returns the native group id of the user. */
    KGroupId groupId() const;

    /*!
     * The name of the group.
     * Returns the name of the group
     */
    QString name() const;

    /*!
     * \a maxCount the maximum number of users to return
     *
     * Returns a list of all users of the group
     */
    QList<KUser> users(uint maxCount = KCOREADDONS_UINT_MAX) const;

    /*!
     * \a maxCount the maximum number of groups to return
     *
     * Returns a list of all user login names of the group
     */
    QStringList userNames(uint maxCount = KCOREADDONS_UINT_MAX) const;

    /*!
     * Destructor.
     */
    ~KUserGroup();

    /*!
     * \a maxCount the maximum number of groups to return
     *
     * Returns a list of all groups on this system
     */
    static QList<KUserGroup> allGroups(uint maxCount = KCOREADDONS_UINT_MAX);

    /*!
     * \a maxCount the maximum number of groups to return
     *
     * Returns a list of all group names on this system
     */
    static QStringList allGroupNames(uint maxCount = KCOREADDONS_UINT_MAX);

private:
    QSharedDataPointer<class KUserGroupPrivate> d;
};

Q_DECLARE_TYPEINFO(KUserGroup, Q_RELOCATABLE_TYPE);

#if !defined(Q_OS_WIN)
// inline UNIX implementation of KUserOrGroupId
template<typename T>
inline bool KUserOrGroupId<T>::isValid() const
{
    return id != NativeType(-1);
}
template<typename T>
inline bool KUserOrGroupId<T>::operator==(const KUserOrGroupId<T> &other) const
{
    return id == other.id;
}
template<typename T>
inline bool KUserOrGroupId<T>::operator!=(const KUserOrGroupId<T> &other) const
{
    return id != other.id;
}
template<typename T>
inline typename KUserOrGroupId<T>::NativeType KUserOrGroupId<T>::nativeId() const
{
    return id;
}
template<typename T>
inline QString KUserOrGroupId<T>::toString() const
{
    return QString::number(id);
}
template<typename T>
inline KUserOrGroupId<T>::KUserOrGroupId()
    : id(-1)
{
}
template<typename T>
inline KUserOrGroupId<T>::KUserOrGroupId(KUserOrGroupId<T>::NativeType nativeId)
    : id(nativeId)
{
}
template<typename T>
inline KUserOrGroupId<T>::KUserOrGroupId(const KUserOrGroupId<T> &other)
    : id(other.id)
{
}
template<typename T>
inline KUserOrGroupId<T> &KUserOrGroupId<T>::operator=(const KUserOrGroupId<T> &other)
{
    id = other.id;
    return *this;
}
template<typename T>
inline KUserOrGroupId<T>::~KUserOrGroupId()
{
}
#endif // !defined(Q_OS_WIN)

inline bool KUser::operator!=(const KUser &other) const
{
    return !operator==(other);
}

inline bool KUserGroup::operator!=(const KUserGroup &other) const
{
    return !operator==(other);
}

#endif
