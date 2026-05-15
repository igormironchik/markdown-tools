/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 1999-2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICE_H
#define KSERVICE_H

#include "kserviceaction.h"
#include <QCoreApplication>
#include <QStringList>
#include <QVariant>
#include <kserviceconversioncheck_p.h>
#include <ksycocaentry.h>

#include <optional>

class QDataStream;
class KDesktopFile;
class QWidget;

class KServicePrivate;

/*!
 * \class KService
 * \inmodule KService
 *
 * \brief Represents an installed application.
 *
 * To obtain a KService instance for a specific application you typically use serviceByDesktopName(), e.g.:
 *
 * \code
 * KService::Ptr service = KService::serviceByDesktopName("org.kde.kate");
 * \endcode
 *
 * Other typical usage would be in combination with KApplicationTrader to obtain e.g. the default application for a given file type.
 *
 * See the \l {https://specifications.freedesktop.org/desktop-entry-spec/latest/} {Desktop Entry Specification}
 */
class KSERVICE_EXPORT KService : public KSycocaEntry
{
public:
    /*!
     * A shared data pointer for KService.
     */
    typedef QExplicitlySharedDataPointer<KService> Ptr;
    /*!
     * A list of shared data pointers for KService.
     */
    typedef QList<Ptr> List;

    /*!
     * Construct a temporary service with a given name, exec-line and icon.
     *
     * \a name the name of the service
     *
     * \a exec the executable
     *
     * \a icon the name of the icon
     */
    KService(const QString &name, const QString &exec, const QString &icon);

    /*!
     * Construct a service and take all information from a .desktop file.
     *
     * \a fullpath Full path to the .desktop file.
     */
    explicit KService(const QString &fullpath);

    /*!
     * Construct a service and take all information from a desktop file.
     *
     * \a config the desktop file to read
     *
     * \a entryPath optional relative path to store for findByName
     */
    explicit KService(const KDesktopFile *config, const QString &entryPath = QString());

    KService(const KService &other);

    ~KService() override;

    /*!
     * Whether this service is an application
     *
     * Returns \c true if this service is an application, i.e. it has \c Type=Application in its
     * .desktop file and exec() will not be empty.
     */
    bool isApplication() const;

    /*!
     * Returns the command that the service executes,
     *         or QString() if not set
     */
    QString exec() const;

    /*!
     * Returns the name of the icon associated with the service,
     *         or QString() if not set
     */
    QString icon() const;
    /*!
     * Checks whether the application should be run in a terminal.
     *
     * This corresponds to \c Terminal=true in the .desktop file.
     *
     * Returns \c true if the application should be run in a terminal.
     */
    bool terminal() const;

    /*!
     * Returns any options associated with the terminal the application
     * runs in, if it requires a terminal.
     *
     * The application must be a TTY-oriented program.
     */
    QString terminalOptions() const;

    /*!
     * Returns \c true if the application indicates that it's preferred to run
     * on a discrete graphics card, otherwise return \c false.
     *
     * In releases older than 5.86, this method checked for the \c X-KDE-RunOnDiscreteGpu
     * key in the .desktop file represented by this service; starting from 5.86 this method
     * now also checks for \c PrefersNonDefaultGPU key (added to the Freedesktop.org desktop
     * entry spec in version 1.4 of the spec).
     *
     * \since KService 5.30
     */
    bool runOnDiscreteGpu() const;

    /*!
     * Returns whether the application needs to run under a different UID.
     * \sa username()
     */
    bool substituteUid() const;
    /*!
     * Returns the user name if the application runs with a
     * different user id.
     * \sa substituteUid()
     */
    QString username() const;

    /*!
     * Returns the filename of the desktop entry without any
     * extension, e.g. "org.kde.kate"
     */
    QString desktopEntryName() const;

    /*!
     * Returns the menu ID of the application desktop entry.
     *
     * The menu ID is used to add or remove the entry to a menu.
     */
    QString menuId() const;

    /*!
     * Returns a normalized ID suitable for storing in configuration files.
     *
     * It will be based on the menu-id when available and otherwise falls
     * back to entryPath()
     */
    QString storageId() const;

    /*!
     * Returns the working directory to run the program in,
     *         or QString() if not set
     * \since 5.63
     */
    QString workingDirectory() const;

    /*!
     * Returns the descriptive comment for the application, if there is one.
     *         if not set
     */
    QString comment() const;

    /*!
     * Returns the generic name for the application, if there is one
     * (e.g. "Mail Client").
     */
    QString genericName() const;

    /*!
     * Returns the untranslated (US English) generic name
     * for the application, if there is one
     * (e.g. "Mail Client").
     */
    QString untranslatedGenericName() const;

    /*!
     * Returns the untranslated name for the given service
     *
     * \since 6.0
     */
    QString untranslatedName() const;
    /*!
     * Returns a list of descriptive keywords for the application, if there are any.
     */
    QStringList keywords() const;

    /*!
     * Returns a list of VFolder categories.
     */
    QStringList categories() const;

    /*!
     * Returns the list of MIME types that this application supports.
     *
     * Note that this doesn't include inherited MIME types,
     * only the MIME types listed in the .desktop file.
     *
     * \since 4.8.3
     */
    QStringList mimeTypes() const;

    /*!
     * Returns the list of scheme handlers this application supports.
     *
     * For example a web browser could return {"http", "https"}.
     *
     * This is taken from the x-scheme-handler MIME types
     * listed in the .desktop file.
     *
     * \since 6.0
     */
    QStringList schemeHandlers() const;

    /*!
     * Returns the list of protocols this application supports.
     *
     * This is taken from the x-scheme-handler MIME types
     * listed in the .desktop file as well as the 'X-KDE-Protocols'
     * entry
     *
     * For example a web browser could return {"http", "https"}.
     *
     * \since 6.0
     */
    QStringList supportedProtocols() const;

    /*!
     * Returns whether the application supports this MIME type
     *
     * \a mimeType The name of the MIME type you are
     *        interested in determining whether this service supports.
     *
     * \since 4.6
     */
    bool hasMimeType(const QString &mimeType) const;

    /*!
     * Returns the actions defined in this desktop file
     *
     * Only valid actions according to specification are included.
     *
     * \note This function copies the action list.
     */
    QList<KServiceAction> actions() const;

    /*!
     * Returns whether this application can handle several files as
     * startup arguments.
     */
    bool allowMultipleFiles() const;

    /*!
     * Returns whether the entry should be hidden from the menu.
     *
     * Such services still appear in trader queries, i.e. in
     * "Open With" popup menus for instance.
     */
    bool noDisplay() const;

    /*!
     * Returns whether the application should be shown in the current desktop
     * (including in context menus).
     *
     * KApplicationTrader honors this and removes such services
     * from its results.
     *
     * \since 5.0
     */
    bool showInCurrentDesktop() const;

    /*!
     * Returns whether the application should be shown on the current
     * platform (e.g. on xcb or on wayland).
     *
     * \since 5.0
     */
    bool showOnCurrentPlatform() const;

    /*!
     * Returns the path to the documentation for this application.
     *
     * \since 4.2
     */
    QString docPath() const;

    /*!
     * Returns the requested property.
     *
     * T the type of the requested property.
     *
     * \a name the name of the property.
     *
     * \since 6.0
     */
    template<typename T>
    T property(const QString &name) const
    {
        KServiceConversionCheck::to_QVariant<T>();
        return property(name, static_cast<QMetaType::Type>(qMetaTypeId<T>())).value<T>();
    }

    /*!
     * Returns a path that can be used for saving changes to this
     * application
     */
    QString locateLocal() const;

    /*!
     * \internal
     * Set the menu id
     */
    void setMenuId(const QString &menuId);
    /*!
     * \internal
     * Sets whether to use a terminal or not
     */
    void setTerminal(bool b);
    /*!
     * \internal
     * Sets the terminal options to use
     */
    void setTerminalOptions(const QString &options);

    /*!
     * Overrides the "Exec=" line of the service.
     *
     * If \a exec is not empty, its value will override the one
     * the one set when this application was created.
     *
     * Please note that entryPath is also cleared so the application
     * will no longer be associated with a specific config file.
     *
     * \internal
     * \since 4.11
     */
    void setExec(const QString &exec);

    /*!
     * Overrides the "Path=" line of the application.
     *
     * If \a workingDir is not empty, its value will override
     * the one set when this application was created.
     *
     * Please note that entryPath is also cleared so the application
     * will no longer be associated with a specific config file.
     *
     * \internal
     * \since 5.79
     */
    void setWorkingDirectory(const QString &workingDir);

    /*!
     * Find a application based on its path as returned by entryPath().
     *
     * It's usually better to use serviceByStorageId() instead.
     *
     * \a _path the path of the configuration file
     *
     * Returns a pointer to the requested application or \c nullptr if the application is
     *         unknown.
     *
     * Very important: Don't store the result in a KService* !
     */
    static Ptr serviceByDesktopPath(const QString &_path);

    /*!
     * Find an application by the name of its desktop file, not depending on
     * its actual location (as long as it's under the applications or application
     * directories). For instance "konqbrowser" or "kcookiejar". Note that
     * the ".desktop" extension is implicit.
     *
     * This is the recommended method (safe even if the user moves stuff)
     * but note that it assumes that no two entries have the same filename.
     *
     * \a _name the name of the configuration file
     *
     * Returns a pointer to the requested application or \c nullptr if the application is
     *         unknown.
     *
     * Very important: Don't store the result in a KService* !
     */
    static Ptr serviceByDesktopName(const QString &_name);

    /*!
     * Find a application by its menu-id
     *
     * \a _menuId the menu id of the application
     *
     * Returns a pointer to the requested application or \c nullptr if the application is
     *         unknown.
     *
     * Very important: Don't store the result in a KService* !
     */
    static Ptr serviceByMenuId(const QString &_menuId);

    /*!
     * Find a application by its storage-id or desktop-file path. This
     * function will try very hard to find a matching application.
     *
     * \a _storageId the storage id or desktop-file path of the application
     *
     * Returns a pointer to the requested application or \c nullptr if the application is
     *         unknown.
     *
     * Very important: Don't store the result in a KService* !
     */
    static Ptr serviceByStorageId(const QString &_storageId);

    /*!
     * Returns the whole list of applications.
     *
     * Useful for being able to
     * to display them in a list box, for example.
     *
     * More memory consuming than the ones above, don't use unless
     * really necessary.
     */
    static List allServices();

    /*!
     * Returns a path that can be used to create a new KService based
     * on \a suggestedName.
     *
     * \a showInMenu \c true, if the application should be shown in the KDE menu
     *        \c false, if the application should be hidden from the menu
     *        This argument isn't used anymore, use \c NoDisplay=true to hide the application.
     *
     * \a suggestedName name to base the file on, if an application with such a
     *        name already exists, a suffix will be added to make it unique
     *        (e.g. foo.desktop, foo-1.desktop, foo-2.desktop).
     *
     * \a menuId If provided, menuId will be set to the menu id to use for
     *        the KService
     *
     * \a reservedMenuIds If provided, the path and menu id will be chosen
     *        in such a way that the new menu id does not conflict with any
     *        of the reservedMenuIds
     */
    static QString newServicePath(bool showInMenu, const QString &suggestedName, QString *menuId = nullptr, const QStringList *reservedMenuIds = nullptr);

    /*!
     * A desktop file name that this application is an alias for.
     *
     * This is used when a \c NoDisplay application is used to enforce specific handling
     * for an application. In that case the \c NoDisplay application is an \c AliasFor another
     * application and be considered roughly equal to the \c AliasFor application (which should
     * not be \c NoDisplay=true)
     *
     * For example Okular supplies a desktop file for each supported format (e.g. PDF), all
     * of which \c NoDisplay and it is merely there to selectively support specific file formats.
     * A UI may choose to display the aliased entry org.kde.okular instead of the NoDisplay entries.
     *
     * Returns QString desktopName of the aliased application (excluding .desktop suffix)
     *
     * \since 5.96
     */
    QString aliasFor() const;

    /*!
     * Returns the value of StartupNotify for this service.
     *
     * If the service doesn't define a value nullopt is returned.
     *
     * See StartupNotify in the \l {https://specifications.freedesktop.org/desktop-entry-spec/latest/} {Desktop Entry Specification}.
     *
     * \since 6.0
     */
    std::optional<bool> startupNotify() const;

private:
    friend class KBuildServiceFactory;

    QVariant property(const QString &_name, QMetaType::Type t) const;

    void setActions(const QList<KServiceAction> &actions);

    Q_DECLARE_PRIVATE(KService)

    friend class KServiceFactory;

    /*!
     * \internal
     * Construct a service from a stream.
     * The stream must already be positioned at the correct offset.
     */
    KSERVICE_NO_EXPORT KService(QDataStream &str, int offset);
};

template<>
KSERVICE_EXPORT QString KService::property<QString>(const QString &name) const;

#endif
