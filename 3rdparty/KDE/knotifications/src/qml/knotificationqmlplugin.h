/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONQMLPLUGIN_H
#define KNOTIFICATIONQMLPLUGIN_H

#include <QQmlEngine>

#include <KNotification>
#include <KNotificationPermission>
#include <KNotificationReplyAction>

/*!
 * \qmltype NotificationAction
 * \inqmlmodule org.kde.notifications
 * \nativetype KNotificationAction
 */
struct NotificationActionForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(NotificationAction)
    QML_FOREIGN(KNotificationAction);
};

/*!
 * \qmltype NotificationReplyAction
 * \inqmlmodule org.kde.notifications
 * \nativetype KNotificationReplyAction
 */
struct NotificationReplyActionForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(NotificationReplyAction)
    QML_UNCREATABLE("")
    QML_FOREIGN(KNotificationReplyAction);
};

/*!
 * \qmltype Notification
 * \inqmlmodule org.kde.notifications
 * \nativetype KNotification
 */
class NotificationWrapper : public KNotification
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Notification)

    /*!
     * \qmlproperty NotificationReplyAction Notification::replyAction
     */
    Q_PROPERTY(KNotificationReplyAction *replyAction READ replyActionFactory CONSTANT)

    /*!
     * \qmlproperty list<NotificationAction> Notification::actions
     */
    Q_PROPERTY(QQmlListProperty<KNotificationAction> actions READ actionsProperty NOTIFY actionsChanged)

    /*!
     * \qmlproperty NotificationAction Notification::defaultAction
     */
    Q_PROPERTY(KNotificationAction *defaultAction READ defaultAction WRITE setDefaultActionQml NOTIFY defaultActionChanged)
public:
    explicit NotificationWrapper(QObject *parent = nullptr);

    KNotificationReplyAction *replyActionFactory();

    int actionCount() const;

    KNotificationAction *actionAt(qsizetype index);

    QQmlListProperty<KNotificationAction> actionsProperty() const;

    static qsizetype actionsCount(QQmlListProperty<KNotificationAction> *list);

    static void appendAction(QQmlListProperty<KNotificationAction> *list, KNotificationAction *value);

    static KNotificationAction *actionAt(QQmlListProperty<KNotificationAction> *list, qsizetype index);

    static void clearActions(QQmlListProperty<KNotificationAction> *list);

private:
    QQmlListProperty<KNotificationAction> m_actionsProperty;
};

/*!
 * \qmltype NotificationPermission
 * \inqmlmodule org.kde.notifications
 *
 * \brief Check or request permissions to show notifications on platforms where
 * that is necessary.
 */
class NotificationPermissionWrapper : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(NotificationPermission)
    QML_SINGLETON
public:
    /*!
     * \qmlmethod bool NotificationPermission::checkPermission()
     *
     * Check if the current application has permissions to show notifications.
     */
    Q_INVOKABLE bool checkPermission()
    {
        return KNotificationPermission::checkPermission() == Qt::PermissionStatus::Granted;
    }

    /*!
     * \qmlmethod void NotificationPermission::requestPermission(var callback)
     *
     * Request notification permissions.
     */
    Q_INVOKABLE void requestPermission(const QJSValue &callback)
    {
        KNotificationPermission::requestPermission(this, [callback](Qt::PermissionStatus status) {
            callback.call({status == Qt::PermissionStatus::Granted});
        });
    }
};

#endif // KNOTIFICATIONQMLPLUGIN_H
