/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDESKTOPFILEACTION_H
#define KDESKTOPFILEACTION_H
#include <kconfigcore_export.h>

#include <QSharedDataPointer>
#include <QString>

class KDesktopFileActionPrivate;

/*!
 * \class KDesktopFileAction
 * \inmodule KConfigCore
 *
 * \brief Class for representing an Action of a desktop file.
 *
 * This represents an Action following the freedesktop
 * \l {https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html}
 * {Desktop Entry Spec}.
 *
 * \since 6.0
 * \sa KDesktopFile
 */
class KCONFIGCORE_EXPORT KDesktopFileAction
{
public:
    /*!
     * Constructs an empty KDesktopFileAction.
     *
     * Needed so the Action can be stored in containers that require type T to be
     * default constructible (such as QVariant).
     */
    explicit KDesktopFileAction();
    /*!
     * Constructs a KDesktopFileAction at the given \a desktopFilePath
     * with all minimum required properties \a name, \a text, \a icon, and \a exec.
     */
    explicit KDesktopFileAction(const QString &name, const QString &text, const QString &icon, const QString &exec, const QString &desktopFilePath);

    KDesktopFileAction(const KDesktopFileAction &other);
    KDesktopFileAction &operator=(const KDesktopFileAction &other);
    KDesktopFileAction(KDesktopFileAction &&other);
    KDesktopFileAction &operator=(KDesktopFileAction &&other);
    ~KDesktopFileAction();

    /*!
     * Returns the action's internal name.
     *
     * For instance "Actions=Setup;"... and the group "[Desktop Action Setup]"
     * define an action with the name "Setup".
     */
    QString actionsKey() const;

    /*!
     * Returns the path of the desktop file this action was loaded from.
     */
    QString desktopFilePath() const;

    /*!
     * Returns the action's name, as defined by the "Name=" key in the desktop action group.
     */
    QString name() const;

    /*!
     * Returns the action's icon, as defined by the "Icon=" key in the desktop action group.
     */
    QString icon() const;

    /*!
     * Returns the action's exec command, as defined by the "Exec=" key in the desktop action group.
     */
    QString exec() const;

    /*!
     * Returns whether the action is a separator.
     *
     * This is \c true when actionsKey contains "_SEPARATOR_".
     */
    bool isSeparator() const;

private:
    QSharedDataPointer<KDesktopFileActionPrivate> d;
};
#endif
