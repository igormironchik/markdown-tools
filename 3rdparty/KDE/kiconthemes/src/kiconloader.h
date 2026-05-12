/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONLOADER_H
#define KICONLOADER_H

#include <QObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <memory>

#if __has_include(<optional>) && __cplusplus >= 201703L
#include <optional>
#endif

#include <kiconthemes_export.h>

class QIcon;
class QMovie;
class QPixmap;

class KIconColors;
class KIconLoaderPrivate;
class KIconEffect;
class KIconTheme;

/*!
 * \class KIconLoader
 * \inmodule KIconThemes
 *
 * \brief Iconloader for KDE.
 *
 * \note For most icon loading use cases perfer using QIcon::fromTheme
 *
 * KIconLoader will load the current icon theme and all its base themes.
 * Icons will be searched in any of these themes. Additionally, it caches
 * icons and applies effects according to the user's preferences.
 *
 * In KDE, it is encouraged to load icons by "Group". An icon group is a
 * location on the screen where icons are being used. Standard groups are:
 * Desktop, Toolbar, MainToolbar, Small and Panel. Each group can have some
 * centrally-configured effects applied to its icons. This makes it possible
 * to offer a consistent icon look in all KDE applications.
 *
 * The standard groups are defined below.
 *
 * \list
 * \li KIconLoader::Desktop: Icons in the iconview of konqueror, kdesktop and similar apps.
 * \li KIconLoader::Toolbar: Icons in toolbars.
 * \li KIconLoader::MainToolbar: Icons in the main toolbars.
 * \li KIconLoader::Small: Various small (typical 16x16) places: titlebars, listviews
 * and menu entries.
 * \li KIconLoader::Panel: Icons in kicker's panel
 * \endlist
 *
 * The icons are stored on disk in an icon theme or in a standalone
 * directory. The icon theme directories contain multiple sizes and/or
 * depths for the same icon. The iconloader will load the correct one based
 * on the icon group and the current theme. Icon themes are stored globally
 * in share/icons, or, application specific in share/apps/$appdir/icons.
 *
 * The standalone directories contain just one version of an icon. The
 * directories that are searched are: $appdir/pics and $appdir/toolbar.
 * Icons in these directories can be loaded by using the special group
 * "User".
 */
class KICONTHEMES_EXPORT KIconLoader : public QObject
{
    Q_OBJECT

public:
    /*!
     * \enum KIconLoader::Context
     *
     * \brief Defines the context of the icon.
     *
     * \value Any Some icon with unknown purpose.
     * \value Action An action icon (e.g. 'save', 'print').
     * \value Application An icon that represents an application.
     * \value Device An icon that represents a device.
     * \value MimeType An icon that represents a mime type (or file type).
     * \value Animation An icon that is animated.
     * \value Category An icon that represents a category.
     * \value Emblem An icon that adds information to an existing icon.
     * \value Emote An icon that expresses an emotion.
     * \value International An icon that represents a country's flag.
     * \value Place An icon that represents a location (e.g. 'home', 'trash').
     * \value StatusIcon An icon that represents an event.
     */
    enum Context {
        Any,
        Action,
        Application,
        Device,
        MimeType,
        Animation,
        Category,
        Emblem,
        Emote,
        International,
        Place,
        StatusIcon,
    };
    Q_ENUM(Context)

    /*!
     * \enum KIconLoader::Type
     *
     * \brief The type of the icon.
     *
     * \value Fixed Fixed-size icon.
     * \value Scalable Scalable-size icon.
     * \value Threshold A threshold icon.
     */
    enum Type {
        Fixed,
        Scalable,
        Threshold,
    };
    Q_ENUM(Type)

    /*!
     * \enum KIconLoader::MatchType
     *
     * \brief The type of a match.
     *
     * \value MatchExact Only try to find an exact match.
     * \value MatchBest Take the best match if there is no exact match.
     * \value MatchBestOrGreaterSize Take the best match or the match with a greater size if there is no exact match. \since 6.0
     */
    enum MatchType {
        MatchExact,
        MatchBest,
        MatchBestOrGreaterSize,
    };
    Q_ENUM(MatchType)

    /*!
     * \enum KIconLoader::Group
     *
     * \brief The group of the icon.
     *
     * \value NoGroup No group
     * \value Desktop Desktop icons
     * \value FirstGroup First group
     * \value Toolbar Toolbar icons
     * \value MainToolbar Main toolbar icons
     * \value Small Small icons, e.g. for buttons
     * \value Panel Panel (Plasma Taskbar) icons
     * \value Dialog Icons for use in dialog titles, page lists, etc
     * \value LastGroup Last group
     * \value User User icons
     */
    enum Group {
        NoGroup = -1,
        Desktop = 0,
        FirstGroup = 0,
        Toolbar,
        MainToolbar,
        Small,
        // TODO KF6: remove this (See https://phabricator.kde.org/T14340)
        Panel,
        Dialog,
        LastGroup,
        User,
    };
    Q_ENUM(Group)

    /*!
     * \enum KIconLoader::StdSizes
     *
     * \brief These are the standard sizes for icons.
     *
     * \value SizeSmall small icons for menu entries
     * \value SizeSmallMedium slightly larger small icons for toolbars, panels, etc
     * \value SizeMedium medium sized icons for the desktop
     * \value SizeLarge large sized icons for the panel
     * \value SizeHuge huge sized icons for iconviews
     * \value SizeEnormous enormous sized icons for iconviews
     */
    enum StdSizes {
        SizeSmall = 16,
        SizeSmallMedium = 22,
        SizeMedium = 32,
        SizeLarge = 48,
        SizeHuge = 64,
        SizeEnormous = 128,
    };
    Q_ENUM(StdSizes)

    /*!
     * \enum KIconLoader::States
     *
     * \brief Defines the possible states of an icon.
     *
     * \value DefaultState The default state.
     * \value ActiveState Icon is active.
     * \value DisabledState Icon is disabled.
     * \value [since 5.22] SelectedState Icon is selected.
     * \value LastState Last state (last constant)
     */
    enum States {
        DefaultState,
        ActiveState,
        DisabledState,
        SelectedState,
        LastState,
    };
    Q_ENUM(States)

    /*!
     * Constructs an iconloader.
     *
     * \a appname Add the data directories of this application to the
     * icon search path for the "User" group. The default argument adds the
     * directories of the current application.
     *
     * \a extraSearchPaths additional search paths, either absolute or relative to GenericDataLocation
     *
     * Usually, you use the default iconloader, which can be accessed via
     * KIconLoader::global(), so you hardly ever have to create an
     * iconloader object yourself. That one is the application's iconloader.
     */
    explicit KIconLoader(const QString &appname = QString(), const QStringList &extraSearchPaths = QStringList(), QObject *parent = nullptr);

    ~KIconLoader() override;

    /*!
     * Returns the global icon loader initialized with the application name.
     */
    static KIconLoader *global();

    /*!
     * Adds \a appname to the list of application specific directories with \a themeBaseDir as its base directory.
     *
     * Assume the icons are in /home/user/app/icons/hicolor/48x48/my_app.png, the base directory would be
     * /home/user/app/icons; KIconLoader automatically searches \a themeBaseDir + "/hicolor"
     *
     * This directory must contain a dir structure as defined by the XDG icons specification
     *
     * \a appname The application name.
     *
     * \a themeBaseDir The base directory of the application's theme (eg. "/home/user/app/icons")
     */
    void addAppDir(const QString &appname, const QString &themeBaseDir = QString());

    /*!
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * \a canReturnNull parameter.
     *
     * \a name The name of the icon, without extension.
     *
     * \a group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     *
     * \a size If nonzero, this overrides the size specified by \a group.
     *             See KIconLoader::StdSizes.
     *
     * \a state The icon state: \c DefaultState, \c ActiveState or
     * \c DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     *
     * \a overlays a list of emblem icons to overlay, by name
     *                 \sa drawOverlays
     *
     * \a path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found \a path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     *
     * \a canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. Note: a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     *
     * Returns the QPixmap. Can be null when not found, depending on
     *         \a canReturnNull.
     */
    QPixmap loadIcon(const QString &name,
                     KIconLoader::Group group,
                     int size = 0,
                     int state = KIconLoader::DefaultState,
                     const QStringList &overlays = QStringList(),
                     QString *path_store = nullptr,
                     bool canReturnNull = false) const;

    // TODO KF6 merge loadIcon() and loadScaledIcon()
    /*!
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * \a canReturnNull parameter.
     *
     * \a name The name of the icon, without extension.
     *
     * \a group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     *
     * \a scale The scale of the icon group to use. If no icon exists in the
     * scaled group, a 1x icon with its size multiplied by the scale will be
     * loaded instead.
     *
     * \a size If nonzero, this overrides the size specified by \a group.
     *             See KIconLoader::StdSizes.
     *
     * \a state The icon state: \c DefaultState, \c ActiveState or
     * \c DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     *
     * \a overlays a list of emblem icons to overlay, by name
     *                 \sa drawOverlays
     *
     * \a path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found \a path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     *
     * \a canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. Note: a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     *
     * Returns the QPixmap. Can be null when not found, depending on
     *         \a canReturnNull.
     * \since 5.48
     */
    QPixmap loadScaledIcon(const QString &name,
                           KIconLoader::Group group,
                           qreal scale,
                           int size = 0,
                           int state = KIconLoader::DefaultState,
                           const QStringList &overlays = QStringList(),
                           QString *path_store = nullptr,
                           bool canReturnNull = false) const;

    /*!
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * \a canReturnNull parameter.
     *
     * \a name The name of the icon, without extension.
     *
     * \a group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     *
     * \a scale The scale of the icon group to use. If no icon exists in the
     * scaled group, a 1x icon with its size multiplied by the scale will be
     * loaded instead.
     *
     * \a size If nonzero, this overrides the size specified by \a group.
     *             See KIconLoader::StdSizes. The icon will be fit into \a size
     *             without changing the aspect ratio, which particularly matters
     *             for non-square icons.
     *
     * \a state The icon state: \c DefaultState, \c ActiveState or \c DisabledState. Depending on the user's preferences, the iconloader may apply a visual
     * effect to hint about its state.
     *
     * \a overlays a list of emblem icons to overlay, by name
     *                 \sa drawOverlays
     *
     * \a path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found \a path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     *
     * \a canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. Note: a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     *
     * Returns the QPixmap. Can be null when not found, depending on
     *         \a canReturnNull.
     * \since 5.81
     */
    QPixmap loadScaledIcon(const QString &name,
                           KIconLoader::Group group,
                           qreal scale,
                           const QSize &size = {},
                           int state = KIconLoader::DefaultState,
                           const QStringList &overlays = QStringList(),
                           QString *path_store = nullptr,
                           bool canReturnNull = false) const;

#if __has_include(<optional>) && __cplusplus >= 201703L
    /*!
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * \a canReturnNull parameter.
     *
     * \a name The name of the icon, without extension.
     *
     * \a group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     *
     * \a scale The scale of the icon group to use. If no icon exists in the
     * scaled group, a 1x icon with its size multiplied by the scale will be
     * loaded instead.
     *
     * \a size If nonzero, this overrides the size specified by \a group.
     *             See KIconLoader::StdSizes. The icon will be fit into \a size
     *             without changing the aspect ratio, which particularly matters
     *             for non-square icons.
     *
     * \a state The icon state: \c DefaultState, \c ActiveState or
     * \c DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     *
     * \a overlays a list of emblem icons to overlay, by name
     *                 \sa drawOverlays()
     *
     * \a path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found \a path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     * \a canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. Note: a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     *
     * \a colorScheme will define the stylesheet used to color this icon.
     *        Note this will only work if \a name is an svg file.
     *
     * Returns the QPixmap. Can be null when not found, depending on
     *         \a canReturnNull.
     * \since 5.88
     */
    QPixmap loadScaledIcon(const QString &name,
                           KIconLoader::Group group,
                           qreal scale,
                           const QSize &size,
                           int state,
                           const QStringList &overlays,
                           QString *path_store,
                           bool canReturnNull,
                           const std::optional<KIconColors> &colorScheme) const;
#endif

    /*!
     * Loads an icon for a mimetype.
     * This is basically like loadIcon except that extra desktop themes are loaded if necessary.
     *
     * Consider using QIcon::fromTheme() with a fallback to "application-octet-stream" instead.
     *
     * \a iconName The name of the icon, without extension, usually from KMimeType.
     *
     * \a group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     *
     * \a size If nonzero, this overrides the size specified by \a group.
     *             See KIconLoader::StdSizes.
     *
     * \a state The icon state: \c DefaultState, \c ActiveState or
     * \c DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     *
     * \a path_store If not null, the path of the icon is stored here.
     *
     * \a overlays a list of emblem icons to overlay, by name
     *                 \sa drawOverlays
     *
     * Returns the QPixmap. Can not be null, the
     * "unknown" pixmap is returned when no appropriate icon has been found.
     */
    QPixmap loadMimeTypeIcon(const QString &iconName,
                             KIconLoader::Group group,
                             int size = 0,
                             int state = KIconLoader::DefaultState,
                             const QStringList &overlays = QStringList(),
                             QString *path_store = nullptr) const;

    /*!
     * Returns the path of an icon.
     *
     * \a name The name of the icon, without extension. If an absolute
     * path is supplied for this parameter, iconPath will return it
     * directly.
     *
     * \a group_or_size If positive, search icons whose size is
     * specified by the icon group \a group_or_size. If negative, search
     * icons whose size is - \a group_or_size.
     *             See KIconLoader::Group and KIconLoader::StdSizes
     *
     * \a canReturnNull Can return a null string? If not, a path to the
     *                      "unknown" icon will be returned.
     *
     * Returns the path of an icon, can be null or the "unknown" icon when
     *         not found, depending on \a canReturnNull.
     */
    QString iconPath(const QString &name, int group_or_size, bool canReturnNull = false) const;

    // TODO KF6 merge iconPath() with and without "scale" and move that argument after "group_or_size"
    /*!
     * Returns the path of an icon.
     *
     * \a name The name of the icon, without extension. If an absolute
     * path is supplied for this parameter, iconPath will return it
     * directly.
     *
     * \a group_or_size If positive, search icons whose size is
     * specified by the icon group \a group_or_size. If negative, search
     * icons whose size is - \a group_or_size.
     *             See KIconLoader::Group and KIconLoader::StdSizes
     *
     * \a canReturnNull Can return a null string? If not, a path to the
     *                      "unknown" icon will be returned.
     *
     * \a scale The scale of the icon group.
     *
     * Returns the path of an icon, can be null or the "unknown" icon when
     *         not found, depending on \a canReturnNull.
     * \since 5.48
     */
    QString iconPath(const QString &name, int group_or_size, bool canReturnNull, qreal scale) const;

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Loads an animated icon.
     *
     * \a name The name of the icon.
     *
     * \a group The icon group. See loadIcon().
     *
     * \a size Override the default size for \a group.
     *             See KIconLoader::StdSizes.
     *
     * \a parent The parent object of the returned QMovie.
     *
     * Returns A QMovie object. Can be null if not found or not valid.
     *         Ownership is passed to the caller.
     * \deprecated[6.5] use QMovie API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use QMovie API")
    QMovie *loadMovie(const QString &name, KIconLoader::Group group, int size = 0, QObject *parent = nullptr) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Returns the path to an animated icon.
     *
     * \a name The name of the icon.
     *
     * \a group The icon group. See loadIcon().
     *
     * \a size Override the default size for \a group.
     *             See KIconLoader::StdSizes.
     *
     * Returns the full path to the movie, ready to be passed to QMovie's constructor.
     *
     * Empty string if not found.
     * \deprecated [6.5] use QMovie API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use QMovie API")
    QString moviePath(const QString &name, KIconLoader::Group group, int size = 0) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Loads an animated icon as a series of still frames. If you want to load
     * a .mng animation as QMovie instead, please use loadMovie() instead.
     *
     * \a name The name of the icon.
     *
     * \a group The icon group. See loadIcon().
     *
     * \a size Override the default size for \a group.
     *             See KIconLoader::StdSizes.
     *
     * Returns a QStringList containing the absolute path of all the frames
     * making up the animation.
     *
     * \deprecated [6.5] use QMovie API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use QMovie API")
    QStringList loadAnimated(const QString &name, KIconLoader::Group group, int size = 0) const;
#endif

    /*!
     * Queries all available icons.
     * \since 6.11
     */
    [[nodiscard]] QStringList queryIcons() const;

    /*!
     * Queries all available icons for a specific group, having a specific
     * context.
     *
     * \a group_or_size If positive, search icons whose size is
     * specified by the icon group \a group_or_size. If negative, search
     * icons whose size is - \a group_or_size.
     *             See KIconLoader::Group and KIconLoader::StdSizes
     *
     * \a context The icon context.
     * Returns a list of all icons
     */
    QStringList queryIcons(int group_or_size, KIconLoader::Context context = KIconLoader::Any) const;

    /*!
     * Queries all available icons for a specific context.
     *
     * \a group_or_size The icon preferred group or size. If available
     * at this group or size, those icons will be returned, in other case,
     * icons of undefined size will be returned. Positive numbers are groups,
     * negative numbers are negated sizes. See KIconLoader::Group and
     * KIconLoader::StdSizes
     *
     * \a context The icon context.
     *
     * Returns A QStringList containing the icon names
     * available for that context
     */
    QStringList queryIconsByContext(int group_or_size, KIconLoader::Context context = KIconLoader::Any) const;

    /*!
     * \internal
     */
    bool hasContext(KIconLoader::Context context) const;

    /*!
     * Returns a list of all icons (*.png or *.xpm extension) in the
     * given directory.
     *
     * \a iconsDir the directory to search in
     * Returns A QStringList containing the icon paths
     */
    QStringList queryIconsByDir(const QString &iconsDir) const;

    /*!
     * Returns all the search paths for this icon loader, either absolute or
     * relative to GenericDataLocation.
     *
     * Mostly internal (for KIconDialog).
     * \since 5.0
     */
    QStringList searchPaths() const;

    /*!
     * Returns the size of the specified icon group.
     *
     * Using e.g. KIconLoader::SmallIcon will return 16.
     *
     * \a group the group to check.
     *
     * Returns the current size for an icon group.
     */
    int currentSize(KIconLoader::Group group) const;

    /*!
     * Returns a pointer to the current theme.
     * Can be used to query
     * available and default sizes for groups.
     *
     * \note The KIconTheme will change if reconfigure() is called and
     * therefore it's not recommended to store the pointer anywhere.
     *
     * Returns a pointer to the current theme. 0 if no theme set.
     */
    KIconTheme *theme() const;

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Returns a pointer to the KIconEffect object used by the icon loader.
     * Returns the KIconEffect.
     *
     * \deprecated [6.5] use the static KIconEffect API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static KIconEffect API")
    KIconEffect *iconEffect() const;
#endif

    /*!
     * Reconfigure the icon loader, for instance to change the associated app name or extra search paths.
     *
     * This also clears the in-memory pixmap cache (even if the appname didn't change, which is useful for unittests)
     *
     * \a appname the application name (empty for the global iconloader)
     *
     * \a extraSearchPaths additional search paths, either absolute or relative to GenericDataLocation
     */
    void reconfigure(const QString &appname, const QStringList &extraSearchPaths = QStringList());

    /*!
     * Returns the unknown icon. An icon that is used when no other icon
     * can be found.
     */
    static QPixmap unknown();

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Draws overlays on the specified pixmap, it takes the width and height
     * of the pixmap into consideration
     *
     * \a overlays List of up to 4 overlays to blend over the pixmap. The first overlay
     *                 will be in the bottom right corner, followed by bottom left, top left
     *                 and top right. An empty QString can be used to leave the specific position
     *                 blank.
     *
     * \a pixmap to draw on
     * \since 4.7
     * \deprecated [6.5]
     * Use KIconUtils::addOverlays from KGuiAddons
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use KIconUtils::addOverlays from KGuiAddons")
    void drawOverlays(const QStringList &overlays, QPixmap &pixmap, KIconLoader::Group group, int state = KIconLoader::DefaultState) const;
#endif

    /*!
     *
     */
    bool hasIcon(const QString &iconName) const;

    /*!
     * Sets the colors for this KIconLoader.
     *
     * \note if you set a custom palette, if you are using some colors from
     * application's palette, you need to track the application palette changes by yourself.
     *
     * If you no longer wish to use a custom palette, use resetPalette()
     * \sa resetPalette()
     * \since 5.39
     */
    void setCustomPalette(const QPalette &palette);

    /*!
     * The colors that will be used for the svg stylesheet in case the
     * loaded icons are svg-based, icons can be colored in different ways in
     * different areas of the application
     *
     * Returns the palette, if any, an invalid one if the user didn't set it
     * \since 5.39
     */
    QPalette customPalette() const;

    /*!
     * Resets the custom palette used by the KIconLoader to use the
     * QGuiApplication::palette() again (and to follow it in case the
     * application's palette changes)
     * \since 5.39
     */
    void resetPalette();

    /*!
     * Returns whether we have set a custom palette using setCustomPalette
     *
     * \since 5.85
     * \sa resetPalette, setCustomPalette
     */
    bool hasCustomPalette() const;

public Q_SLOTS:
    // TODO KF7: while marked as deprecated, newIconLoader() is still used:
    // internally by KIconLoadeer as well as by Plasma's Icons kcm module (state: 5.17)
    // this needs some further cleanup work before removing it from the API with KICONTHEMES_ENABLE_DEPRECATED_SINCE
    /*!
     * Re-initialize the global icon loader
     *
     * \deprecated[5.0]
     * Use emitChange(Group)
     */
    KICONTHEMES_DEPRECATED_VERSION(5, 0, "Use KIconLoader::emitChange(Group)")
    void newIconLoader();

    /*!
     * Emits an iconChanged() signal on all the KIconLoader instances in the system
     * indicating that a system's icon group has changed in some way. It will also trigger
     * a reload in all of them to update to the new theme.
     *
     * \a group indicates the group that has changed
     *
     * \since 5.0
     */
    static void emitChange(Group group);

Q_SIGNALS:
    /*!
     * Emitted by newIconLoader once the new settings have been loaded
     */
    void iconLoaderSettingsChanged();

    /*!
     * Emitted when the system icon theme changes
     *
     * \since 5.0
     */
    void iconChanged(int group);

private:
    friend class KIconLoaderPrivate;
    std::unique_ptr<KIconLoaderPrivate> const d;

    Q_PRIVATE_SLOT(d, void _k_refreshIcons(int group))
};

/*!
 * \namespace KDE
 * \inmodule KIconThemes
 */
namespace KDE
{
/*!
 * \relates KIconLoader
 * Returns a QIcon with an appropriate
 * KIconEngine to perform loading and rendering.  KIcons thus adhere to
 * KDE style and effect standards.
 * \since 5.0
 */
KICONTHEMES_EXPORT QIcon icon(const QString &iconName, KIconLoader *iconLoader = nullptr);

/*!
 * \relates KIconLoader
 * Returns a QIcon with an appropriate
 * KIconEngine to perform loading and rendering.  KIcons thus adhere to
 * KDE style and effect standards.
 * \since 5.0
 */
KICONTHEMES_EXPORT QIcon icon(const QString &iconName, const KIconColors &colors, KIconLoader *iconLoader = nullptr);

/*!
 * \relates KIconLoader
 * Returns a QIcon for the given icon, with additional overlays.
 * \since 5.0
 */
KICONTHEMES_EXPORT QIcon icon(const QString &iconName, const QStringList &overlays, KIconLoader *iconLoader = nullptr);

}

inline KIconLoader::Group &operator++(KIconLoader::Group &group)
{
    group = static_cast<KIconLoader::Group>(group + 1);
    return group;
}
inline KIconLoader::Group operator++(KIconLoader::Group &group, int)
{
    KIconLoader::Group ret = group;
    ++group;
    return ret;
}

#endif // KICONLOADER_H
