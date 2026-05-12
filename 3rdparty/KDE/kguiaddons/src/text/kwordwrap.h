/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kwordwrap_h
#define kwordwrap_h

#include <kguiaddons_export.h>

#include <QSharedDataPointer>
#include <qnamespace.h>

class QFontMetrics;
class QRect;
class QString;
class QPainter;
class KWordWrapPrivate;

/*!
 * \class KWordWrap
 * \inmodule KGuiAddons
 * \brief Word-wrap algorithm that takes into account beautifulness.
 *
 * That means:
 * \list
 * \li not letting a letter alone on the last line,
 * \li breaking at punctuation signs (not only at spaces)
 * \li improved handling of (), [] and {}
 * \li improved handling of '/' (e.g. for paths)
 * \endlist
 *
 * Usage: call the static method, formatText, with the text to
 * wrap and the constraining rectangle etc., it will return an instance of KWordWrap
 * containing internal data, result of the word-wrapping.
 * From that instance you can retrieve the boundingRect, and invoke drawing.
 *
 * This design allows to call the word-wrap algorithm only when the text changes
 * and not every time we want to know the bounding rect or draw the text. *
 */
class KGUIADDONS_EXPORT KWordWrap
{
public:
    /*!
     * Use this flag in drawText() if you want to fade out the text if it does
     * not fit into the constraining rectangle.
     *
     * \value FadeOut
     * \value Truncate
     */
    enum {
        FadeOut = 0x10000000,
        Truncate = 0x20000000,
    };

    /*!
     * Main method for wrapping text.
     *
     * \a fm Font metrics, for the chosen font. Better cache it, creating a QFontMetrics is expensive.
     *
     * \a r Constraining rectangle. Only the width and height matter. With
     *          negative height the complete text will be rendered
     *
     * \a flags currently unused
     *
     * \a str The text to be wrapped.
     *
     * \a len Length of text to wrap (default is -1 for all).
     *
     * Returns a KWordWrap instance. The caller is responsible for storing and deleting the result.
     */
    static KWordWrap formatText(QFontMetrics &fm, const QRect &r, int flags, const QString &str, int len = -1);

    /*!
     * Returns the bounding rect, calculated by formatText. The width is the
     *         width of the widest text line, and never wider than
     *         the rectangle given to formatText. The height is the
     *         text block. X and Y are always 0.
     */
    QRect boundingRect() const;

    /*!
     * Returns the original string, with '\n' inserted where
     * the text is broken by the wordwrap algorithm.
     */
    QString wrappedString() const; // gift for Dirk :)

    /*!
     * Returns the original string, truncated to the first line.
     * If \a dots was set, '...' is appended in case the string was truncated.
     * Bug: Note that the '...' come out of the bounding rect.
     */
    QString truncatedString(bool dots = true) const;

    /*!
     * Draw the text that has been previously wrapped, at position x,y.
     * Flags are for alignment, e.g. Qt::AlignHCenter. Default is
     * Qt::AlignAuto.
     *
     * \a painter the QPainter to use.
     *
     * \a x the horizontal position of the text
     *
     * \a y the vertical position of the text
     *
     * \a flags the ORed text alignment flags from the Qt namespace,
     *              ORed with FadeOut if you want the text to fade out if it
     *              does not fit (the \a painter's background must be set
     *              accordingly)
     */
    void drawText(QPainter *painter, int x, int y, int flags = Qt::AlignLeft) const;

    ~KWordWrap();

    KWordWrap(const KWordWrap &other);

    KWordWrap &operator=(const KWordWrap &other);

    /*!
     * Draws the string \a t at the given coordinates, if it does not
     * fit into @p maxW the text will be faded out.
     *
     * \a p the painter to use. Must have set the pen for the text
     *        color and the background for the color to fade out
     *
     * \a x the horizontal position of the text
     *
     * \a y the vertical position of the text
     *
     * \a maxW the maximum width of the text (including the fade-out
     *             effect)
     *
     * \a t the text to draw
     */
    static void drawFadeoutText(QPainter *p, int x, int y, int maxW, const QString &t);

    /*!
     * Draws the string \a t at the given coordinates, if it does not
     * fit into \a maxW the text will be truncated.
     *
     * \a p the painter to use
     *
     * \a x the horizontal position of the text
     *
     * \a y the vertical position of the text
     *
     * \a maxW the maximum width of the text (including the '...')
     *
     * \a t the text to draw
     */
    static void drawTruncateText(QPainter *p, int x, int y, int maxW, const QString &t);

private:
    KGUIADDONS_NO_EXPORT explicit KWordWrap(const QRect &r);

    QExplicitlySharedDataPointer<KWordWrapPrivate> d;
};

#endif
