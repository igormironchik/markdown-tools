/*
    SPDX-FileCopyrightText: 2001-2003 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KSTARTUPINFO_H
#define KSTARTUPINFO_H

#include <kwindowsystem_export.h>

#include <QChildEvent>
#include <QObject>
#include <QString>
#include <QWidgetList> // for WId
#include <QWindow>

#include <sys/types.h>

typedef struct _XDisplay Display;

struct xcb_connection_t;

class KStartupInfoId;
class KStartupInfoData;

/*!
 * \class KStartupInfo
 * \inmodule KWindowSystem
 * \brief Class for manipulating the application startup notification.
 *
 * This class can be used to send information about started application,
 * change the information and receive this information. For detailed
 * description, see \l https://invent.kde.org/frameworks/kwindowsystem/-/blob/master/docs/README.kstartupinfo.
 *
 * You usually don't need to use this class for sending the notification
 * information, as KDE libraries should do this when an application is
 * started (e.g. KRun class).
 *
 * For receiving the startup notification info, create an instance and connect
 * to its slots. It will automatically detect started applications and when
 * they are ready.
 *
 * \sa KStartupInfoId
 * \sa KStartupInfoData
 */
class KWINDOWSYSTEM_EXPORT KStartupInfo : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Manual notification that the application has started.
     *
     * If you do not map a (toplevel) window, then startup
     * notification will not disappear for the application
     * until a timeout. You can use this as an alternative
     * method in this case.
     */
    static void appStarted();

    /*!
     * \brief Sends explicit notification that the startup notification
     * with \a startup_id should end.
     */
    static void appStarted(const QByteArray &startup_id);

    /*!
     * \brief Sets a new value for the application startup notification
     * window property for newly created toplevel windows.
     *
     * \a startup_id The startup notification identifier.
     *
     * \sa KStartupInfo::setNewStartupId
     */
    static void setStartupId(const QByteArray &startup_id);

    /*!
     * \brief Use this function if the application got a request with startup
     * notification from outside (for example, when KUniqueApplication::newInstance()
     * is called, or e.g. when KHelpCenter opens a new URL in its window).
     *
     * The \a window can be either an already existing and visible window,
     * or a new one, before being shown. Note that this function is usually
     * needed only when a window is reused.
     *
     * \a startup_id the startup notification identifier
     */
    static void setNewStartupId(QWindow *window, const QByteArray &startup_id);

    /*!
     * \brief Creates and returns a new startup id. The id includes properly setup
     * user timestamp.
     *
     * On the X11 platform the current timestamp will be fetched from the
     * X-Server. If the caller has an adequate timestamp (e.g. from a QMouseEvent)
     * it should prefer using createNewStartupIdForTimestamp to not trigger a
     * roundtrip to the X-Server.
     *
     * \sa createNewStartupIdForTimestamp
     */
    static QByteArray createNewStartupId();
    /*!
     * \brief Creates and returns new startup id with \a timestamp
     * as user timestamp part.
     *
     * \a timestamp The timestamp for the startup id.
     *
     * \sa createNewStartupId
     * \since 5.5
     **/
    static QByteArray createNewStartupIdForTimestamp(quint32 timestamp);

    enum {
        CleanOnCantDetect = 1 << 0,
        DisableKWinModule = 1 << 1,
        AnnounceSilenceChanges = 1 << 2,
    };

    /*!
     * \brief Creates an instance that will receive the startup notifications,
     * as a child of \a parent.
     *
     * The various \a flags passed may be:
     * \value CleanOnCantDetect
     *        When a new unknown window appears, all startup
     *        notifications for applications that are not compliant with
     *        the startup protocol are removed.
     * \value DisableKWinModule
     *        KWinModule, which is normally used to detect new windows,
     *        is disabled. With this flag, checkStartup() must be
     *        called in order to check newly mapped windows.
     * \value AnnounceSilenceChanges
     *        Normally, startup notifications are "removed"
     *        when they're silenced, and "recreated" when they're resumed.
     *        With this flag, the change is normally announced with gotStartupChange().
     */
    explicit KStartupInfo(int flags, QObject *parent = nullptr);

    ~KStartupInfo() override;
    /*!
     * \brief Sends given notification data about started application
     * with the given startup identification.
     *
     * If no notification for this identification
     * exists yet, it is created, otherwise it's updated. Note that the name field
     * in data is required.
     *
     * Returns \c true if successful, \c false otherwise
     *
     * \a id The id of the application.
     *
     * \a data The application's data.
     *
     * \sa KStartupInfoId
     * \sa KStartupInfoData
     */
    static bool sendStartup(const KStartupInfoId &id, const KStartupInfoData &data);

    /*!
     * \brief Like sendStartup, uses \a conn instead of
     * QX11Info::connection() for sending the info.
     *
     * Returns \c true if successful, \c false otherwise.
     *
     * \a conn The xcb connection of the application. Note that the name field
     * in data is required.
     *
     * \a screen The x11 screen the connection belongs to.
     *
     * \a id The id of the application.
     *
     * \a data The application's data.
     *
     * \since 5.18
     */
    static bool sendStartupXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id, const KStartupInfoData &data);

    /*!
     * \brief Sends given notification data about started application
     * with the given startup identification.
     *
     * This is used for updating the notification
     * info, if no notification for this identification exists, it's ignored.
     *
     * Returns \c true if successful, \c false otherwise.
     *
     * \a id The id of the application.
     *
     * \a data The application's data.
     *
     * \sa KStartupInfoId
     * \sa KStartupInfoData
     */
    static bool sendChange(const KStartupInfoId &id, const KStartupInfoData &data);

    /*!
     * \brief Like sendChange, uses \a conn instead of
     * QX11Info::connection() for sending the info.
     *
     * Returns \c true if successful, \c false otherwise.
     *
     * \a conn The xcb connection of the application.
     *
     * \a screen The x11 screen the connection belongs to.
     *
     * \a id The id of the application.
     *
     * \a data The application's data.
     *
     * \since 5.18
     */
    static bool sendChangeXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id, const KStartupInfoData &data);

    /*!
     * \brief Ends startup notification with the given identification.
     *
     * Returns \c true if successful, \c false otherwise
     *
     * \a id The id of the application.
     */
    static bool sendFinish(const KStartupInfoId &id);

    /*!
     * \brief Like sendFinish , uses \a conn instead of
     * QX11Info::connection() for sending the info.
     *
     * Returns \c true if successful, \c false otherwise.
     *
     * \a conn The xcb connection of the application.
     *
     * \a screen The x11 screen the connection belongs to.
     *
     * \a id The id of the application.
     *
     * \since 5.18
     */
    static bool sendFinishXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id);

    /*!
     * \brief Ends startup notification with the given identification and the given data
     * (e.g.\ PIDs of processes for this startup notification that exited).
     *
     * Returns \c true if successful, \c false otherwise
     *
     * \a id The id of the application.
     *
     * \a data The application's data.
     */
    static bool sendFinish(const KStartupInfoId &id, const KStartupInfoData &data);

    /*!
     * \brief Like sendFinish , uses \a conn instead of
     * QX11Info::connection() for sending the info.
     *
     * Returns \c true if successful, \c false otherwise.
     *
     * \a conn The xcb connection of the application.
     *
     * \a screen The x11 screen the connection belongs to.
     *
     * \a id The id of the application.
     *
     * \a data The application's data.
     *
     * \since 5.18
     */
    static bool sendFinishXcb(xcb_connection_t *conn, int screen, const KStartupInfoId &id, const KStartupInfoData &data);

    /*!
     * \brief Unsets the startup notification environment variable.
     */
    static void resetStartupEnv();
    /*!
     * \enum KStartupInfo::startup_t
     * \value NoMatch
     *        The window doesn't match any existing startup notification.
     * \value Match
     *        The window matches an existing startup notification.
     * \value CantDetect
     *        Unable to detect if the window matches any existing
     *        startup notification.
     */
    enum startup_t {
        NoMatch,
        Match,
        CantDetect
    };
    /*!
     * \brief Checks if the given window \a w matches any existing startup notification.
     */
    startup_t checkStartup(WId w);
    /*!
     * \brief Checks if the given window \a w matches any existing startup notification,
     * if yes, returns the identification in \a id.
     */
    startup_t checkStartup(WId w, KStartupInfoId &id);
    /*!
     * \brief Checks if the given window \a w matches any existing startup notification,
     * if yes, returns the notification data in \a data.
     */
    startup_t checkStartup(WId w, KStartupInfoData &data);
    /*!
     * \brief Checks if the given window \a w matches any existing startup notification,
     * if yes, returns the identification in \a id and notification data in \a data.
     */
    startup_t checkStartup(WId w, KStartupInfoId &id, KStartupInfoData &data);
    /*!
     * \brief Sets the timeout for notifications in \a secs,
     * after this timeout a notification is removed.
     */
    void setTimeout(unsigned int secs);
    /*!
     * \brief Returns startup notification identification of the given window \a w.
     *
     * Returns the startup notification id. Can be null if not found.
     */
    static QByteArray windowStartupId(WId w);
    /*!
     * \internal
     */
    class Data;

    /*!
     * \internal
     */
    class Private;
Q_SIGNALS:
    /*!
     * \brief Emitted when a new startup notification is created
     * (i.e. a new application is being started).
     *
     * \a id The notification identification.
     *
     * \a data The notification data.
     */
    void gotNewStartup(const KStartupInfoId &id, const KStartupInfoData &data);
    /*!
     * \brief Emitted when a startup notification changes.
     *
     * \a id The notification identification.
     *
     * \a data The notification data.
     */
    void gotStartupChange(const KStartupInfoId &id, const KStartupInfoData &data);
    /*!
     * \brief Emitted when a startup notification is removed
     * (either because it was detected
     * that the application is ready or because of a timeout).
     *
     * \a id The notification identification.
     *
     * \a data The notification data.
     */
    void gotRemoveStartup(const KStartupInfoId &id, const KStartupInfoData &data);

protected:
    void customEvent(QEvent *e_P) override;

private:
    Q_PRIVATE_SLOT(d, void startups_cleanup())
    Q_PRIVATE_SLOT(d, void startups_cleanup_no_age())
    Q_PRIVATE_SLOT(d, void got_message(const QString &msg))
    Q_PRIVATE_SLOT(d, void window_added(WId w))
    Q_PRIVATE_SLOT(d, void slot_window_added(WId w))

    Private *const d;

    Q_DISABLE_COPY(KStartupInfo)
};

/*!
 * \class KStartupInfoId
 * \inheaderfile KStartupInfo
 * \inmodule KWindowSystem
 * \brief Class representing an identification of application startup notification.
 *
 * Every existing notification about a starting application has its own unique
 * identification, that's used to identify and manipulate the notification.
 *
 * \sa KStartupInfo
 * \sa KStartupInfoData
 */
class KWINDOWSYSTEM_EXPORT KStartupInfoId
{
public:
    /*!
     * \brief Overloaded operator.
     *
     * Returns \c true if the given notification \a id is the same.
     */
    bool operator==(const KStartupInfoId &id) const;
    /*!
     * \brief Overloaded operator.
     *
     * Returns \c true if the given notification \a id is different.
     */
    bool operator!=(const KStartupInfoId &id) const;
    /*!
     * \brief Checks whether the identifier is valid.
     *
     * Returns true if this object doesn't represent a valid notification identification.
     */
    bool isNull() const;

    /*!
     * Initializes this object with the given identification (which may be also "0"
     * for no notification), or if "" is given, tries to read it from the startup
     * notification environment variable, and if it's not set, creates a new one.
     *
     * \a id The new identification, "0" for no notification or "" to read
     *       the environment variable.
     */
    void initId(const QByteArray &id = "");
    /*!
     * Returns the notification identifier as string.
     */
    const QByteArray &id() const;
    /*!
     * Returns the user timestamp for the startup notification, or 0 if no timestamp
     * is set.
     */
    unsigned long timestamp() const;
    /*!
     * Sets the startup notification environment variable to this identification.
     *
     * Returns \c true if successful, \c false otherwise.
     */
    bool setupStartupEnv() const;
    /*!
     * Creates an empty identification
     */
    KStartupInfoId();
    /*!
     * Copy constructor.
     */
    KStartupInfoId(const KStartupInfoId &data);
    ~KStartupInfoId();
    KStartupInfoId &operator=(const KStartupInfoId &data);
    bool operator<(const KStartupInfoId &id) const;

private:
    explicit KStartupInfoId(const QString &txt);
    friend class KStartupInfo;
    friend class KStartupInfo::Private;
    struct Private;
    Private *const d;
};

/*!
 * \class KStartupInfoData
 * \inheaderfile KStartupInfo
 * \inmodule KWindowSystem
 * \brief Class representing data about an application startup notification.
 *
 * Such data include the icon of the starting application, the desktop on which
 * the application should start, the binary name of the application, etc.
 *
 * \sa KStartupInfo
 * \sa KStartupInfoId
 */
class KWINDOWSYSTEM_EXPORT KStartupInfoData
{
public:
    /*!
     * Sets the binary name \a bin of the application (e.g. 'kcontrol').
     */
    void setBin(const QString &bin);
    /*!
     * \brief Returns the binary name of the starting application.
     */
    const QString &bin() const;
    /*!
     * Sets the \a name for the notification (e.g. 'Control Center').
     */
    void setName(const QString &name);
    /*!
     * \brief Returns the name of the startup notification.
     *
     * If it's not available, it tries to use other information (binary name).
     */
    const QString &findName() const;
    /*!
     * Returns the name of the startup notification,
     * or an empty string if not set.
     */
    const QString &name() const;
    /*!
     * Sets the description \a descr for the notification (e.g. 'Launching Control Center').
     *
     * That is, name() describes what is being started, while description() is
     * the actual action performed by the starting.
     */
    void setDescription(const QString &descr);
    /*!
     * Returns the description of the startup notification.
     *
     * If it's not available, it returns name().
     */
    const QString &findDescription() const;
    /*!
     * Returns the name of the startup notification,
     * or an empty string if not set.
     */
    const QString &description() const;
    /*!
     * Sets the \a icon for the startup notification (e.g.\ 'kcontrol').
     */
    void setIcon(const QString &icon);
    /*!
     * Returns the icon of the startup notification, and if it's not available,
     * tries to get it from the binary name.
     */
    const QString &findIcon() const;
    /*!
     * Returns the name of the startup notification icon,
     * or an empty string if not set.
     */
    const QString &icon() const;
    /*!
     * Sets the \a desktop for the startup notification (i.e. the desktop on which
     * the starting application should appear).
     */
    void setDesktop(int desktop);
    /*!
     * Returns the desktop for the startup notification.
     */
    int desktop() const;
    /*!
     * \brief Sets a WM_CLASS value \a wmclass for the startup notification.
     *
     * It may be used for increasing the chance that the windows created
     * by the starting application will be detected correctly.
     */
    void setWMClass(const QByteArray &wmclass);
    /*!
     * Returns the WM_CLASS value for the startup notification, or binary name if not
     * available.
     */
    const QByteArray findWMClass() const;
    /*!
     * Returns the WM_CLASS value for the startup notification,
     * or empty if not available.
     */
    QByteArray WMClass() const;
    /*!
     * Adds a PID \a pid to the list of processes that belong to the startup notification.
     *
     * It may be used to increase the chance that the windows created by the starting
     * application will be detected correctly, and also for detecting if the application
     * has quit without creating any window.
     */
    void addPid(pid_t pid);
    /*!
     * Returns all PIDs for the startup notification.
     */
    QList<pid_t> pids() const;
    /*!
     * Returns \c true if the given \a pid is in the list of PIDs for the startup notification
     */
    bool is_pid(pid_t pid) const;
    /*!
     * \brief Sets the \a hostname on which the application is starting.
     *
     * It's necessary to set it if PIDs are set.
     * If it's a null string, the current hostname is used.
     */
    void setHostname(const QByteArray &hostname = QByteArray());
    /*!
     * Returns the hostname for the startup notification.
     */
    QByteArray hostname() const;

    /*!
     * \enum KStartupInfoData::TriState
     * \value Yes
     * \value No
     * \value Unknown
     */
    enum TriState {
        Yes,
        No,
        Unknown
    };

    /*!
     * Sets whether the visual feedback for this startup notification
     * with the given \a state should be silenced (temporarily suspended).
     */
    void setSilent(TriState state);

    /*!
     * Returns KStartupInfoData::Yes if visual feedback
     * for the startup notification is silenced.
     */
    TriState silent() const;

    /*!
     * The X11 screen on which the startup notification is happening, -1 if unknown.
     */
    int screen() const;

    /*!
     * Sets the X11 \a screen on which the startup notification should happen.
     *
     * This is usually not necessary to set, as it's set by default to QX11Info::screen().
     */
    void setScreen(int screen);

    /*!
     * The Xinerama screen for the startup notification, -1 if unknown.
     */
    int xinerama() const;

    /*!
     * Sets the \a xinerama screen for the startup notification (i.e. the screeen on which
     * the starting application should appear).
     */
    void setXinerama(int xinerama);

    /*!
     * The .desktop file used to initiate this startup notification, or empty.
     *
     * This information should be used only to identify the application,
     * not to read any additional information.
     * \since 4.5
     **/
    QString applicationId() const;

    /*!
     * Sets the \a desktop file that was used to initiate the startup notification.
     * \since 4.5
     */
    void setApplicationId(const QString &desktop);

    /*!
     * Updates the notification data from the given \a data.
     *
     * Some data, such as the desktop or the name,
     * won't be rewritten if already set.
     */
    void update(const KStartupInfoData &data);

    /*!
     * Constructor. Initializes all the data to their default empty values.
     */
    KStartupInfoData();

    /*!
     * Copy constructor.
     */
    KStartupInfoData(const KStartupInfoData &data);
    ~KStartupInfoData();
    KStartupInfoData &operator=(const KStartupInfoData &data);

private:
    explicit KStartupInfoData(const QString &txt);
    friend class KStartupInfo;
    friend class KStartupInfo::Data;
    friend class KStartupInfo::Private;
    struct Private;
    Private *const d;
};

#endif
