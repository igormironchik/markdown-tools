/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2007 Daniel M. Duley <daniel.duley@verizon.net>

    with minor additions and based on ideas from
    SPDX-FileContributor: Torsten Rahn <torsten@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONEFFECT_H
#define KICONEFFECT_H

#include <kiconthemes_export.h>

#include <QColor>
#include <QImage>
#include <QPixmap>

#include <memory>

class KIconEffectPrivate;

/*!
 * \class KIconEffect
 * \inmodule KIconThemes
 *
 * \brief Applies effects to icons.
 *
 * This class applies effects to icons depending on their state and
 * group. For example, it can be used to make all disabled icons
 * in a toolbar gray.
 *
 * \image kiconeffect-apply.png "Various Effects applied to an image"
 *
 * \sa QIcon::fromTheme
 */
class KICONTHEMES_EXPORT KIconEffect
{
public:
#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Create a new KIconEffect.
     * You will most likely never have to use this to create a new KIconEffect
     * yourself, as you can use the KIconEffect provided by the global KIconLoader
     * (which itself is accessible by KIconLoader::global()) through its
     * iconEffect() function.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    KIconEffect();
#endif
    ~KIconEffect();

    KIconEffect(const KIconEffect &) = delete;
    KIconEffect &operator=(const KIconEffect &) = delete;

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * This is the enumeration of all possible icon effects.
     *
     * \value NoEffect Do not apply any icon effect
     * \value ToGray Tints the icon gray
     * \value Colorize Tints the icon with a specific color
     * \value ToGamma Change the gamma value of the icon
     * \value DeSaturate Reduce the saturation of the icon
     * \value ToMonochrome Produces a monochrome icon
     * \omitvalue LastEffect
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    enum Effects {
        NoEffect,
        ToGray,
        Colorize,
        ToGamma,
        DeSaturate,
        ToMonochrome,
        LastEffect,
    };
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Rereads configuration.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    void init();
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Tests whether an effect has been configured for the given icon group.
     *
     * \a group the group to check, see KIconLoader::Group
     *
     * \a state the state to check, see KIconLoader::States
     *
     * Returns true if an effect is configured for the given \a group
     * in \a state, otherwise false.
     *
     * \sa KIconLoader::Group, KIconLoader::States
     *
     * \deprecated [6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    bool hasEffect(int group, int state) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Returns a fingerprint for the effect by encoding
     * the given \a group and \a state into a QString.
     *
     * This is useful for caching.
     *
     * \a group the group, see KIconLoader::Group
     *
     * \a state the state, see KIconLoader::States
     *
     * Returns the fingerprint of the given \a group + \a state
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QString fingerprint(int group, int state) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Applies an effect to an image. The effect to apply depends on the
     * \a group and \a state parameters, and is configured by the user.
     *
     * \a src The image.
     *
     * \a group The group for the icon, see KIconLoader::Group
     *
     * \a state The icon's state, see KIconLoader::States
     *
     * Returns An image with the effect applied.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QImage apply(const QImage &src, int group, int state) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Applies an effect to an image.
     *
     * \a src The image.
     *
     * \a effect The effect to apply, one of KIconEffect::Effects.
     *
     * \a value Strength of the effect. 0 <= \a value <= 1.
     *
     * \a rgb Color parameter for effects that need one.
     *
     * \a trans Add Transparency if trans = true.
     *
     * Returns An image with the effect applied.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QImage apply(const QImage &src, int effect, float value, const QColor &rgb, bool trans) const;

    /*!
     * Applies an effect to an image.
     *
     * \a src The image.
     *
     * \a effect The effect to apply, one of KIconEffect::Effects.
     *
     * \a value Strength of the effect. 0 <= \a value <= 1.
     *
     * \a rgb Color parameter for effects that need one.
     *
     * \a trans Add Transparency if trans = true.
     *
     * Returns An image with the effect applied.
     *
     * \deprecated[6.5] use the static API
     */
    QImage apply(const QImage &src, int effect, float value, const QColor &rgb, const QColor &rgb2, bool trans) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Applies an effect to a pixmap.
     *
     * \a src The pixmap.
     *
     * \a group The group for the icon, see KIconLoader::Group
     *
     * \a state The icon's state, see KIconLoader::States
     *
     * Returns A pixmap with the effect applied.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QPixmap apply(const QPixmap &src, int group, int state) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Applies an effect to a pixmap.
     *
     * \a src The pixmap.
     *
     * \a effect The effect to apply, one of KIconEffect::Effects.
     *
     * \a value Strength of the effect. 0 <= \a value <= 1.
     *
     * \a rgb Color parameter for effects that need one.
     *
     * \a trans Add Transparency if trans = true.
     *
     * Returns A pixmap with the effect applied.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QPixmap apply(const QPixmap &src, int effect, float value, const QColor &rgb, bool trans) const;

    /*!
     * Applies an effect to a pixmap.
     *
     * \a src The pixmap.
     *
     * \a effect The effect to apply, one of KIconEffect::Effects.
     *
     * \a value Strength of the effect. 0 <= \a value <= 1.
     *
     * \a rgb Color parameter for effects that need one.
     *
     * \a trans Add Transparency if trans = true.
     *
     * Returns A pixmap with the effect applied.
     *
     * \deprecated[6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QPixmap apply(const QPixmap &src, int effect, float value, const QColor &rgb, const QColor &rgb2, bool trans) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /*!
     * Returns an image twice as large, consisting of 2x2 pixels.
     *
     * \a src the image.
     *
     * Returns the scaled image.
     *
     * \deprecated [6.5] use the static API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static API")
    QImage doublePixels(const QImage &src) const;
#endif

    /*!
     * Tints an image gray.
     *
     * \a image The image
     *
     * \a value Strength of the effect. 0 <= \a value <= 1
     */
    static void toGray(QImage &image, float value);

    /*!
     * Colorizes an image with a specific color.
     *
     * \a image The image
     *
     * \a col The color with which the \a image is tinted
     *
     * \a value Strength of the effect. 0 <= \a value <= 1
     */
    static void colorize(QImage &image, const QColor &col, float value);

    /*!
     * Produces a monochrome icon with a given foreground and background color
     *
     * \a image The image
     *
     * \a white The color with which the white parts of \a image are painted
     *
     * \a black The color with which the black parts of \a image are painted
     *
     * \a value Strength of the effect. 0 <= \a value <= 1
     */
    static void toMonochrome(QImage &image, const QColor &black, const QColor &white, float value);

    /*!
     * Desaturates an image.
     *
     * \a image The image
     *
     * \a value Strength of the effect. 0 <= \a value <= 1
     */
    static void deSaturate(QImage &image, float value);

    /*!
     * Changes the gamma value of an image.
     *
     * \a image The image
     *
     * \a value Strength of the effect. 0 <= \a value <= 1
     */
    static void toGamma(QImage &image, float value);

    /*!
     * Renders an image semi-transparent.
     *
     * \a image The image
     */
    static void semiTransparent(QImage &image);

    /*!
     * Renders a pixmap semi-transparent.
     *
     * \a pixmap The pixmap
     */
    static void semiTransparent(QPixmap &pixmap);

    /*!
     * Overlays an image with an other image.
     *
     * \a src The image
     *
     * \a overlay The image to overlay \a src with
     */
    static void overlay(QImage &src, QImage &overlay);

    /*!
     * Applies a disabled effect
     *
     * \a image The image
     *
     * \since 6.5
     */
    static void toDisabled(QImage &image);

    /*!
     * Applies a disabled effect
     *
     * \a pixmap The image
     *
     * \since 6.5
     */
    static void toDisabled(QPixmap &pixmap);

    /*!
     * Applies an effect for an icon that is in an 'active' state
     *
     * \a image The image
     *
     * \since 6.5
     */
    static void toActive(QImage &image);

    /*!
     * Applies an effect for an icon that is in an 'active' state
     *
     * \a pixmap The image
     *
     * \since 6.5
     */
    static void toActive(QPixmap &pixmap);

private:
    std::unique_ptr<KIconEffectPrivate> const d;
};

#endif
