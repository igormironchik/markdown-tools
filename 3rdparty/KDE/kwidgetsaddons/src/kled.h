/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Jörg Habenicht <j.habenicht@europemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLED_H
#define KLED_H

#include <kwidgetsaddons_export.h>

#include <QWidget>
#include <memory>

class QColor;

/*!
 * \class KLed
 * \inmodule KWidgetsAddons
 *
 * \brief An LED widget.
 *
 * Displays a round or rectangular light emitting diode.
 *
 * It is configurable to arbitrary colors, the two on/off states and three
 * styles (or "looks");
 *
 * It may display itself in a performant flat view, a round view with
 * light spot or a round view sunken in the screen.
 *
 * \image kled.png "KLed Widget"
 */
class KWIDGETSADDONS_EXPORT KLed : public QWidget
{
    Q_OBJECT

    /*!
     * \property KLed::state
     */
    Q_PROPERTY(State state READ state WRITE setState)

    /*!
     * \property KLed::shape
     */
    Q_PROPERTY(Shape shape READ shape WRITE setShape)

    /*!
     * \property KLed::look
     */
    Q_PROPERTY(Look look READ look WRITE setLook)

    /*!
     * \property KLed::color
     */
    Q_PROPERTY(QColor color READ color WRITE setColor)

    /*!
     * \property KLed::darkFactor
     */
    Q_PROPERTY(int darkFactor READ darkFactor WRITE setDarkFactor)

public:
    /*!
     * Status of the light is on/off.
     * \brief LED on/off.
     *
     * \value Off
     * \value On
     */
    enum State {
        Off,
        On
    };
    Q_ENUM(State)

    /*!
     * Shades of the lamp.
     * \brief LED shape
     *
     * \value Rectangular
     * \value Circular
     */
    enum Shape {
        Rectangular,
        Circular
    };
    Q_ENUM(Shape)

    /*!
     * Displays a flat, round or sunken LED.
     *
     * \brief LED look.
     *
     * \value Flat
     * \value Raised
     * \value Sunken
     */
    enum Look {
        Flat,
        Raised,
        Sunken,
    };
    Q_ENUM(Look)

    /*!
     * Constructs a green, round LED widget which will initially
     * be turned on.
     *
     * \a parent The parent widget.
     */
    explicit KLed(QWidget *parent = nullptr);

    /*!
     * Constructs a round LED widget with the supplied color which will
     * initially be turned on.
     *
     * \a color Initial color of the LED.
     *
     * \a parent The parent widget.
     */
    explicit KLed(const QColor &color, QWidget *parent = nullptr);

    /*!
     * Constructor with the color, state and look.
     *
     * Differs from above only in the parameters, which configure all settings.
     *
     * \a color  Initial color of the LED.
     *
     * \a state  Sets the State.
     *
     * \a look   Sets the Look.
     *
     * \a shape  Sets the Shape (rectangular or circular).
     *
     * \a parent The parent widget.
     */
    KLed(const QColor &color, KLed::State state, KLed::Look look, KLed::Shape shape, QWidget *parent = nullptr);

    ~KLed() override;

    /*!
     * Returns the current color of the widget.
     * \sa setColor()
     */
    QColor color() const;

    /*!
     * Returns the current state of the widget (on/off).
     *
     * \sa State
     */
    State state() const;

    /*!
     * Returns the current look of the widget.
     *
     * \sa Look
     */
    Look look() const;

    /*!
     * Returns the current shape of the widget.
     *
     * \sa Shape
     */
    Shape shape() const;

    /*!
     * Returns the factor to darken the LED.
     *
     * \sa setDarkFactor()
     */
    int darkFactor() const;

    /*!
     * Set the color of the widget.
     *
     * The LED is shown with \a color when in the KLed::On state
     * or with the darken color in KLed::Off state.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * \a color New color of the LED.
     *
     * \sa color() darkFactor()
     */
    void setColor(const QColor &color);

    /*!
     * Sets the state of the widget to On or Off.
     *
     * \a state The LED state: on or off.
     *
     * \sa on() off() toggle()
     */
    void setState(State state);

    /*!
     * Sets the look of the widget.
     *
     * The look may be Flat, Raised or Sunken.
     *
     * The widget calls the update() method, so it will
     * be updated when entering the main event loop.
     *
     * \a look New look of the LED.
     *
     * \sa Look
     */
    void setLook(Look look);

    /*!
     * Set the shape of the LED.
     *
     * \a shape The LED shape.
     * \brief Set LED shape.
     */
    void setShape(Shape shape);

    /*!
     * Sets the factor to darken the LED in KLed::Off state.
     *
     * The \a darkFactor should be greater than 100, otherwise the LED
     * becomes lighter in KLed::Off state.
     *
     * Defaults to 300.
     *
     * \a darkFactor Sets the factor to darken the LED.
     *
     * \sa setColor
     */
    void setDarkFactor(int darkFactor);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public Q_SLOTS:

    /*!
     * Toggles the state of the led from Off to On or vice versa.
     */
    void toggle();

    /*!
     * Sets the state of the widget to On.
     *
     * \sa off(), toggle(), setState()
     */
    void on();

    /*!
     * Sets the state of the widget to Off.
     *
     * \sa on(), toggle(), setState()
     */
    void off();

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    /*
     * invalidates caches after property changes and calls update()
     */
    KWIDGETSADDONS_NO_EXPORT void updateCachedPixmap();

    KWIDGETSADDONS_NO_EXPORT void updateAccessibleName();

private:
    std::unique_ptr<class KLedPrivate> const d;
};

#endif
