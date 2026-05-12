/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONTHEME_H
#define KICONTHEME_H

#include <kiconthemes_export.h>

#include <QList>
#include <QString>
#include <QStringList>

#include <memory>

#include "kiconloader.h"

class QAction;

/*!
 * \class KIconTheme
 * \inmodule KIconThemes
 *
 * \brief Class to use/access icon themes in KDE.
 *
 * This class is used by the
 * iconloader but can be used by others too.
 * \warning You should not use this class externally. This class is exported because
 *          the KCM needs it.
 * \sa KIconLoader
 */
class KICONTHEMES_EXPORT KIconTheme
{
public:
    /*!
     * Load an icon theme by name.
     *
     * \a name the name of the theme (e.g. "hicolor" or "keramik")
     *
     * \a appName the name of the application. Can be null. This argument
     *        allows applications to have themed application icons.
     *
     * \a basePathHint optional hint where to search the app themes.
     *        This is appended at the end of the search paths
     */
    explicit KIconTheme(const QString &name, const QString &appName = QString(), const QString &basePathHint = QString());
    ~KIconTheme();

    KIconTheme(const KIconTheme &) = delete;
    KIconTheme &operator=(const KIconTheme &) = delete;

    /*!
     * The stylized name of the icon theme.
     *
     * Returns the (human-readable) name of the theme
     */
    QString name() const;

    /*!
     * The internal name of the icon theme (same as the name argument passed
     *  to the constructor).
     *
     * Returns the internal name of the theme
     */
    QString internalName() const;

    /*!
     * A description for the icon theme.
     *
     * Returns a human-readable description of the theme, QString()
     *         if there is none
     */
    QString description() const;

    /*!
     * Return the name of the "example" icon. This can be used to
     * present the theme to the user.
     *
     * Returns the name of the example icon, QString() if there is none
     */
    QString example() const;

    /*!
     * Returns the name of the screenshot, QString() if there is none
     */
    QString screenshot() const;

    /*!
     * Returns the toplevel theme directory.
     */
    QString dir() const;

    /*!
     * The themes this icon theme falls back on.
     *
     * Returns a list of icon themes that are used as fall-backs
     */
    QStringList inherits() const;

    /*!
     * The icon theme exists?
     *
     * Returns true if the icon theme is valid
     */
    bool isValid() const;

    /*!
     * The icon theme should be hidden to the user?
     *
     * Returns true if the icon theme is hidden
     */
    bool isHidden() const;

    /*!
     * The minimum display depth required for this theme. This can either
     * be 8 or 32.
     *
     * Returns the minimum bpp (8 or 32)
     */
    int depth() const;

    /*!
     * The default size of this theme for a certain icon group.
     *
     * \a group The icon group. See KIconLoader::Group.
     *
     * Returns The default size in pixels for the given icon group.
     */
    int defaultSize(KIconLoader::Group group) const;

    /*!
     * Query available sizes for a group.
     *
     * \a group The icon group. See KIconLoader::Group.
     *
     * Returns a list of available sizes for the given group
     */
    QList<int> querySizes(KIconLoader::Group group) const;

    /*!
     * Query all available icons.
     *
     * Returns the list of icon names
     *
     * \since 6.11
     */
    [[nodiscard]] QStringList queryIcons() const;

    /*!
     * Query available icons for a size and context.
     *
     * \a size the size of the icons
     *
     * \a context the context of the icons
     *
     * Returns the list of icon names
     */
    QStringList queryIcons(int size, KIconLoader::Context context = KIconLoader::Any) const;

    /*!
     * Query available icons for a context and preferred size.
     *
     * \a size the size of the icons
     *
     * \a context the context of the icons
     *
     * Returns the list of icon names
     */
    QStringList queryIconsByContext(int size, KIconLoader::Context context = KIconLoader::Any) const;

    /*!
     * Lookup an icon in the theme.
     *
     * \a name The name of the icon, with extension.
     *
     * \a size The desired size of the icon.
     *
     * \a match The matching mode. KIconLoader::MatchExact returns an icon
     * only if matches exactly. KIconLoader::MatchBest returns the best matching
     * icon.
     *
     * Returns An absolute path to the file of the icon if it's found, QString() otherwise.
     *
     * KIconLoader::isValid will return true, and false otherwise.
     */
    QString iconPath(const QString &name, int size, KIconLoader::MatchType match) const;

    // TODO KF6 merge iconPath() with and without "scale" and move that argument after "size"
    /*!
     * Lookup an icon in the theme.
     *
     * \a name The name of the icon, with extension.
     *
     * \a size The desired size of the icon.
     *
     * \a match The matching mode. KIconLoader::MatchExact returns an icon
     * only if matches exactly. KIconLoader::MatchBest returns the best matching
     * icon.
     *
     * \a scale The scale of the icon group.
     *
     * Returns An absolute path to the file of the icon if it's found, QString() otherwise.
     *
     * KIconLoader::isValid will return true, and false otherwise.
     * \since 5.48
     */
    QString iconPath(const QString &name, int size, KIconLoader::MatchType match, qreal scale) const;

    /*!
     * Lookup an icon in the theme.
     *
     * \a name The name of the icon, without extension.
     *
     * \a size The desired size of the icon.
     *
     * \a match The matching mode. KIconLoader::MatchExact returns an icon
     * only if matches exactly. KIconLoader::MatchBest returns the best matching
     * icon.
     *
     * Returns An absolute path to the file of the icon if it's found, QString() otherwise.
     *
     * KIconLoader::isValid will return true, and false otherwise.
     *
     * \since 5.22
     */
    QString iconPathByName(const QString &name, int size, KIconLoader::MatchType match) const;

    // TODO KF6 merge iconPathByName() with and without "scale" and move that argument after "size"
    /*!
     * Lookup an icon in the theme.
     *
     * \a name The name of the icon, without extension.
     *
     * \a size The desired size of the icon.
     *
     * \a match The matching mode. KIconLoader::MatchExact returns an icon
     * only if matches exactly. KIconLoader::MatchBest returns the best matching
     * icon.
     *
     * \a scale The scale of the icon group.
     *
     * Returns An absolute path to the file of the icon if it's found, QString() otherwise.
     *
     * KIconLoader::isValid will return true, and false otherwise.
     *
     * \since 5.48
     */
    QString iconPathByName(const QString &name, int size, KIconLoader::MatchType match, qreal scale) const;

    /*!
     * Returns true if the theme has any icons for the given context.
     */
    bool hasContext(KIconLoader::Context context) const;

    /*!
     * If true, this theme is made of SVG icons that will be colorized following the system
     * color scheme. This is necessary for monochrome themes that should look visible on both
     * light and dark color schemes.
     *
     * Returns true if the SVG will be colorized with a stylesheet.
     * \since 5.22
     */
    bool followsColorScheme() const;

    /*!
     * List all icon themes installed on the system, global and local.
     *
     * Returns the list of all icon themes
     */
    static QStringList list();

    /*!
     * Returns the current icon theme.
     */
    static QString current();

    /*!
     * Force a current theme and disable automatic resolution of the current
     * theme in favor of the forced theme.
     *
     * To reset a forced theme, simply set an empty themeName.
     *
     * \a themeName name of the theme to force as current theme, or an
     *        empty string to unset theme forcing.
     *
     * \note This should only be used when a very precise expectation about
     *       the current theme is present. Most notably in unit tests
     *       this can be used to force a given theme.
     *
     * \warning A forced theme persists across reconfigure() calls!
     *
     * \sa current
     * \since 5.23
     */
    static void forceThemeForTests(const QString &themeName);

    /*!
     * Reconfigure the theme.
     */
    static void reconfigure();

    /*!
     * Returns the name of the default theme name
     */
    static QString defaultThemeName();

    /*!
     * Enforces the Breeze icon theme (including our KIconEngine for re-coloring).
     *
     * If the user has configured a different icon set, that one will be taken.
     *
     * (following the settings in the application configuration with fallback to kdeglobals)
     *
     * Must be called before the construction of the first Q(Core)Application, as it
     * might need to modify plugin loading and installs a startup function.
     *
     * If the application is already using the KDE platform theme, the icon set will not
     * be touched and the platform theme will ensure proper theming.
     *
     * Does setup the needed KColorSchemeManager to follow the system color mode starting
     * with version 6.6.
     *
     * \since 6.3
     */
    static void initTheme();

private:
    std::unique_ptr<class KIconThemePrivate> const d;
};

#endif
