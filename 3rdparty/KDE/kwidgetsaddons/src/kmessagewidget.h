/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KMESSAGEWIDGET_H
#define KMESSAGEWIDGET_H

#include <kwidgetsaddons_export.h>

#include <QFrame>
#include <memory>

/*!
 * \class KMessageWidget
 * \inmodule KWidgetsAddons
 *
 * \brief A widget to provide feedback or propose opportunistic interactions.
 *
 * KMessageWidget can be used to provide inline positive or negative
 * feedback, or to implement opportunistic interactions.
 *
 * As a feedback widget, KMessageWidget provides a less intrusive alternative
 * to "OK Only" message boxes. If you want to avoid a modal KMessageBox,
 * consider using KMessageWidget instead.
 *
 * Examples of KMessageWidget look as follows, all of them having an icon set
 * with setIcon(), and the first three show a close button:
 *
 * \image kmessagewidget.png "KMessageWidget with different message types"
 *
 * \section1 Negative feedback
 *
 * The KMessageWidget can be used as a secondary indicator of failure: the
 * first indicator is usually the fact the action the user expected to happen
 * did not happen.
 *
 * Example: User fills a form, clicks "Submit".
 * \list
 * \li Expected feedback: form closes
 * \li First indicator of failure: form stays there
 * \li Second indicator of failure: a KMessageWidget appears on top of the
 * form, explaining the error condition
 * \endlist
 *
 * When used to provide negative feedback, KMessageWidget should be placed
 * close to its context. In the case of a form, it should appear on top of the
 * form entries.
 *
 * KMessageWidget should get inserted in the existing layout. Space should not
 * be reserved for it, otherwise it becomes "dead space", ignored by the user.
 * KMessageWidget should also not appear as an overlay to prevent blocking
 * access to elements the user needs to interact with to fix the failure.
 *
 * \section1 Positive feedback
 *
 * KMessageWidget can be used for positive feedback but it shouldn't be
 * overused. It is often enough to provide feedback by simply showing the
 * results of an action.
 *
 * Examples of acceptable uses:
 * \list
 * \li Confirm success of "critical" transactions
 * \li Indicate completion of background tasks
 * \endlist
 *
 * Example of unadapted uses:
 * \list
 * \li Indicate successful saving of a file
 * \li Indicate a file has been successfully removed
 * \endlist
 *
 * \section1 Opportunistic interaction
 *
 * Opportunistic interaction is the situation where the application suggests to
 * the user an action he could be interested in perform, either based on an
 * action the user just triggered or an event which the application noticed.
 *
 * Example of acceptable uses:
 * \list
 * \li A browser can propose remembering a recently entered password
 * \li A music collection can propose ripping a CD which just got inserted
 * \li A chat application may notify the user a "special friend" just connected
 * \endlist
 *
 * \since 4.7
 */
class KWIDGETSADDONS_EXPORT KMessageWidget : public QFrame
{
    Q_OBJECT

    /*!
     * \property KMessageWidget::text
     */
    Q_PROPERTY(QString text READ text WRITE setText)

    /*!
     * \property KMessageWidget::textFormat
     */
    Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat)

    /*!
     * \property KMessageWidget::wordWrap
     */
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)

    /*!
     * \property KMessageWidget::closeButtonVisible
     */
    Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible)

    /*!
     * \property KMessageWidget::messageType
     */
    Q_PROPERTY(MessageType messageType READ messageType WRITE setMessageType)

    /*!
     * \property KMessageWidget::icon
     */
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

    /*!
     * \property KMessageWidget::position
     */
    Q_PROPERTY(Position position READ position WRITE setPosition)
public:
    /*!
     * Available message types.
     * The background colors are chosen depending on the message type.
     *
     * \value Positive Positive message type
     * \value Information Information message type
     * \value Warning Warning message type
     * \value Error Error message type
     */
    enum MessageType {
        Positive,
        Information,
        Warning,
        Error,
    };
    Q_ENUM(MessageType)

    /*!
     * Position of the KMessageWidget
     *
     * This will update the look of the KMessageWidget to be appropriate to the position.
     *
     * \value Inline The message widget is display inside the content.
     * \value Header The message widget is displayed as header.
     * \value Footer The message widget is displayed as footer.
     *
     * \since 6.0
     */
    enum Position {
        Inline,
        Header,
        Footer,
    };
    Q_ENUM(Position);

    /*!
     * Constructs a KMessageWidget with the specified \a parent.
     */
    explicit KMessageWidget(QWidget *parent = nullptr);

    /*!
     * Constructs a KMessageWidget with the specified \a parent and
     * contents \a text.
     */
    explicit KMessageWidget(const QString &text, QWidget *parent = nullptr);

    /*!
     * Destructor.
     */
    ~KMessageWidget() override;

    /*!
     * Get the position of this message. By default this is KMessageWidget::Inline.
     * \sa setPosition()
     * \since 6.0
     */
    Position position() const;

    /*!
     * Get the text of this message widget.
     * \sa setText()
     */
    QString text() const;

    /*!
     * Get the text format of the message widget's label.
     * \sa QLabel::textFormat()
     * \since 6.0
     */
    Qt::TextFormat textFormat() const;

    /*!
     * Set the text format of the message widget's label.
     * \sa QLabel::setTextFormat()
     * \since 6.0
     */
    void setTextFormat(Qt::TextFormat textFormat);

    /*!
     * Check whether word wrap is enabled.
     *
     * If word wrap is enabled, the message widget wraps the displayed text
     * as required to the available width of the widget. This is useful to
     * avoid breaking widget layouts.
     *
     * \sa setWordWrap()
     */
    bool wordWrap() const;

    /*!
     * Check whether the close button is visible.
     *
     * \sa setCloseButtonVisible()
     */
    bool isCloseButtonVisible() const;

    /*!
     * Get the type of this message.
     * By default, the type is set to KMessageWidget::Information.
     *
     * \sa KMessageWidget::MessageType, setMessageType()
     */
    MessageType messageType() const;

    /*!
     * Add \a action to the message widget.
     *
     * For each action a button is added to the message widget in the
     * order the actions were added.
     *
     * \a action the action to add
     * \sa removeAction(), QWidget::actions()
     */
    void addAction(QAction *action);

    /*!
     * Remove \a action from the message widget.
     *
     * \a action the action to remove
     *
     * \sa KMessageWidget::MessageType, addAction(), setMessageType()
     */
    void removeAction(QAction *action);

    /*!
     * Clears all actions from the message widget.
     *
     * \sa KMessageWidget::MessageType, addAction() and removeAction()
     * \since 5.100
     */
    void clearActions();

    QSize sizeHint() const override;

    QSize minimumSizeHint() const override;

    int heightForWidth(int width) const override;

    /*!
     * The icon shown on the left of the text. By default, no icon is shown.
     * \since 4.11
     */
    QIcon icon() const;

    /*!
     * Check whether the hide animation started by calling animatedHide()
     * is still running. If animations are disabled, this function always
     * returns \c false.
     *
     * \sa animatedHide(), hideAnimationFinished()
     * \since 5.0
     */
    bool isHideAnimationRunning() const;

    /*!
     * Check whether the show animation started by calling animatedShow()
     * is still running. If animations are disabled, this function always
     * returns \c false.
     *
     * \sa animatedShow(), showAnimationFinished()
     * \since 5.0
     */
    bool isShowAnimationRunning() const;

public Q_SLOTS:
    /*!
     * Set the text of the message widget to \a text.
     * If the message widget is already visible, the text changes on the fly.
     *
     * \a text the text to display, rich text is allowed
     *
     * \sa text()
     */
    void setText(const QString &text);

    /*!
     * Set the position of this message
     * \sa position()
     * \since 6.0
     */
    void setPosition(Position position);

    /*!
     * Set word wrap to \a wordWrap. If word wrap is enabled, the text()
     * of the message widget is wrapped to fit the available width.
     * If word wrap is disabled, the message widget's minimum size is
     * such that the entire text fits.
     *
     * By default word wrap is disabled.
     *
     * \a wordWrap disable/enable word wrap
     * \sa wordWrap()
     */
    void setWordWrap(bool wordWrap);

    /*!
     * Set the visibility of the close button. If \a visible is \c true,
     * a close button is shown that calls animatedHide() if clicked.
     *
     * By default the close button is set to be visible.
     *
     * \sa closeButtonVisible(), animatedHide()
     */
    void setCloseButtonVisible(bool visible);

    /*!
     * Set the message type to \a type.
     * By default, the message type is set to KMessageWidget::Information.
     * Appropriate colors are chosen to mimic the appearance of Kirigami's
     * InlineMessage.
     *
     * \sa messageType(), KMessageWidget::MessageType
     */
    void setMessageType(KMessageWidget::MessageType type);

    /*!
     * Show the widget using an animation.
     */
    void animatedShow();

    /*!
     * Hide the widget using an animation.
     */
    void animatedHide();

    /*!
     * Define an icon to be shown on the left of the text
     * \since 4.11
     */
    void setIcon(const QIcon &icon);

Q_SIGNALS:
    /*!
     * This signal is emitted when the user clicks a link in the text label.
     *
     * The URL referred to by the href anchor is passed in contents.
     *
     * \a contents text of the href anchor
     *
     * \sa QLabel::linkActivated()
     * \since 4.10
     */
    void linkActivated(const QString &contents);

    /*!
     * This signal is emitted when the user hovers over a link in the text label.
     *
     * The URL referred to by the href anchor is passed in contents.
     *
     * \a contents text of the href anchor
     *
     * \sa QLabel::linkHovered()
     * \since 4.11
     */
    void linkHovered(const QString &contents);

    /*!
     * This signal is emitted when the hide animation is finished, started by
     * calling animatedHide(). If animations are disabled, this signal is
     * emitted immediately after the message widget got hidden.
     *
     * \note This signal is not emitted if the widget was hidden by
     *       calling hide(), so this signal is only useful in conjunction
     *       with animatedHide().
     *
     * \sa animatedHide()
     * \since 5.0
     */
    void hideAnimationFinished();

    /*!
     * This signal is emitted when the show animation is finished, started by
     * calling animatedShow(). If animations are disabled, this signal is
     * emitted immediately after the message widget got shown.
     *
     * \note This signal is not emitted if the widget was shown by
     *       calling show(), so this signal is only useful in conjunction
     *       with animatedShow().
     *
     * \sa animatedShow()
     * \since 5.0
     */
    void showAnimationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

    bool event(QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

private:
    friend class KMessageWidgetPrivate;
    std::unique_ptr<class KMessageWidgetPrivate> const d;
};

#endif /* KMESSAGEWIDGET_H */
