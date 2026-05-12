/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWINDOWEFFECTS_H
#define KWINDOWEFFECTS_H

#include "kwindowsystem_export.h"
#include <optional>

#include <QWidgetList> // for WId, etc.

#include <QColor>
#include <QRegion>

/*!
 * \namespace KWindowEffects
 * \inmodule KWindowSystem
 * \brief Namespace for common standardized window effects.
 */
namespace KWindowEffects
{
/*!
 * \value Slide
 * \value BlurBehind
 * \value BackgroundContrast
 */
enum Effect {
    Slide = 1,
    BlurBehind = 7,
    BackgroundContrast = 9,
};

/*!
 * \value NoEdge
 * \value TopEdge
 * \value RightEdge
 * \value BottomEdge
 * \value LeftEdge
 */
enum SlideFromLocation {
    NoEdge = 0,
    TopEdge,
    RightEdge,
    BottomEdge,
    LeftEdge,
};
/*!
 * Returns if an atom property is available
 *
 * \a effect the effect we want to check
 */
KWINDOWSYSTEM_EXPORT bool isEffectAvailable(Effect effect);

/*!
 * Instructs the window manager to blur the background
 * in the specified region behind the given window.
 * The given region will overwrite any previous blur-behind region.
 * Passing a null region will enable the blur effect for the whole window.
 * The region is relative to the top-left corner of the client area.
 *
 * If \a enable is \c false, blur will be disabled for the whole window
 * (\a region is ignored).
 *
 * Note that you will usually want to set the region to the shape of the window,
 * excluding any shadow or halo.
 *
 * \a window The window for which to enable the blur effect
 *
 * \a enable Enable the effect if \c true, disable it if \c false
 *
 * \a region The region within the window where the background will be blurred, specified in logical pixels
 *
 * \since 5.82
 */
KWINDOWSYSTEM_EXPORT void enableBlurBehind(QWindow *window, bool enable = true, const QRegion &region = QRegion());

/*!
 * Instructs the window manager to modify the color of the background
 * in the specified region behind the given window,
 * in order to improve the contrast and readability of any text
 * in the translucent window.
 * The given region will overwrite any previous backgroundcontrast region.
 * Passing a null region will enable the blur effect for the whole window.
 * The region is relative to the top-left corner of the client area.
 *
 * If \a enable is \c false, blur will be disabled for the whole window
 * (\a region is ignored).
 *
 * Note that you will usually want to set the region to the shape of the window,
 * excluding any shadow or halo.
 *
 * \a window The window for which to enable the background contrast effect
 *
 * \a enable Enable the effect if \c true, disable it if \c false
 *
 * \a brightness How to modify the area brightness: from 0 (make it black) to 2 (make it white), 1 leaves it unchanged
 *
 * \a region The region within the window where the background will be modified, specified in logical pixels
 *
 * \since 5.82
 */
KWINDOWSYSTEM_EXPORT void
enableBackgroundContrast(QWindow *window, bool enable = true, qreal contrast = 1, qreal intensity = 1, qreal saturation = 1, const QRegion &region = QRegion());

/*!
 * Mark a window as sliding from screen edge
 *
 * \a id of the window on which we want to apply the effect
 *
 * \a location edge of the screen from which we want the sliding effect.
 *               Desktop and Floating won't have effect.
 * \a offset distance in pixels from the screen edge defined by location
 *
 * \since 5.82
 */
KWINDOWSYSTEM_EXPORT void slideWindow(QWindow *window, SlideFromLocation location, int offset = -1);
}

#endif
