/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2013-2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATION_H
#define KNOTIFICATION_H

#include <knotifications_export.h>

#include <QList>
#include <QObject>
#include <QPair>
#include <QPixmap>
#include <QUrl>
#include <QVariant>
#include <QWindow>

#include <memory>

class KNotificationReplyAction;
class KNotificationAction;

class KNotificationActionPrivate;

/*!
 * \class KNotificationAction
 * \inheaderfile KNotification
 * \inmodule KNotifications
 *
 * \brief This class represents an action in a notification.
 *
 * This can be a button on the notification popup,
 * or triggered by clicking the notification popup itself.
 *
 * \since 6.0
 */
class KNOTIFICATIONS_EXPORT KNotificationAction : public QObject
{
    Q_OBJECT
    /*!
     * \property KNotificationAction::label
     */
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)

public:
    explicit KNotificationAction(QObject *parent = nullptr);

    /*!
     * Creates an action with given label
     *
     * \a label The label for the action
     */
    explicit KNotificationAction(const QString &label);

    ~KNotificationAction() override;

    /*!
     * The user-facing label for the action
     */
    QString label() const;

    /*!
     * Set the user-facing label for the action
     */
    void setLabel(const QString &label);

Q_SIGNALS:
    /*!
     * Emitted when the user activates the action
     */
    void activated();

    /*!
     * Emitted when \a label changed.
     */
    void labelChanged(const QString &label);

private:
    friend class KNotification;
    friend class NotifyByPortalPrivate;
    friend class NotifyByPopup;
    friend class NotifyBySnore;
    friend class NotifyByAndroid;

    void setId(const QString &id);
    QString id() const;

    std::unique_ptr<KNotificationActionPrivate> const d;
};

/*!
 * \class KNotification
 * \inmodule KNotifications
 *
 * \brief KNotification is the main class for creating notifications.
 */
class KNOTIFICATIONS_EXPORT KNotification : public QObject
{
    Q_OBJECT
    /*!
     * \property KNotification::eventId
     * \since 5.88
     */
    Q_PROPERTY(QString eventId READ eventId WRITE setEventId NOTIFY eventIdChanged)
    /*!
     * \property KNotification::title
     * \since 5.88
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    /*!
     * \property KNotification::text
     * \since 5.88
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    /*!
     * \property KNotification::iconName
     * \since 5.88
     */
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)
    /*!
     * \property KNotification::flags
     * \since 5.88
     */
    Q_PROPERTY(NotificationFlags flags READ flags WRITE setFlags NOTIFY flagsChanged)
    /*!
     * \property KNotification::componentName
     * \since 5.88
     */
    Q_PROPERTY(QString componentName READ componentName WRITE setComponentName NOTIFY componentNameChanged)
    /*!
     * \property KNotification::urls
     * \since 5.88
     */
    Q_PROPERTY(QList<QUrl> urls READ urls WRITE setUrls NOTIFY urlsChanged)
    /*!
     * \property KNotification::urgency
     * \since 5.88
     */
    Q_PROPERTY(Urgency urgency READ urgency WRITE setUrgency NOTIFY urgencyChanged)
    /*!
     * \property KNotification::autoDelete
     * \since 5.88
     */
    Q_PROPERTY(bool autoDelete READ isAutoDelete WRITE setAutoDelete NOTIFY autoDeleteChanged)
    /*!
     * \property KNotification::xdgActivationToken
     * \since 5.90
     */
    Q_PROPERTY(QString xdgActivationToken READ xdgActivationToken NOTIFY xdgActivationTokenChanged)
    /*!
     * \property KNotification::hints
     * \since 5.101
     */
    Q_PROPERTY(QVariantMap hints READ hints WRITE setHints NOTIFY hintsChanged)

public:
    /*!
     *
     * \value CloseOnTimeout The notification will be automatically closed after a timeout. (this is the default)
     * \value Persistent The notification will NOT be automatically closed after a timeout. You will have to track the notification, and close it with the close
     * function manually when the event is done, otherwise there will be a memory leak.
     * \value LoopSound The audio plugin will loop the sound until the notification is closed.
     * \value [since 5.18] SkipGrouping Sends a hint to Plasma to skip grouping for this notification.
     * \value [since 6.0] CloseWhenWindowActivated The notification will be automatically closed if the window() becomes activated. You need to set a window
     * using setWindow().
     * \value DefaultEvent The event is a standard kde event, and not an event of the application.
     */
    enum NotificationFlag {
        CloseOnTimeout = 0x00,
        Persistent = 0x02,
        LoopSound = 0x08,
        SkipGrouping = 0x10,
        CloseWhenWindowActivated = 0x20,
        DefaultEvent = 0xF000,
    };
    Q_DECLARE_FLAGS(NotificationFlags, NotificationFlag)
    Q_FLAG(NotificationFlags)

    /*!
     * Default events you can use in the event function
     *
     * \value Notification
     * \value Warning
     * \value Error
     * \value Catastrophe
     */
    enum StandardEvent {
        Notification,
        Warning,
        Error,
        Catastrophe,
    };

    /*!
     * The urgency of a notification.
     *
     * \since 5.58
     * \sa setUrgency
     *
     * \value DefaultUrgency
     * \value LowUrgency
     * \value NormalUrgency
     * \value HighUrgency
     * \value CriticalUrgency
     */
    enum Urgency {
        DefaultUrgency = -1,
        LowUrgency = 10,
        NormalUrgency = 50,
        HighUrgency = 70,
        CriticalUrgency = 90,
    };
    Q_ENUM(Urgency)

    /*!
     * Create a new notification.
     *
     * You have to use sendEvent to show the notification.
     *
     * The pointer is automatically deleted when the event is closed.
     *
     * \since 4.4
     *
     * \a eventId is the name of the event
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \a parent parent object
     */
    explicit KNotification(const QString &eventId, NotificationFlags flags = CloseOnTimeout, QObject *parent = nullptr);

    ~KNotification() override;

    /*!
     * Returns the name of the event
     */
    QString eventId() const;
    /*!
     * Set the event id, if not already passed to the constructor.
     * \since 5.88
     */
    void setEventId(const QString &eventId);

    /*!
     * Returns the notification title
     * \sa setTitle
     * \since 4.3
     */
    QString title() const;

    /*!
     * Set the title of the notification popup.
     * If no title is set, the application name will be used.
     *
     * \a title The title of the notification
     * \since 4.3
     */
    void setTitle(const QString &title);

    /*!
     * Returns the notification text
     * \sa setText
     */
    QString text() const;

    /*!
     * Set the notification text that will appear in the popup.
     *
     * In Plasma workspace, the text is shown in a QML label which uses Text.StyledText,
     * ie. it supports a small subset of HTML entities (mostly just formatting tags)
     *
     * If the notifications server does not advertise "body-markup" capability,
     * all HTML tags are stripped before sending it to the server
     *
     * \a text The text to display in the notification popup
     */
    void setText(const QString &text);

    /*!
     * Returns the icon shown in the popup
     * \sa setIconName
     * \since 5.4
     */
    QString iconName() const;

    /*!
     * Set the icon that will be shown in the popup.
     *
     * \a icon the icon
     *
     * \since 5.4
     */
    void setIconName(const QString &icon);

    /*!
     * Returns the pixmap shown in the popup
     * \sa setPixmap
     */
    QPixmap pixmap() const;
    /*!
     * Set the pixmap that will be shown in the popup. If you want to use an icon from the icon theme use setIconName instead.
     *
     * \a pix the pixmap
     */
    void setPixmap(const QPixmap &pix);

    /*!
     * Returns the default action, or nullptr if none is set
     * \since 6.0
     */
    KNotificationAction *defaultAction() const;

    /*!
     * Add a default action that will be triggered when the notification is
     * activated (typically, by clicking on the notification popup). The default
     * action typically raises a window belonging to the application that sent it.
     *
     * The string will be used as a label for the action, so ideally it should
     * be wrapped in i18n() or tr() calls.
     *
     * The visual representation of actions depends on the notification server.
     * In Plasma and Gnome desktops, the actions are performed by clicking on
     * the notification popup, and the label is not presented to the user.
     *
     * Calling this overrides the current default action
     *
     * \since 6.0
     */
    [[nodiscard]] KNotificationAction *addDefaultAction(const QString &label);

    /*!
     * Add an action to the notification.
     *
     * The visual representation of actions depends
     * on the notification server.
     *
     * \a label the user-visible label of the action
     *
     * \sa KNotificationAction
     *
     * \since 6.0
     */
    [[nodiscard]] KNotificationAction *addAction(const QString &label);

    /*!
     * Removes all actions previously added by addAction()
     * from the notification.
     *
     * \sa addAction
     *
     * \since 6.0
     */
    void clearActions();

    /*!
     * Returns the inline reply action.
     * \since 5.81
     */
    KNotificationReplyAction *replyAction() const;

    /*!
     * Add an inline reply action to the notification.
     *
     * On supported platforms this lets the user type a reply to a notification,
     * such as replying to a chat message, from the notification popup, for example:
     *
     * \code
     * KNotification *notification = new KNotification(QStringLiteral("notification"));
     * ...
     * auto replyAction = std::make_unique<KNotificationReplyAction>(i18nc("@action:button", "Reply"));
     * replyAction->setPlaceholderText(i18nc("@info:placeholder", "Reply to Dave..."));
     * QObject::connect(replyAction.get(), &KNotificationReplyAction::replied, [](const QString &text) {
     *     qDebug() << "you replied with" << text;
     * });
     * notification->setReplyAction(std::move(replyAction));
     * \endcode
     *
     * \a replyAction the reply action to set
     *
     * \since 5.81
     */
    void setReplyAction(std::unique_ptr<KNotificationReplyAction> replyAction);

    /*!
     * Returns the notification flags.
     */
    NotificationFlags flags() const;

    /*!
     * Set the notification flags.
     *
     * These must be set before calling sendEvent()
     */
    void setFlags(const NotificationFlags &flags);

    /*!
     * Returns the component name used to determine the location of the configuration file.
     * \since 5.88
     */
    QString componentName() const;
    /*!
     * The componentData is used to determine the location of the config file.
     *
     * If no componentName is set, the app name is used by default
     *
     * \a componentName the new component name
     */
    void setComponentName(const QString &componentName);

    /*!
     * URLs associated with this notification
     * \since 5.29
     */
    QList<QUrl> urls() const;

    /*!
     * Sets URLs associated with this notification
     *
     * For example, a screenshot application might want to provide the
     * URL to the file that was just taken so the notification service
     * can show a preview.
     *
     * \note This feature might not be supported by the user's notification service
     *
     * \a urls A list of URLs
     *
     * \since 5.29
     */
    void setUrls(const QList<QUrl> &urls);

    /*!
     * The urgency of the notification.
     * \since 5.58
     */
    Urgency urgency() const;

    /*!
     * Sets the urgency of the notification.
     *
     * This defines the importance of the notification. For example,
     * a track change in a media player would be a low urgency.
     * "You have new mail" would be normal urgency. "Your battery level
     * is low" would be a critical urgency.
     *
     * Use critical notifications with care as they might be shown even
     * when giving a presentation or when notifications are turned off.
     *
     * \a urgency The urgency.
     *
     * \since 5.58
     */
    void setUrgency(Urgency urgency);

    /*!
     * Sets the window associated with this notification.
     * This is relevant when using the CloseWhenWindowActivated flag.
     *
     * \since 6.0
     */
    void setWindow(QWindow *window);

    /*!
     * The window associated with this notification. nullptr by default.
     * Returns the window set by setWindow()
     *
     * \since 6.0
     */
    QWindow *window() const;

    /*!
     * \internal
     * appname used for the D-Bus object
     */
    QString appName() const;

    /*!
     * Returns whether this notification object will be automatically deleted after closing.
     * \since 5.88
     */
    bool isAutoDelete() const;
    /*!
     * Sets whether this notification object will be automatically deleted after closing.
     * This is on by default for C++, and off by default for QML.
     * \since 5.88
     */
    void setAutoDelete(bool autoDelete);

    /*!
     * Returns the activation token to use to activate a window.
     * \since 5.90
     */
    QString xdgActivationToken() const;

Q_SIGNALS:
    /*!
     * Emitted when the notification is closed.
     *
     * Can be closed either by the user clicking the close button,
     * the timeout running out or when an action was triggered.
     */
    void closed();

    /*!
     * The notification has been ignored
     */
    void ignored();

    /*!
     * Emitted when eventId changed.
     * \since 5.88
     */
    void eventIdChanged();
    /*!
     * Emitted when title changed.
     * \since 5.88
     */
    void titleChanged();
    /*!
     * Emitted when text changed.
     * \since 5.88
     */
    void textChanged();
    /*!
     * Emitted when iconName changed.
     * \since 5.88
     */
    void iconNameChanged();
    /*!
     * Emitted when defaultAction changed.
     * \since 5.88
     */
    void defaultActionChanged();
    /*!
     * Emitted when actions changed.
     * \since 5.88
     */
    void actionsChanged();
    /*!
     * Emitted when flags changed.
     * \since 5.88
     */
    void flagsChanged();
    /*!
     * Emitted when componentName changed.
     * \since 5.88
     */
    void componentNameChanged();
    /*!
     * Emitted when urls changed.
     * \since 5.88
     */
    void urlsChanged();
    /*!
     * Emitted when urgency changed.
     * \since 5.88
     */
    void urgencyChanged();
    /*!
     * Emitted when autoDelete changed.
     * \since 5.88
     */
    void autoDeleteChanged();
    /*!
     * Emitted when xdgActivationToken changes.
     * \since 5.90
     */
    void xdgActivationTokenChanged();
    /*!
     * Emitted when hints changes.
     * \since 5.101
     */
    void hintsChanged();

public Q_SLOTS:
    /*!
     * Close the notification without activating it.
     *
     * This will delete the notification.
     */
    void close();

    /*!
     * Send the notification to the server.
     *
     * This will cause all the configured plugins to execute their actions on this notification
     * (eg. a sound will play, a popup will show, a command will be executed etc).
     */
    void sendEvent();

    /*!
     * \since 5.57
     * Adds a custom hint to the notification. Those are key-value pairs that can be interpreted by the respective notification backend to trigger additional,
     * non-standard features.
     *
     * \a hint the hint's key
     *
     * \a value the hint's value
     */
    Q_INVOKABLE void setHint(const QString &hint, const QVariant &value);

    /*!
     * \since 5.57
     * Returns the custom hints set by setHint()
     */
    QVariantMap hints() const;

    /*!
     * \since 5.101
     * Set custom hints on the notification.
     * \sa setHint
     */
    void setHints(const QVariantMap &hints);

private:
    friend class KNotificationManager;
    friend class NotificationWrapper;
    friend class NotifyByPopup;
    friend class NotifyByPortal;
    friend class NotifyByPortalPrivate;
    friend class NotifyByExecute;
    friend class NotifyBySnore;
    friend class NotifyByAndroid;
    friend class NotifyByMacOSNotificationCenter;
    struct Private;

    KNOTIFICATIONS_NO_EXPORT void slotWindowActiveChanged();

    /*!
     * \internal
     * Activate the action specified action
     * If the action is zero, then the default action is activated
     */
    KNOTIFICATIONS_NO_EXPORT void activate(const QString &action);

    /*!
     * \internal
     * the id given by the notification manager
     */
    KNOTIFICATIONS_NO_EXPORT int id();

    /*!
     * \internal
     * update the texts, the icon, and the actions of one existing notification
     */
    KNOTIFICATIONS_NO_EXPORT void update();

    /*!
     * \internal
     * The notification will automatically be closed if all presentations are finished.
     * if you want to show your own presentation in your application, you should use this
     * function, so it will not be automatically closed when there is nothing to show.
     *
     * Don't forgot to deref, or the notification may be never closed if there is no timeout.
     *
     * \sa deref
     */
    KNOTIFICATIONS_NO_EXPORT void ref();

    /*!
     * \internal
     * Remove a reference made with ref(). If the ref counter hits zero,
     * the notification will be closed and deleted.
     *
     * \sa ref
     */
    KNOTIFICATIONS_NO_EXPORT void deref();

    // Like setActions, but doesn't take ownership
    void setActionsQml(QList<KNotificationAction *> actions);
    void setDefaultActionQml(KNotificationAction *action);
    QList<KNotificationAction *> actions() const;

    static QString standardEventToEventId(StandardEvent event);
    static QString standardEventToIconName(StandardEvent event);

    std::unique_ptr<Private> const d;

public:
    /*!
     * Emit an event
     *
     * This method creates the KNotification, setting every parameter, and fire the event.
     * You don't need to call sendEvent
     *
     * A popup may be displayed or a sound may be played, depending the config.
     *
     * Returns a KNotification .  You may use that pointer to connect some signals or slot.
     * the pointer is automatically deleted when the event is closed.
     *
     * \a eventId is the name of the event
     *
     * \a title is title of the notification to show in the popup.
     *
     * \a text is the text of the notification to show in the popup.
     *
     * \a pixmap is a picture which may be shown in the popup.
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \a componentName used to determine the location of the config file.  by default, appname is used
     * \since 4.4
     */
    static KNotification *event(const QString &eventId,
                                const QString &title,
                                const QString &text,
                                const QPixmap &pixmap = QPixmap(),
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /*!
     * Emit a standard event
     *
     * \overload
     *
     * This will emit a standard event
     *
     * \a eventId is the name of the event
     *
     * \a text is the text of the notification to show in the popup.
     *
     * \a pixmap is a picture which may be shown in the popup.
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \a componentName used to determine the location of the config file.  by default, plasma_workspace is used
     */
    static KNotification *event(const QString &eventId,
                                const QString &text = QString(),
                                const QPixmap &pixmap = QPixmap(),
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /*!
     * Emit a standard event
     *
     * \overload
     *
     * This will emit a standard event
     *
     * \a eventId is the name of the event
     *
     * \a text is the text of the notification to show in the popup
     *
     * \a pixmap is a picture which may be shown in the popup
     *
     * \a flags is a bitmask of NotificationFlag
     */
    static KNotification *
    event(StandardEvent eventId, const QString &text = QString(), const QPixmap &pixmap = QPixmap(), const NotificationFlags &flags = CloseOnTimeout);

    /*!
     * Emit a standard event
     *
     * \overload
     *
     * This will emit a standard event
     *
     * \a eventId is the name of the event
     *
     * \a title is title of the notification to show in the popup.
     *
     * \a text is the text of the notification to show in the popup
     *
     * \a pixmap is a picture which may be shown in the popup
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \since 4.4
     */
    static KNotification *
    event(StandardEvent eventId, const QString &title, const QString &text, const QPixmap &pixmap, const NotificationFlags &flags = CloseOnTimeout);

    /*!
     * Emit a standard event with the possibility of setting an icon by icon name
     *
     * \overload
     *
     * This will emit a standard event
     *
     * \a eventId is the name of the event
     *
     * \a title is title of the notification to show in the popup.
     *
     * \a text is the text of the notification to show in the popup
     *
     * \a iconName a Freedesktop compatible icon name to be shown in the popup
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \a componentName used to determine the location of the config file.  by default, plasma_workspace is used
     *
     * \since 5.4
     */
    static KNotification *event(const QString &eventId,
                                const QString &title,
                                const QString &text,
                                const QString &iconName,
                                const NotificationFlags &flags = CloseOnTimeout,
                                const QString &componentName = QString());

    /*!
     * Emit a standard event with the possibility of setting an icon by icon name
     *
     * \overload
     *
     * This will emit a standard event with a custom icon
     *
     * \a eventId the type of the standard (not app-defined) event
     *
     * \a title is title of the notification to show in the popup.
     *
     * \a text is the text of the notification to show in the popup
     *
     * \a iconName a Freedesktop compatible icon name to be shown in the popup
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \since 5.9
     */
    static KNotification *
    event(StandardEvent eventId, const QString &title, const QString &text, const QString &iconName, const NotificationFlags &flags = CloseOnTimeout);

    /*!
     * Emit a standard event
     *
     * \overload
     *
     * This will emit a standard event with its standard icon
     *
     * \a eventId the type of the standard (not app-defined) event
     *
     * \a title is title of the notification to show in the popup.
     *
     * \a text is the text of the notification to show in the popup
     *
     * \a flags is a bitmask of NotificationFlag
     *
     * \since 5.9
     */
    static KNotification *event(StandardEvent eventId, const QString &title, const QString &text, const NotificationFlags &flags = CloseOnTimeout);

    /*!
     * This is a simple substitution for QApplication::beep()
     *
     * \a reason a short text explaining what has happened (may be empty)
     */
    static void beep(const QString &reason = QString());

    // prevent warning
    using QObject::event;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KNotification::NotificationFlags)

#endif
