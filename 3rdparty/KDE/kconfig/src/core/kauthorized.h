/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 1998, 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KAUTHORIZED_H
#define KAUTHORIZED_H

#include <kconfigcore_export.h>

#include <QMetaEnum>
#include <QObject>
#include <QStringList>
#include <QVariant>

class QUrl;
class QString;
class QQmlEngine;
class QJSEngine;

/*!
 * \class KAuthorized
 * \inmodule KConfigCore
 * \brief Used for checking whether actions are allowed for a user.
 *
 * The functions in this namespace provide the core of the Kiosk action
 * restriction system; the KIO and KXMLGui frameworks build on this.
 *
 * KDE's Kiosk settings allow to control whether a user is allowed
 * to perform actions such as opening files or running shell commands.
 *
 * The relevant settings are read from the application's KSharedConfig
 * instance, so actions can be disabled on a per-application or global
 * basis (by using the kdeglobals file).
 */
class KCONFIGCORE_EXPORT KAuthorized : public QObject
{
    Q_OBJECT
public:
    /*!
     * The enum values lower cased represent the action that is authorized
     * For example the SHELL_ACCESS value is converted to the "shell_access" string.
     *
     * \value SHELL_ACCESS If the user is authorized to open a shell or execute shell commands
     * \value GHNS If the collaborative data sharing framework KNewStuff is authorized GUI behavior
     * \value LINEEDIT_REVEAL_PASSWORD If typed characters in password fields can be made visible
     * \value LINEEDIT_TEXT_COMPLETION If line edits should be allowed to display completions
     * \value MOVABLE_TOOLBARS If toolbars of apps should be movable
     * \value RUN_DESKTOP_FILES If .desktop files should be run as executables when clicked
     *
     * \since 5.88
     */
    enum GenericRestriction {
        SHELL_ACCESS = 1,
        GHNS,
        LINEEDIT_REVEAL_PASSWORD,
        LINEEDIT_TEXT_COMPLETION,
        MOVABLE_TOOLBARS,
        RUN_DESKTOP_FILES,
    };
    Q_ENUM(GenericRestriction)

    /*!
     * \value OPEN_WITH If the open-with menu should be shown for files etc.
     * \value EDITFILETYPE If mime-type accociations are allowed to be configured
     * \value OPTIONS_SHOW_TOOLBAR If the toolbar should be displayed in apps
     * \value SWITCH_APPLICATION_LANGUAGE If an action to switch the app language should be shown
     * \value BOOKMARKS If saving bookmarks is allowed
     * \since 5.88
     */
    enum GenericAction {
        OPEN_WITH = 1,
        EDITFILETYPE,

        OPTIONS_SHOW_TOOLBAR,
        SWITCH_APPLICATION_LANGUAGE,
        BOOKMARKS,
    };
    Q_ENUM(GenericAction)

    /*!
     * Returns whether the user is permitted to perform a certain \a action.
     *
     * All settings are read from the \c {[KDE Action Restrictions]} group.
     * For example, if kdeglobals contains
     * \badcode
     * [KDE Action Restrictions][$i]
     * shell_access=false
     * \endcode
     * then
     * \code
     * KAuthorized::authorize("shell_access");
     * \endcode
     * will return \c false.
     *
     * This method is intended for actions that do not necessarily have a
     * one-to-one correspondence with a menu or toolbar item (ie: a QAction
     * in a KXMLGui application).  "shell_access" is an example of such a
     * "generic" action.
     *
     * The convention for actions like "File->New" is to prepend the action
     * name with "action/", for example "action/file_new".  This is what
     * authorizeAction() does.
     *
     * Returns \c true if the action is authorized, \c false otherwise.
     *
     * \sa authorizeAction()
     */
    Q_INVOKABLE static bool authorize(const QString &action);

    /*!
     * Returns whether the user is permitted to perform a common \a action.
     *
     * The enum values lower cased represent the action that is
     * passed in to KAuthorized::authorize(const QString &action).
     *
     * \overload KAuthorized::authorize(const QString &action)
     * \since 5.88
     */
    Q_INVOKABLE static bool authorize(GenericRestriction action);

    /*!
     * Returns whether the user is permitted to perform a certain \a action.
     *
     * This behaves like authorize(), except that "action/" is prepended to
     * \a action.  So if kdeglobals contains
     * \badcode
     * [KDE Action Restrictions][$i]
     * action/file_new=false
     * \endcode
     * then
     * \code
     * KAuthorized::authorizeAction("file_new");
     * \endcode
     * will return \c false.
     *
     * KXMLGui-based applications should not normally need to call this
     * function, as KActionCollection will do it automatically.
     *
     * Returns \c true if the QAction is authorized, \c false
     *                otherwise.
     * \since 5.24
     *
     * \sa authorize()
     */
    Q_INVOKABLE static bool authorizeAction(const QString &action);

    /*!
     * Overload to authorize a common \a action.
     *
     * \overload KAuthorized::authorizeAction(const QString &action)
     * \since 5.88
     */
    Q_INVOKABLE static bool authorizeAction(GenericAction action);

    /*!
     * Returns whether the user is permitted to use a certain control module with the given \a pluginId.
     *
     * All settings are read from the "[KDE Control Module Restrictions]"
     * group.  For example, if kdeglobals contains
     * \badcode
     * [KDE Control Module Restrictions][$i]
     * kcm_desktop-settings=false
     * \endcode
     * then
     * \code
     * KAuthorized::authorizeControlModule("kcm_desktop-settings");
     * \endcode
     * will return \c false.
     *
     * Returns \c true if access to the module is authorized, \c false otherwise.
     */
    Q_INVOKABLE static bool authorizeControlModule(const QString &pluginId);

    static KAuthorized *create(QQmlEngine *, QJSEngine *)
    {
        return new KAuthorized;
    }

private:
    friend class KConfigQmlPlugin;
    explicit KAuthorized();
};

#endif
