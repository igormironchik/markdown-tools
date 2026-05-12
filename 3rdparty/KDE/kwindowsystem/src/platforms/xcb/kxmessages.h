/*
    SPDX-FileCopyrightText: 2001-2003 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KXMESSAGES_H
#define KXMESSAGES_H

#include <QObject>
#include <kwindowsystem_export.h>

#include <config-kwindowsystem.h> // KWINDOWSYSTEM_HAVE_X11
#if KWINDOWSYSTEM_HAVE_X11
#include <xcb/xcb.h>
typedef struct _XDisplay Display;

class QString;

class KXMessagesPrivate;

/*!
 * Sending string messages to other applications using the X Client Messages.
 *
 * Used internally by KStartupInfo and kstart.
 * You usually don't want to use this, use D-Bus instead.
 *
 * \internal
 */
class KWINDOWSYSTEM_EXPORT KXMessages : public QObject
{
    Q_OBJECT
public:
    /*!
     * Creates an instance which will receive X messages.
     *
     * \a accept_broadcast if non-nullptr, all broadcast messages with
     *                         this message type will be received.
     *
     * \a parent the parent of this widget
     */
    explicit KXMessages(const char *accept_broadcast = nullptr, QObject *parent = nullptr);

    /*!
     * \overload
     * Overload passing in the xcb_connection_t to use instead relying on platform xcb.
     *
     * \a connection The xcb connection
     *
     * \a rootWindow The rootWindow to use
     *
     * \a accept_broadcast if non-nullptr, all broadcast messages with
     *                         this message type will be received.
     * \a parent the parent of this object
     *
     * \since 5.8
     **/
    explicit KXMessages(xcb_connection_t *connection, xcb_window_t rootWindow, const char *accept_broadcast = nullptr, QObject *parent = nullptr);

    ~KXMessages() override;
    /*!
     * Broadcasts the given message with the given message type.
     *
     * \a msg_type the type of the message
     *
     * \a message the message itself
     *
     * \a screen X11 screen to use, -1 for the default
     */
    void broadcastMessage(const char *msg_type, const QString &message, int screen = -1);

    /*!
     * Broadcasts the given message with the given message type.
     *
     * \a c X11 connection which will be used instead of
     *             QX11Info::connection()
     *
     * \a msg_type the type of the message
     *
     * \a message the message itself
     *
     * \a screenNumber X11 screen to use
     *
     * Returns false when an error occurred, true otherwise
     */
    static bool broadcastMessageX(xcb_connection_t *c, const char *msg_type, const QString &message, int screenNumber);

Q_SIGNALS:
    /*!
     * Emitted when a message was received.
     *
     * \a message the message that has been received
     */
    void gotMessage(const QString &message);

private:
    friend class KXMessagesPrivate;
    KXMessagesPrivate *const d;
};

#endif
#endif
