
/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QScopedPointer>
#include <QWidget>

namespace MdShared
{

//
// Switch
//

class SwitchPrivate;

/*!
    Switch is a button with two states: on and off. It's
    usefull widget for switching state of something, for example
    to turn on or off some functionality of the application,
    i.e. in settings menu.

    Switch can has four states. Main states are two: AcceptedUncheck
    and AcceptedCheck. The difference between not accepted and accepted
    states is that that, for example, user can toggle button with a finger
    to the checked state but application needs some time to turn of the
    functionality conjugated with this button. And while this functionality
    will not be turned on and setState() will not be
    called button will be in the NotAcceptedCheck state. In this state button
    will be turned on but will not be highlighted, that tell the user that
    his action is in process. When application is done with turning on
    the conjugated functionality programmaticaly should be called setState()
    slot with AcceptedCheck as argument. And at this point Switch will
    highlight his checked state.
*/
class Switch : public QWidget
{
    Q_OBJECT

    /*!
        \property state

        \brief the state of the Switch button.

        Switch can has four states. Main states are two: AcceptedUncheck
        and AcceptedCheck. The difference between not accepted and accepted
        states is that that, for example, user can toggle button with a finger
        to the checked state but application needs some time to turn of the
        functionality conjugated with this button. And while this functionality
        will not be turned on and setState() will not be
        called button will be in the NotAcceptedCheck state. In this state button
        will be turned on but will not be highlighted, that tell the user that
        his action is in process. When application is done with turning on
        the conjugated functionality programmaticaly should be called setState()
        slot with AcceptedCheck as argument. And at this point Switch will
        highlight his checked state.
    */
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged USER true)
    /*!
        \property checked

        \brief is this button checked or not.

        For NotAcceptedCheck and AcceptedCheck
        this property will be true, and for NotAcceptedUncheck and
        AcceptedUncheck this property will be false.
    */
    Q_PROPERTY(bool checked READ isChecked NOTIFY toggled)
    /*!
        \property onText

        \brief text that displays when button is checked.

        By default button doesn't display any text in checked state.
        I.e. this property is empty.
    */
    Q_PROPERTY(QString onText READ onText WRITE setOnText)
    /*!
        \property offText

        \brief text that displays when button is unchecked.

        By default button doesn't display any text in unchecked state.
        I.e. this property is empty.
    */
    Q_PROPERTY(QString offText READ offText WRITE setOffText)
    /*!
        \property onColor

        \brief color used to highlight checked state of the button.

        By default onColor is QPalette::Highlight.
    */
    Q_PROPERTY(QColor onColor READ onColor WRITE setOnColor)

signals:
    /*!
        This signal emits when user switches state of the button.
        If state of the button toggled programmaticaly this signal
        emits too. This signal emits as for not accepted states as
        for accepted states. For NotAcceptedCheck and AcceptedCheck
        \a checked will be true, and for NotAcceptedUncheck and
        AcceptedUncheck \a checked will be false.
    */
    void toggled(bool checked);
    /*!
        This signal emits when user switches state of the button or
        state of the button toggled programmaticaly, or button accepted
        it's new state. \sa State

        \a state contains the button's new State.
    */
    void stateChanged(int state);

public:
    /*!
        State of the button.
    */
    enum State {
        //! Button is in not accepted unchecked state.
        NotAcceptedUncheck = 0,
        //! Button is in accepted unchecked state.
        AcceptedUncheck = 1,
        //! Button is in not accepted checked state.
        NotAcceptedCheck = 2,
        //! Button is in accepted checked state.
        AcceptedCheck = 3
    }; // enum State

    Q_ENUM(State)

public:
    Switch(QWidget *parent = 0);
    explicit Switch(State st,
                    QWidget *parent = 0);

    virtual ~Switch();

    //! \return State of the button.
    State state() const;

    //! \return Is button checked?
    bool isChecked() const;

    //! \return Text that displays in checked state.
    QString onText() const;
    //! Set text that will be displayed in checked state.
    void setOnText(const QString &text);

    //! \return Text that displays in unchecked state.
    QString offText() const;
    //! Set text that will be displayed in unchecked state.
    void setOffText(const QString &text);

    //! \return Color to highlight checked state.
    QColor onColor() const;
    //! Set color to highlight checked state.
    void setOnColor(const QColor &c);

    QSize sizeHint() const override;

public slots:
    /*!
        Set state of the button.
    */
    void setState(State st);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    friend class SwitchPrivate;

    Q_DISABLE_COPY(Switch)

    QScopedPointer<SwitchPrivate> d;
}; // class Switch

} /* namespace MdShared */
