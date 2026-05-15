/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEME_H
#define KCOLORSCHEME_H

#include "kcolorscheme_export.h"
#include <KSharedConfig>

#include <QExplicitlySharedDataPointer>

#include <QPalette>

class QColor;
class QBrush;

class KColorSchemePrivate;

/*!
 * \class KColorScheme
 * \inmodule KColorScheme
 * \brief A set of methods used to work with colors.
 *
 * KColorScheme currently provides access to the system color palette that the
 * user has selected (in the future, it is expected to do more).  It greatly
 * expands on QPalette by providing five distinct "sets" with several color
 * choices each, covering background, foreground, and decoration colors.
 *
 * A KColorScheme instance represents colors corresponding to a "set", where a
 * set consists of those colors used to draw a particular type of element, such
 * as a menu, button, view, selected text, or tooltip. Each set has a distinct
 * set of colors, so you should always use the correct set for drawing and
 * never assume that a particular foreground for one set is the same as the
 * foreground for any other set. Individual colors may be quickly referenced by
 * creating an anonymous instance and invoking a lookup member.
 *
 * \note
 * The color palettes for the various states of a widget (active, inactive,
 * disabled) may be wildly different.  Therefore, it is important to take the
 * state into account. This is why the KColorScheme constructor requires a
 * QPalette::ColorGroup as an argument.
 *
 * To facilitate working with potentially-varying states, two convenience API's
 * are provided. These are KColorScheme::adjustBackground and its sister
 * KColorScheme::adjustForeground, and the helper class KStatefulBrush.
 *
 * \sa KColorScheme::ColorSet, KColorScheme::ForegroundRole,
 * KColorScheme::BackgroundRole, KColorScheme::DecorationRole,
 * KColorScheme::ShadeRole
 */
class KCOLORSCHEME_EXPORT KColorScheme
{
public:
    /*!
     * \enum KColorScheme::ColorSet
     *
     * This enumeration describes the color set for which a color is being
     * selected.
     *
     * Color sets define a color "environment", suitable for drawing all parts
     * of a given region. Colors from different sets should not be combined.
     *
     * \value View Views; for example, frames, input fields, etc.\br\br
     *             If it contains things that can be selected, it is probably a \c View.
     * \value Window Non-editable window elements; for example, menus.\br\br
     *               If it isn't a \c Button, \c View, or \c Tooltip, it is probably a \c Window.
     * \value Button Buttons and button-like controls.\br\br
     *               In addition to buttons, "button-like" controls such as non-editable
     *               dropdowns, scrollbar sliders, slider handles, etc. should also use
     *               this role.
     * \value Selection Selected items in views.\br\br
     *                  Note that unfocused or disabled selections should use the \c Window
     *                  role. This makes it more obvious to the user that the view
     *                  containing the selection does not have input focus.
     * \value Tooltip Tooltips.\br\br
     *                The tooltip set can often be substituted for the view
     *                set when editing is not possible, but the Window set is deemed
     *                inappropriate. "What's This" help is an excellent example, another
     *                might be pop-up notifications (depending on taste).
     * \value [since KColorScheme 5.19] Complementary Complementary areas.\br\br
     *                                  Some applications want some areas to have a different color scheme.
     *                                  Usually dark areas over a light theme. For instance the fullscreen UI
     *                                  of a picture viewer, or the logout/lock screen of the plasma workspace
     *                                  ask for a dark color scheme even on light themes.
     * \value Header
     * \omitvalue NColorSets
     */
    enum ColorSet {
        View,
        Window,
        Button,
        Selection,
        Tooltip,
        Complementary,
        Header,
        NColorSets,
    };

    /*!
     * \enum KColorScheme::BackgroundRole
     *
     * This enumeration describes the background color being selected from the
     * given set.
     *
     * Background colors are suitable for drawing under text, and should never
     * be used to draw text. In combination with one of the overloads of
     * KColorScheme::shade, they may be used to generate colors for drawing
     * frames, bevels, and similar decorations.
     *
     * \value NormalBackground Normal background.
     * \value AlternateBackground Alternate background; for example, for use in lists.
     *                            This color may be the same as \c BackgroundNormal, especially in sets
     *                            other than \c View and \c Window.
     * \value ActiveBackground Third color; for example, items which are new, active, requesting
     *                         attention, etc.\br\br
     *                         Alerting the user that a certain field must be filled out would be a
     *                         good usage (although NegativeBackground could be used to the same
     *                         effect, depending on what you are trying to achieve). Unlike
     *                         \c ActiveText, this should not be used for mouseover effects.
     * \value LinkBackground Fourth color; corresponds to (unvisited) links.\br\br
     *                       Exactly what this might be used for is somewhat harder to qualify;
     *                       it might be used for bookmarks, as a 'you can click here' indicator,
     *                       or to highlight recent content (i.e. in a most-recently-accessed
     *                       list).
     * \value VisitedBackground Fifth color; corresponds to visited links.\br\br
     *                          This can also be used to indicate "not recent" content, especially
     *                          when a color is needed to denote content which is "old" or
     *                          "archival".
     * \value NegativeBackground Sixth color; for example, errors, untrusted content, etc.
     * \value NeutralBackground Seventh color; for example, warnings, secure/encrypted content.
     * \value PositiveBackground Eighth color; for example, success messages, trusted content.
     * \omitvalue NBackgroundRoles
     */
    enum BackgroundRole {
        NormalBackground,
        AlternateBackground,
        ActiveBackground,
        LinkBackground,
        VisitedBackground,
        NegativeBackground,
        NeutralBackground,
        PositiveBackground,
        NBackgroundRoles,
    };

    /*!
     * \enum KColorScheme::ForegroundRole
     *
     * This enumeration describes the foreground color being selected from the
     * given set.
     *
     * Foreground colors are suitable for drawing text or glyphs (such as the
     * symbols on window decoration buttons, assuming a suitable background
     * brush is used), and should never be used to draw backgrounds.
     *
     * For window decorations, the following is suggested, but not set in
     * stone:
     * \list
     *     \li Maximize - \c PositiveText
     *     \li Minimize - \c NeutralText
     *     \li Close - \c NegativeText
     *     \li WhatsThis - \c LinkText
     *     \li Sticky - \c ActiveText
     * \endlist
     *
     * \value NormalText Normal foreground.
     * \value InactiveText Second color; for example, comments, items which are old, inactive
     *                     or disabled.\br\br
     *                     Generally used for things that are meant to be "less
     *                     important". \c InactiveText is not the same role as NormalText in the
     *                     inactive state.
     * \value ActiveText Third color; for example items which are new, active, requesting
     *                   attention, etc.\br\br
     *                   May be used as a hover color for clickable items.
     * \value LinkText Fourth color; use for (unvisited) links.\br\br
     *                 May also be used for other
     *                 clickable items or content that indicates relationships, items that
     *                 indicate somewhere the user can visit, etc.
     * \value VisitedText Fifth color; used for (visited) links.\br\br
     *                    As with \c LinkText, may be used
     *                    for items that have already been "visited" or accessed. May also be
     *                    used to indicate "historical" (i.e. "old") items or information,
     *                    especially if \c InactiveText is being used in the same context to
     *                    express something different.
     * \value NegativeText Sixth color; for example, errors, untrusted content, deletions,
     *                     etc.
     * \value NeutralText Seventh color; for example, warnings, secure/encrypted content.
     * \value PositiveText Eighth color; for example, additions, success messages, trusted
     *                     content.
     * \omitvalue NForegroundRoles
     */
    enum ForegroundRole {
        NormalText,
        InactiveText,
        ActiveText,
        LinkText,
        VisitedText,
        NegativeText,
        NeutralText,
        PositiveText,
        NForegroundRoles,
    };

    /*!
     * \enum KColorScheme::DecorationRole
     *
     * This enumeration describes the decoration color being selected from the
     * given set.
     *
     * Decoration colors are used to draw decorations (such as frames) for
     * special purposes. Like color shades, they are neither foreground nor
     * background colors. Text should not be painted over a decoration color,
     * and decoration colors should not be used to draw text.
     *
     * \value FocusColor Color used to draw decorations for items which have input focus.
     * \value HoverColor Color used to draw decorations for items which will be activated by
     *                   clicking.
     * \omitvalue NDecorationRoles
     */
    enum DecorationRole {
        FocusColor,
        HoverColor,
        NDecorationRoles,
    };

    /*!
     * \enum KColorScheme::ShadeRole
     *
     * This enumeration describes the color shade being selected from the given
     * set.
     *
     * Color shades are used to draw "3d" elements, such as frames and bevels.
     * They are neither foreground nor background colors. Text should not be
     * painted over a shade, and shades should not be used to draw text.
     *
     * \value LightShade The light color is lighter than QPalette::dark() or QPalette::shadow() and contrasts
     *                   with the base color.
     * \value MidlightShade The midlight color is in between base() and QPalette::light().
     * \value MidShade The mid color is in between QPalette::base() and QPalette::dark().
     * \value DarkShade The dark color is in between QPalette::mid() and QPalette::shadow().
     * \value ShadowShade The shadow color is darker than QPalette::light() or QPalette::midlight() and contrasts
     *                    the base color.
     * \omitvalue NShadeRoles
     */
    enum ShadeRole {
        LightShade,
        MidlightShade,
        MidShade,
        DarkShade,
        ShadowShade,
        NShadeRoles,
    };

    /*! Destructor */
    virtual ~KColorScheme(); // TODO KF6: remove virtual

    KColorScheme(const KColorScheme &);
    KColorScheme &operator=(const KColorScheme &);
    KColorScheme(KColorScheme &&);
    KColorScheme &operator=(KColorScheme &&);

    /*!
     * Construct a palette from given color set and state. Colors are taken
     * from the given KConfig. If null, the application's color scheme is used
     *  (either the system default or one set by KColorSchemeManager).
     *
     * \note KColorScheme provides direct access to the color scheme for users
     * that deal directly with widget states. Unless you are a low-level user
     * or have a legitimate reason to only care about a fixed, limited number
     * of states (e.g. windows that cannot be inactive), consider using a
     * KStatefulBrush instead.
     */
    explicit KColorScheme(QPalette::ColorGroup = QPalette::Normal, ColorSet = View, KSharedConfigPtr = KSharedConfigPtr());

    /*!
     * Retrieve the requested background brush.
     */
    QBrush background(BackgroundRole = NormalBackground) const;

    /*!
     * Retrieve the requested foreground brush.
     */
    QBrush foreground(ForegroundRole = NormalText) const;

    /*!
     * Retrieve the requested decoration brush.
     */
    QBrush decoration(DecorationRole) const;

    /*!
     * Retrieve the requested shade color, using
     * KColorScheme::background(KColorScheme::NormalBackground)
     * as the base color and the contrast setting from the KConfig used to
     * create this KColorScheme instance.
     *
     * \note Shades are chosen such that all shades would contrast with the
     * base color. This means that if base is very dark, the 'dark' shades will
     * be lighter than the base color, with QPalette::midlight() == QPalette::shadow().
     * Conversely, if the base color is very light, the 'light' shades will be
     * darker than the base color, with QPalette::light() == QPalette::mid().
     */
    QColor shade(ShadeRole) const;

    /*!
     * Returns the contrast for borders as a floating point value.
     *
     * \a config pointer to the config from which to read the contrast
     * setting. If null, the application's color scheme will be used
     *   (either the system default or one set by KColorSchemeManager).
     *
     * Returns the contrast (between 0.0 for minimum and 1.0 for maximum
     *         contrast)
     */
    static qreal contrastF(const KSharedConfigPtr &config = KSharedConfigPtr());

    /*!
     * Retrieve the requested shade color, using the specified color as the
     * base color and the application's contrast setting.
     *
     * \note Shades are chosen such that all shades would contrast with the
     * base color. This means that if base is very dark, the 'dark' shades will
     * be lighter than the base color, with QPalette::midlight() == QPalette::shadow().
     * Conversely, if the base color is very light, the 'light' shades will be
     * darker than the base color, with QPalette::light() == QPalette::mid().
     */
    static QColor shade(const QColor &, ShadeRole);

    /*!
     * Retrieve the requested shade color, using the specified color as the
     * base color and the specified contrast.
     *
     * \a contrast Amount roughly specifying the contrast by which to
     * adjust the base color, between -1.0 and 1.0 (values between 0.0 and 1.0
     * correspond to the value from KColorScheme::contrastF)
     *
     * \a chromaAdjust (optional) Amount by which to adjust the chroma of
     * the shade (1.0 means no adjustment)
     *
     * \note Shades are chosen such that all shades would contrast with the
     * base color. This means that if base is very dark, the 'dark' shades will
     * be lighter than the base color, with QPalette::midlight() == QPalette::shadow().
     * Conversely, if the base color is very light, the 'light' shades will be
     * darker than the base color, with QPalette::light() == QPalette::mid().
     *
     * \sa KColorUtils::shade
     */
    static QColor shade(const QColor &, ShadeRole, qreal contrast, qreal chromaAdjust = 0.0);

    /*!
     * Adjust a QPalette by replacing the specified QPalette::ColorRole with
     * the requested background color for all states. Using this method is
     * safer than replacing individual states, as it insulates you against
     * changes in QPalette::ColorGroup.
     *
     * \note Although it is possible to replace a foreground color using this
     * method, it's bad usability to do so. Just say "no".
     */
    static void adjustBackground(QPalette &,
                                 BackgroundRole newRole = NormalBackground,
                                 QPalette::ColorRole color = QPalette::Base,
                                 ColorSet set = View,
                                 KSharedConfigPtr = KSharedConfigPtr());

    /*!
     * Adjust a QPalette by replacing the specified QPalette::ColorRole with
     * the requested foreground color for all states. Using this method is
     * safer than replacing individual states, as it insulates you against
     * changes in QPalette::ColorGroup.
     *
     * \note Although it is possible to replace a background color using this
     * method, it's bad usability to do so. Just say "no".
     */
    static void adjustForeground(QPalette &,
                                 ForegroundRole newRole = NormalText,
                                 QPalette::ColorRole color = QPalette::Text,
                                 ColorSet set = View,
                                 KSharedConfigPtr = KSharedConfigPtr());

    /*!
     * Used to obtain the QPalette that will be used to set the application
     * palette from KDE Platform theme.
     *
     * \a config KConfig from which to load the colors
     *
     * Returns the QPalette
     *
     * \since 5.0
     */
    static QPalette createApplicationPalette(const KSharedConfigPtr &config);

    /*!
     * Used to check if the color scheme has a given set.
     *
     * \a config KConfig from which to load the colors
     *
     * \a set The color set to check for.
     *
     * Returns whether the color scheme has a given color set
     *
     * \since 5.75
     */
    static bool isColorSetSupported(const KSharedConfigPtr &config, KColorScheme::ColorSet set);

    /*!
     * Customizable frameContrast value that will override the contrast
     * of frames in using styles (Breeze).
     *
     * \a config pointer to the config from which to read the contrast
     * setting. If null, the application's color scheme will be used
     *   (either the system default or one set by KColorSchemeManager).
     *
     * Returns the contrast (between 0.00 and 1.00)
     *
     * \since 6.20
     */
    static qreal frameContrast(const KSharedConfigPtr &config = KSharedConfigPtr());

    /*!
     * \since 5.92
     */
    bool operator==(const KColorScheme &other) const;

private:
    QExplicitlySharedDataPointer<KColorSchemePrivate> d;
};

Q_DECLARE_METATYPE(KColorScheme)

#endif // KCOLORSCHEME_H
