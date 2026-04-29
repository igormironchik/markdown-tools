/* -*- c++ -*- */
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Jörg Habenicht <j.habenicht@europemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRULER_H
#define KRULER_H

#include <kwidgetsaddons_export.h>

#include <QAbstractSlider>
#include <memory>

/*!
 * \class KRuler
 * \inmodule KWidgetsAddons
 *
 * \brief A ruler widget.
 *
 * The vertical ruler looks similar to this:
 *
 * \badcode
 *    meters                       inches
 *
 *    ------   <--- end mark  ---> ------
 *        --                            -
 *        --   <---little mark--->     --
 *        --                            -
 *        --                          ---
 *       ---   <---medium mark          -
 *        --                           --
 *        --        tiny mark---->      -
 *        --                         ----
 *        --                            -
 *      ----   <-----big mark          --
 *        --                            -
 *      |>--   <--ruler pointer-->   |>--
 *
 * \endcode
 *
 * There are tiny marks, little marks, medium marks, and big marks along the
 *  ruler.
 *
 * To receive mouse clicks or mouse moves, the class has to be overloaded.
 *
 * \image kruler.png "KRuler Widget"
 */
class KWIDGETSADDONS_EXPORT KRuler : public QAbstractSlider
{
    Q_OBJECT

    /*!
     * \property KRuler::showTinyMarks
     */
    Q_PROPERTY(bool showTinyMarks READ showTinyMarks WRITE setShowTinyMarks)

    /*!
     * \property KRuler::showLittleMarks
     */
    Q_PROPERTY(bool showLittleMarks READ showLittleMarks WRITE setShowLittleMarks)

    /*!
     * \property KRuler::showMediumMarks
     */
    Q_PROPERTY(bool showMediumMarks READ showMediumMarks WRITE setShowMediumMarks)

    /*!
     * \property KRuler::showBigMarks
     */
    Q_PROPERTY(bool showBigMarks READ showBigMarks WRITE setShowBigMarks)

    /*!
     * \property KRuler::showPointer
     */
    Q_PROPERTY(bool showPointer READ showPointer WRITE setShowPointer)

    /*!
     * \property KRuler::showEndLabel
     */
    Q_PROPERTY(bool showEndLabel READ showEndLabel WRITE setShowEndLabel)

    /*!
     * \property KRuler::tinyMarkDistance
     */
    Q_PROPERTY(int tinyMarkDistance READ tinyMarkDistance WRITE setTinyMarkDistance)

    /*!
     * \property KRuler::littleMarkDistance
     */
    Q_PROPERTY(int littleMarkDistance READ littleMarkDistance WRITE setLittleMarkDistance)

    /*!
     * \property KRuler::mediumMarkDistance
     */
    Q_PROPERTY(int mediumMarkDistance READ mediumMarkDistance WRITE setBigMarkDistance)

    /*!
     * \property KRuler::bigMarkDistance
     */
    Q_PROPERTY(int bigMarkDistance READ bigMarkDistance WRITE setBigMarkDistance)

    /*!
     * \property KRuler::pixelPerMark
     */
    Q_PROPERTY(double pixelPerMark READ pixelPerMark WRITE setPixelPerMark)

    /*!
     * \property KRuler::lengthFixed
     */
    Q_PROPERTY(bool lengthFixed READ lengthFixed WRITE setLengthFixed)

    /*!
     * \property KRuler::endLabel
     */
    Q_PROPERTY(QString endLabel READ endLabel WRITE setEndLabel)

    /*!
     * \property KRuler::length
     */
    Q_PROPERTY(int length READ length WRITE setLength)

    /*!
     * \property KRuler::offset
     */
    Q_PROPERTY(int offset READ offset)

    /*!
     * \property KRuler::endOffset
     */
    Q_PROPERTY(int endOffset READ endOffset)

public:
    /*!
     * The types of units used.
     *
     * \value Custom
     * \value Pixel
     * \value Inch
     * \value Millimetres
     * \value Centimetres
     * \value Metres
     */
    enum MetricStyle {
        Custom = 0,
        Pixel,
        Inch,
        Millimetres,
        Centimetres,
        Metres
    };
    Q_ENUM(MetricStyle)

    /*!
     * Constructs a horizontal ruler.
     */
    explicit KRuler(QWidget *parent = nullptr);

    /*!
     * Constructs a ruler with orientation \a orient.
     *
     * \a parent and \a f are passed to QFrame.
     * The default look is a raised widget
     * but may be changed with the inherited QFrame methods.
     *
     * \a orient Orientation of the ruler.
     *
     * \a parent Will be handed over to QFrame.
     *
     * \a f Will be handed over to QFrame.
     */
    explicit KRuler(Qt::Orientation orient, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /*!
     * Constructs a ruler with orientation \a orient and initial width \a widgetWidth.
     *
     * The width sets the fixed width of the widget. This is useful if you
     * want to draw the ruler bigger or smaller than the default size.
     *
     * \note The size of the marks doesn't change.
     *
     * \a parent and \a f are passed to QFrame.
     *
     * \a orient Orientation of the ruler.
     *
     * \a widgetWidth Fixed width of the widget.
     *
     * \a parent Will be handed over to QFrame.
     *
     * \a f Will be handed over to QFrame.
     *
     */
    KRuler(Qt::Orientation orient, int widgetWidth, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    ~KRuler() override;

    /*!
     * Sets the distance between tiny marks.
     *
     * This is mostly used in the English system (inches) with distance of 1.
     */
    void setTinyMarkDistance(int);

    /*!
     * Returns the distance between tiny marks.
     */
    int tinyMarkDistance() const;

    /*!
     * Sets the distance between little marks.
     *
     * The default value is 1 in the metric system and 2 in the English (inches) system.
     */
    void setLittleMarkDistance(int);

    /*!
     * Returns the distance between little marks.
     */
    int littleMarkDistance() const;

    /*!
     * Sets the distance between medium marks.
     *
     * For English (inches) styles it defaults to twice the little mark distance.
     * For metric styles it defaults to five times the little mark distance.
     */
    void setMediumMarkDistance(int);

    /*!
     *
     */
    int mediumMarkDistance() const;

    /*!
     * Sets distance between big marks.
     *
     * For English (inches) or metric styles it is twice the medium mark distance.
     */
    void setBigMarkDistance(int);
    /*!
     * Returns the distance between big marks.
     */
    int bigMarkDistance() const;

    /*!
     * Shows/hides tiny marks.
     */
    void setShowTinyMarks(bool);
    /*!
     *
     */
    bool showTinyMarks() const;
    /*!
     * Shows/hides little marks.
     */
    void setShowLittleMarks(bool);
    /*!
     *
     */
    bool showLittleMarks() const;
    /*!
     * Shows/hides medium marks.
     */
    void setShowMediumMarks(bool);
    /*!
     *
     */
    bool showMediumMarks() const;
    /*!
     * Shows/hides big marks.
     */
    void setShowBigMarks(bool);
    /*!
     *
     */
    bool showBigMarks() const;
    /*!
     * Shows/hides end marks.
     */
    void setShowEndMarks(bool);
    /*!
     *
     */
    bool showEndMarks() const;
    /*!
     * Shows/hides the pointer.
     */
    void setShowPointer(bool);
    /*!
     *
     */
    bool showPointer() const;

    /*!
     * Show/hide number values of the end marks.
     *
     * Default is \a false.
     */
    void setShowEndLabel(bool);
    /*!
     *
     */
    bool showEndLabel() const;

    /*!
     * Sets the label this is drawn at the beginning of the visible part
     * of the ruler to \a label
     */
    void setEndLabel(const QString &);
    /*!
     *
     */
    QString endLabel() const;

    /*!
     * Sets up the necessary tasks for the provided styles.
     *
     * A convenience method.
     */
    void setRulerMetricStyle(KRuler::MetricStyle);

    /*!
     * Sets the number of pixels between two base marks.
     *
     * Calling this method stretches or shrinks your ruler.
     *
     * For pixel display ( MetricStyle) the value is 10.0 marks
     * per pixel ;-)
     *
     * For English (inches) it is 9.0, and for centimetres ~2.835 -> 3.0 .
     * If you want to magnify your part of display, you have to
     * adjust the mark distance here.
     *
     * Notice: The double type is only supported to give the possibility
     *         of having some double values.
     *         It should be used with care.  Using values below 10.0
     *         shows visible jumps of markpositions (e.g. 2.345).
     *         Using whole numbers is highly recommended.
     *
     * To use int values use setPixelPerMark((int)your_int_value);
     *
     * default: 1 mark per 10 pixels
     */
    void setPixelPerMark(double rate);

    /*!
     * Returns the number of pixels between two base marks.
     */
    double pixelPerMark() const;

    /*!
     * Sets the length of the ruler, i.e. the difference between
     * the begin mark and the end mark of the ruler.
     *
     * Same as (width() - offset())
     *
     * when the length is not locked, it gets adjusted with the
     * length of the widget.
     */
    void setLength(int);

    /*!
     *
     */
    int length() const;

    /*!
     * Locks the length of the ruler, i.e. the difference between
     * the two end marks doesn't change when the widget is resized.
     *
     * \a fix fixes the length, if true
     */
    void setLengthFixed(bool fix);

    /*!
     *
     */
    bool lengthFixed() const;

    /*!
     * Sets the number of pixels by which the ruler may slide up or left.
     * The number of pixels moved is realive to the previous position.
     * The Method makes sense for updating a ruler, which is working with
     * a scrollbar.
     *
     * This doesn't affect the position of the ruler pointer.
     * Only the visible part of the ruler is moved.
     *
     * \a count Number of pixel moving up or left relative to the previous position
     */
    void slideUp(int count = 1);

    /*!
     * Sets the number of pixels by which the ruler may slide down or right.
     * The number of pixels moved is realive to the previous position.
     * The Method makes sense for updating a ruler, which is working with
     * a scrollbar.
     *
     * This doesn't affect the position of the ruler pointer.
     * Only the visible part of the ruler is moved.
     *
     * \a count Number of pixel moving up or left relative to the previous position
     */
    void slideDown(int count = 1);

    /*!
     * Sets the ruler slide offset.
     *
     * This is like slideup() or slidedown() with an absolute offset
     * from the start of the ruler.
     *
     * \a offset Number of pixel to move the ruler up or left from the beginning
     */
    void setOffset(int offset);

    /*!
     * Returns the current ruler offset.
     */
    int offset() const;

    /*!
     *
     */
    int endOffset() const;

public Q_SLOTS:

    /*!
     * Sets the pointer to a new position.
     *
     * The offset is NOT updated.
     * QWidget::repaint() is called afterwards.
     */
    void slotNewValue(int);

    /*!
     * Sets the ruler marks to a new position.
     *
     * The pointer is NOT updated.
     * QWidget::repaint() is called afterwards.
     */
    void slotNewOffset(int);

    /*!
     *
     */
    void slotEndOffset(int);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    KWIDGETSADDONS_NO_EXPORT void initWidget(Qt::Orientation orientation);

private:
    std::unique_ptr<class KRulerPrivate> const d;
};

#endif
