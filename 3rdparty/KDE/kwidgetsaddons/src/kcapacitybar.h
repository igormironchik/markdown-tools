/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCAPACITYBAR_H
#define KCAPACITYBAR_H

#include <QStyle>
#include <QWidget>
#include <memory>

#include <kwidgetsaddons_export.h>

class QPaintEvent;

/*!
 * \class KCapacityBar
 * \inmodule KWidgetsAddons
 *
 * \brief This widget shows a bar which is filled to show the level of usage of
 *        a certain device.
 *
 * This widget represents a bar which goal is to show the level of usage of a
 * device. Its look is similar to a progress bar, but different, because this
 * widget does not want to give a notion of progress.
 *
 * \since 4.2
 *
 * \image kcapacitybar.png "KCapacityBar Widget"
 */
class KWIDGETSADDONS_EXPORT KCapacityBar : public QWidget
{
    Q_OBJECT

    /*!
     * \property KCapacityBar::value
     */
    Q_PROPERTY(int value READ value WRITE setValue)

    /*!
     * \property KCapacityBar::text
     */
    Q_PROPERTY(QString text READ text WRITE setText)

    /*!
     * \property KCapacityBar::drawTextMode
     */
    Q_PROPERTY(DrawTextMode drawTextMode READ drawTextMode WRITE setDrawTextMode)

    /*!
     * \property KCapacityBar::fillFullBlocks
     */
    Q_PROPERTY(bool fillFullBlocks READ fillFullBlocks WRITE setFillFullBlocks)

    /*!
     * \property KCapacityBar::continuous
     */
    Q_PROPERTY(bool continuous READ continuous WRITE setContinuous)

    /*!
     * \property KCapacityBar::barHeight
     */
    Q_PROPERTY(int barHeight READ barHeight WRITE setBarHeight)

    /*!
     * \property KCapacityBar::horizontalTextAlignment
     */
    Q_PROPERTY(Qt::Alignment horizontalTextAlignment READ horizontalTextAlignment WRITE setHorizontalTextAlignment)

public:
    /*!
     * \value DrawTextInline If any text set, draw it into the capacity bar
     * \value DrawTextOutline If any text set, draw it out of the capacity bar
     */
    enum DrawTextMode {
        DrawTextInline = 0,
        DrawTextOutline,
    };
    Q_ENUM(DrawTextMode)

    /*!
     * Constructs a capacity bar with DrawTextOutline as draw text mode.
     *
     * \a parent The parent of the widget.
     * \since 5.24
     */
    explicit KCapacityBar(QWidget *parent = nullptr);

    /*!
     * Capacity bar constructor.
     *
     * \a drawTextMode If any text set, whether to draw it into the capacity bar
     *                     or not.
     *
     * \a parent The parent of the widget.
     */
    explicit KCapacityBar(DrawTextMode drawTextMode, QWidget *parent = nullptr);
    ~KCapacityBar() override;

    /*!
     * Capacity bar fill value.
     *
     * \a value This parameter can take values from 0 to 100.
     *
     * Its value is 0 by default.
     */
    void setValue(int value);

    /*!
     * Returns the fill value of the capacity bar.
     */
    int value() const;

    /*!
     * Sets the text for the capacity bar.
     *
     * \a text The text that the capacity bar will show.
     *
     * This is an empty string by default.
     */
    void setText(const QString &text);

    /*!
     * Returns the text that the capacity bar will show.
     */
    QString text() const;

    /*!
     * When the capacity bar is non-continuous, sets whether the last block
     * shown should be drawn full or can be cut off (depending on the capacity
     * bar width, and the value set on it).
     *
     * \a fillFullBlocks If true, the last block drawn will be fully filled,
     *                       on other case, the last block drawn could be cut off.
     *
     * \note This method is only relevant if the capacity bar is in
     *       non-continuous mode.
     *
     * Its value is true by default.
     *
     * \sa setContinuous, continuous
     */
    void setFillFullBlocks(bool fillFullBlocks);

    /*!
     * Returns whether the last block shown can be cut off when necessary.
     */
    bool fillFullBlocks() const;

    /*!
     * Sets whether the fill of the capacity bar should be continuous or in
     * block mode.
     *
     * \a continuous If true, the fill of the capacity bar is done in a
     *                   continuous way. In other case, the fill is done with
     *                   separated blocks.
     *
     * \note Its value is true by default.
     */
    void setContinuous(bool continuous);

    /*!
     * Returns whether the fill of the capacity bar should be continuous or
     *         block-based.
     */
    bool continuous() const;

    /*!
     * Sets the height (in pixels) of the bar.
     *
     * \a barHeight The preferred height (in pixels) of the capacity bar.
     *
     * \note If you set a certain text and the capacity bar is in inline mode,
     *       the height of the bar will be the maximum of the font height and
     *       this value.
     *
     * \note If you set a certain text and the capacity bar is in outline mode,
     *       the height of the whole capacity bar will be bigger than this
     *       value. Take in count the height of this widget is got from adding
     *       the bar height, the font metrics height and a small separator
     *       between the bar and the outline text.
     *
     * \note Its value is 12 pixels by default.
     */
    void setBarHeight(int barHeight);

    /*!
     * Returns the preferred height of the capacity bar.
     */
    int barHeight() const;

    /*!
     * If the capacity bar is in outline text mode, draw the text with
     * \a textAlignment alignment.
     *
     * \a textAlignment Sets the horizontal alignment for the text if
     *                         the capacity bar is in outline text mode.
     *
     * \note If \a textAlignemt contains vertical alignment flags, they will be
     *       ignored.
     *
     * \note If the capacity bar is in inline text mode, the text is always
     *       centered, and both vertical and horizontal flags set through this
     *       method are ignored.
     *
     * \note Its value is centered by default.
     */
    void setHorizontalTextAlignment(Qt::Alignment textAlignment);

    /*!
     * Returns the horizontal alignment for the text that will be drawn.
     */
    Qt::Alignment horizontalTextAlignment() const;

    /*!
     * Set the way text is drawn if any is set
     *
     * \a mode If any text set, whether to draw it into the capacity bar
     *                     or not.
     */
    void setDrawTextMode(DrawTextMode mode);

    /*!
     * The way text is drawn, inside the capacity bar or outside of it
     */
    DrawTextMode drawTextMode() const;

    /*!
     * This method allows you to draw the widget, directly, for example on
     * item delegates. You only need the painter object and the rect where
     * this widget should be drawn.
     */
    void drawCapacityBar(QPainter *p, const QRect &rect) const;

    /*!
     * This method allows you to draw the widget, directly, for example on
     * item delegates. You need the painter object and the rect where
     * this widget should be drawn, and the state that it should be painted in.
     *
     * For example, when in a selected delegate, setting the correct state
     * ensures that the bar keeps sufficient contrast to the selection color.
     *
     * \since 6.12
     */
    void drawCapacityBar(QPainter *p, const QRect &rect, QStyle::State state) const;

    // Reimplemented from QWidget
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

    void changeEvent(QEvent *event) override;

private:
    std::unique_ptr<class KCapacityBarPrivate> const d;
};

#endif
