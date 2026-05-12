// SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KCONFIGTYPES_H
#define KCONFIGTYPES_H

#include <QQmlEngine>

#include <kauthorized.h>
#include <kconfigskeleton.h>
#include <kcoreconfigskeleton.h>

/*!
 * \qmlsingletontype KAuthorized
 * \inqmlmodule org.kde.config
 * \nativetype KAuthorized
 * \brief Singleton used for checking whether actions are allowed by a user.
 *
 * KDE's Kiosk settings allow to control whether a user is allowed
 * to perform actions such as opening files or running shell commands.
 *
 * This singleton can be used to implement per-application controls for what the user is allowed to do.
 *
 * The relevant settings are read from the application's KSharedConfig
 * instance, so actions can be disabled on a per-application or global
 * basis (by using the kdeglobals file).
 * \sa {https://develop.kde.org/docs/administration/kiosk/}{Kiosk - Simple configuration management for large deployment}
 */

/*!
 * \qmlproperty enumeration KAuthorized::GenericRestriction
 * \qmlenumeratorsfrom KAuthorized::GenericRestriction
 * \brief The enum values lower cased represent the action that is authorized.
 *
 * For example the KAuthorized.SHELL_ACCESS value is converted to the "shell_access" string.
 *
 * \value SHELL_ACCESS If the user is authorized to open a shell or execute shell commands
 * \value GHNS If the collaborative data sharing framework KNewStuff is authorized GUI behavior
 * \value LINEEDIT_REVEAL_PASSWORD If typed characters in password fields can be made visible
 * \value LINEEDIT_TEXT_COMPLETION If line edits should be allowed to display completions
 * \value MOVABLE_TOOLBARS If toolbars of apps should be movable
 * \value RUN_DESKTOP_FILES If .desktop files should be run as executables when clicked
 *
 * \since 5.88
 *
 */

/*!
 * \qmlproperty enumeration KAuthorized::GenericAction
 * \qmlenumeratorsfrom KAuthorized::GenericAction
 * \brief Actions pertaining to the KDE Kiosk system that are prepended with "action/".
 * \value OPEN_WITH If the open-with menu should be shown for files etc.
 * \value EDITFILETYPE If mime-type accociations are allowed to be configured
 * \value OPTIONS_SHOW_TOOLBAR If the toolbar should be displayed in apps
 * \value SWITCH_APPLICATION_LANGUAGE If an action to switch the app language should be shown
 * \value BOOKMARKS If saving bookmarks is allowed
 * \since 5.88
 */


/*!
 * \qmlmethod void KAuthorized::authorize(string action)
 * \brief Returns whether the user is permitted to perform a certain \a action.
 *
 * All settings are read from the \c {[KDE Action Restrictions]} group.
 * For example, if kdeglobals contains
 * \badcode
 * [KDE Action Restrictions][$i]
 * shell_access=false
 * \endcode
 * then
 * \code
 * KAuthorized.authorize("shell_access");
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
 * \sa authorizeAction()
 * \sa GenericRestriction
 */

/*!
 * \qmlmethod void KAuthorized::authorize(KAuthorized.GenericRestriction action)
 * \overload KAuthorized::authorize(string action)
 *
 * Returns whether the user is permitted to perform a common \a action.
 *
 * The enum values lower cased represent the action that is
 * passed in to KAuthorized::authorize(const QString &action).
 * \since 5.88
 */

/*!
 * \qmlmethod void KAuthorized::authorizeAction(string action)
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
 * \since 5.24
 * \sa authorize()
 */

/*!
 * \qmlmethod void KAuthorized::authorizeAction(KAuthorized.GenericAction action)
 * \overload KAuthorized::authorize(string action)
 * \since 5.88
 */

/*!
 * \qmlmethod void KAuthorized::authorizeControlModule(string pluginId)
 *
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
 */


struct KAuthorizedForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(KAuthorized)
    QML_SINGLETON
    QML_FOREIGN(KAuthorized)
};

struct KCoreConfigSkeletonForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(KCoreConfigSkeleton)
};

struct KConfigSkeletonForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(KConfigSkeleton)
};

#endif
