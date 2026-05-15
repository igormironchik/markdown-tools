/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 Ian Reinhart Geiser <geiseri@yahoo.com>
    SPDX-FileCopyrightText: 2006 Thiago Macieira <thiago@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMAINWINDOWIFACE_P_H
#define KMAINWINDOWIFACE_P_H

#include <QDBusAbstractAdaptor>

class KXmlGuiWindow;

/*!
 * \brief D-Bus interface to KMainWindow.
 * \inmodule KXmlGui
 *
 * This is the main interface to the KMainWindow.  This provides a consistent
 * D-Bus interface to all KDE applications that use it.
 */
class KMainWindowInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KMainWindow")

public:
    /*!
    \brief Constructs a new interface object.

    \a mainWindow - The parent KMainWindow object
    that will provide us with the QAction objects.
    */
    explicit KMainWindowInterface(KXmlGuiWindow *mainWindow);

    /*!
    \brief Destructor.

    Cleans up the dcop action proxy object.
    */
    ~KMainWindowInterface() override;

public Q_SLOTS:
    /*!
    \brief Returns a list of actions available to the application's window.
    */
    QStringList actions();

    /*!
    \brief Activates the requested \a action.

    The names of valid actions can be found by calling actions().

    Returns the success of the operation.
    */
    bool activateAction(const QString &action);

    /*!
    \brief Disables the requested \a action.

    The names of valid actions can be found by calling actions().

    Returns the success of the operation.
    */
    bool disableAction(const QString &action);

    /*!
    \brief Enables the requested \a action.

    The names of valid actions can be found by calling actions().

    Returns the success of the operation.
    */
    bool enableAction(const QString &action);

    /*!
    \brief Returns the status of the requested \a action.

    The names of valid actions can be found by calling actions().
    */
    bool actionIsEnabled(const QString &action);

    /*!
    \brief Returns the tool tip text of the requested \a action.
    The names of valid actions can be found by calling actions().
    */
    QString actionToolTip(const QString &action);

    /*!
    \brief Returns the ID of the current main window.

    This is useful for automated screen captures or other evil
    widget fun.

    Returns A integer value of the main window's ID.
    **/
    qlonglong winId();

    /*!
    \brief Copies a pixmap representation of the current main window to
    the clipboard.
    **/
    void grabWindowToClipBoard();

private:
    KXmlGuiWindow *m_MainWindow;
};

#endif // KMAINWINDOWIFACE_P_H
