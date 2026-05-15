/*
    SPDX-FileCopyrightText: 2005, 2009 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KFONTUTILS_H
#define KFONTUTILS_H

#include <kguiaddons_export.h>

#include <qglobal.h>

class QPainter;
class QSizeF;
class QString;

/*!
 * \namespace KFontUtils
 * \inmodule KGuiAddons
 * \brief Provides utility functions for font data.
 */
namespace KFontUtils
{
/*!
 * Modifiers for the adaptFontSize function
 *
 * \value NoFlags No modifier
 * \value DoNotAllowWordWrap Do not use word wrapping
 *
 */
enum AdaptFontSizeOption {
    NoFlags = 0x01,
    DoNotAllowWordWrap = 0x02,
};

Q_DECLARE_FLAGS(AdaptFontSizeOptions, AdaptFontSizeOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(AdaptFontSizeOptions)

/*! Helper function that calculates the biggest font size (in points) used
    drawing a centered text using word wrapping.

    \a painter The painter where the text will be painted. The font set
                   in the painter is used for the calculation. Note the
                   painter font size is modified by this call

    \a text The text you want to draw

    \a width The available width for drawing

    \a height The available height for drawing

    \a maxFontSize The maximum font size (in points) to consider

    \a minFontSize The minimum font size (in points) to consider

    \a flags The modifiers for how the text is painted

    Returns the calculated biggest font size (in points) that draws the text
            in the given dimensions. Can return smaller than minFontSize,
            that means the text doesn't fit in the given rectangle. Can
            return -1 on error
    \since 4.7
*/
qreal KGUIADDONS_EXPORT adaptFontSize(QPainter &painter,
                                      const QString &text,
                                      qreal width,
                                      qreal height,
                                      qreal maxFontSize = 28.0,
                                      qreal minFontSize = 1.0,
                                      AdaptFontSizeOptions flags = NoFlags);

/*! Convenience function for adaptFontSize that accepts a QSizeF instead two qreals
    \since 4.7
*/
qreal KGUIADDONS_EXPORT adaptFontSize(QPainter &painter,
                                      const QString &text,
                                      const QSizeF &availableSize,
                                      qreal maxFontSize = 28.0,
                                      qreal minFontSize = 1.0,
                                      AdaptFontSizeOptions flags = NoFlags);
}

#endif
