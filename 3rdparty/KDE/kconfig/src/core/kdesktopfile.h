/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Pietro Iglio <iglio@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KDESKTOPFILE_H
#define KDESKTOPFILE_H

#include <kconfig.h>

class KConfigGroup;
class KDesktopFileAction;
class KDesktopFilePrivate;

/*!
 * \class KDesktopFile
 * \inmodule KConfigCore
 *
 * This class implements KDE's support for the freedesktop.org
 * \l {https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html}
 * {Desktop Entry Spec}.
 *
 * \sa KConfigBase
 * \sa KConfig
 * \sa KDesktopFileAction
 */
class KCONFIGCORE_EXPORT KDesktopFile : public KConfig
{
public:
    /*!
     * Constructs a KDesktopFile object in \a resourceType with the given \a fileName.
     *
     * The \a resourceType allows to change where to search
     * for the desktop file if \a fileName is not absolute.
     * For instance, you might want to specify GenericConfigLocation.
     * \sa QStandardPaths::StandardLocation
     */
    explicit KDesktopFile(QStandardPaths::StandardLocation resourceType, const QString &fileName);

    /*!
     * Constructs a KDesktopFile object with the given \a fileName in its standard location QStandardPaths::ApplicationsLocation.
     * \sa QStandardPaths::ApplicationsLocation
     */
    explicit KDesktopFile(const QString &fileName);

    /*!
     * Destructs the KDesktopFile object.
     *
     * Writes back any changed configuration entries.
     */
    ~KDesktopFile() override;

    /*!
     * Returns whether the file in the given \a path is a desktop file.
     *
     * The check is performed looking at the file extension (the file is not
     * opened).
     * Currently, the only valid extension is ".desktop".
     */
    static bool isDesktopFile(const QString &path);

    /*!
     * Returns whether the user is authorized to run desktop file at the given \a path.
     *
     * By default users are authorized to run all desktop files but
     * the Kiosk framework can be used to activate certain restrictions.
     *
     * Note that desktop files that are not in a standard location (as
     * specified by \c XDG_DATA_DIRS) must have their executable bit set
     * to be authorized regardless of Kiosk settings to prevent users
     * from inadvertently running trojan desktop files.
     * \sa KAuthorized
     * \sa {https://develop.kde.org/docs/administration/kiosk/}{Kiosk - Simple configuration management for large deployment}
     */
    static bool isAuthorizedDesktopFile(const QString &path);

    /*!
     * Returns the appropriate location to use to write changes to the desktop file based on the given \a path.
     *
     * If the \a path is non-writable (such as /usr/share/applications),
     * this function returns the appropriate equivalent.
     *
     * If the appropriate location cannot be determined,
     * the writable location will be in QStandardPaths::GenericDataLocation.
     *
     * For example:
     *
     * \table
     * \header
     *   \li Path
     *   \li Writable Location
     * \row
     *   \li /etc/xdg/autostart/myapp.desktop
     *   \li ~/.config/autostart/myapp.desktop
     * \row
     *   \li /usr/share/applications/myapp.desktop
     *   \li ~/.local/share/applications/myapp.desktop
     * \row
     *   \li ~/Documents/myapp.desktop
     *   \li ~/.local/share/myapp.desktop
     * \row
     *   \li myapp.desktop
     *   \li ~/.local/share/myapp.desktop
     * \endtable
     */
    static QString locateLocal(const QString &path);

    /*!
     * Returns the main config group (named "Desktop Entry") in a .desktop file.
     */
    KConfigGroup desktopGroup() const;

    /*!
     * Returns the value of the "Type=" entry, or QString() if not specified.
     */
    QString readType() const;

    /*!
     * Returns the value of the "Icon=" entry, or QString() if not specified.
     */
    QString readIcon() const;

    /*!
     * Returns the value of the "Name=" entry, or QString() if not specified.
     */
    QString readName() const;

    /*!
     * Returns the value of the "Comment=" entry, or QString() if not specified.
     */
    QString readComment() const;

    /*!
     * Returns the value of the "GenericName=" entry, or QString() if not specified.
     */
    QString readGenericName() const;

    /*!
     * Returns the value of the "Path=" entry, or QString() if not specified.
     */
    QString readPath() const;

    /*!
     * Returns the value of the "URL=" entry, or QString() if not specified.
     */
    QString readUrl() const;

    /*!
     * Returns a list of the "Actions=" entries.
     */
    QStringList readActions() const;

    /*!
     * Returns a list of the "MimeType=" entries.
     * \since 5.15
     */
    QStringList readMimeTypes() const;

    /*!
     * Sets a new desktop action \a group.
     */
    KConfigGroup actionGroup(const QString &group);

    /*!
     * Sets a new desktop action \a group.
     */
    KConfigGroup actionGroup(const QString &group) const;

    /*!
     * Returns whether the given action \a group exists.
     */
    bool hasActionGroup(const QString &group) const;

    /*!
     * Returns whether there is a "Type=Link" entry.
     *
     * The link points to the "URL=" entry.
     */
    bool hasLinkType() const;

    /*!
     * Returns whether there is an entry "Type=Application".
     */
    bool hasApplicationType() const;

    /*!
     * Returns whether there is an entry "Type=FSDevice".
     */
    bool hasDeviceType() const;

    /*!
     * Returns whether the "TryExec=" field contains a binary
     * that is found on the local system.
     */
    bool tryExec() const;

    /*!
     * Returns the value of the "X-DocPath=" Or "DocPath=" entry.
     */
    QString readDocPath() const;

    /*!
     * Returns whether the entry should be suppressed in menus.
     *
     * This handles the "NoDisplay=" key.
     * \since 4.1
     */
    bool noDisplay() const;

    /*!
     * Copies all entries from this config object to a new
     * KDesktopFile object that will save itself to \a file.
     *
     * Actual saving to \a file happens when the returned object is
     * destructed or when sync() is called upon it.
     */
    KDesktopFile *copyTo(const QString &file) const;

    /*!
     * Returns the name of the .desktop file that was used to construct this KDesktopFile.
     */
    QString fileName() const;

    /*!
     * Returns the desktop file's actions.
     * \since 6.0
     */
    QList<KDesktopFileAction> actions() const;

private:
    Q_DISABLE_COPY(KDesktopFile)

    Q_DECLARE_PRIVATE(KDesktopFile)
};

#endif
