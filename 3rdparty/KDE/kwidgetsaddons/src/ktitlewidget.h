/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007-2009 Urs Wolfer <uwolfer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTITLEWIDGET_H
#define KTITLEWIDGET_H

#include <kwidgetsaddons_export.h>

#include <QWidget>
#include <memory>

/*!
 * \class KTitleWidget
 * \inmodule KWidgetsAddons
 *
 * \brief Standard title widget.
 *
 * This class provides a widget often used for dialog titles.
 *
 * \image ktitlewidget.png "KTitleWidget with title and icon"
 *
 * KTitleWidget uses the general application font at 1.4 times its size to
 * style the text. This is a visual change from 4.x.
 *
 * Usage:
 *
 * KTitleWidget is very simple to use. You can either use its default text
 * (and pixmap) properties or display your own widgets in the title widget.
 *
 * A title text with a right-aligned pixmap:
 * \code
 * KTitleWidget *titleWidget = new KTitleWidget(this);
 * titleWidget->setText(i18n("Title"));
 * titleWidget->setIcon(QIcon::fromTheme("screen"));
 * \endcode
 *
 * Use it with an own widget:
 * \code
 * KTitleWidget *checkboxTitleWidget = new KTitleWidget(this);
 *
 * QWidget *checkBoxTitleMainWidget = new QWidget(this);
 * QVBoxLayout *titleLayout = new QVBoxLayout(checkBoxTitleMainWidget);
 * titleLayout->setContentsMargins(6, 6, 6, 6);
 *
 * QCheckBox *checkBox = new QCheckBox("Text Checkbox", checkBoxTitleMainWidget);
 * titleLayout->addWidget(checkBox);
 *
 * checkboxTitleWidget->setWidget(checkBoxTitleMainWidget);
 * \endcode
 *
 * \sa KPageView
 */
class KWIDGETSADDONS_EXPORT KTitleWidget : public QWidget
{
    Q_OBJECT

    /*!
     * \property KTitleWidget::text
     */
    Q_PROPERTY(QString text READ text WRITE setText)

    /*!
     * \property KTitleWidget::comment
     */
    Q_PROPERTY(QString comment READ comment WRITE setComment)

    /*!
     * \property KTitleWidget::icon
     * \since 5.72
     */
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

    /*!
     * \property KTitleWidget::iconSize
     * \since 5.72
     */
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

    /*!
     * \property KTitleWidget::autoHideTimeout
     */
    Q_PROPERTY(int autoHideTimeout READ autoHideTimeout WRITE setAutoHideTimeout)

public:
    /*!
     * Possible title pixmap alignments.
     *
     * \value ImageLeft Display the pixmap left
     * \value ImageRight Display the pixmap right (default)
     */
    enum ImageAlignment {
        ImageLeft,
        ImageRight,
    };
    Q_ENUM(ImageAlignment)

    /*!
     * Comment message types
     *
     * \value PlainMessage Normal comment
     * \value InfoMessage Information the user should be alerted to
     * \value WarningMessage A warning the user should be alerted to
     * \value ErrorMessage An error message
     */
    enum MessageType {
        PlainMessage,
        InfoMessage,
        WarningMessage,
        ErrorMessage,
    };

    /*!
     * Constructs a title widget.
     */
    explicit KTitleWidget(QWidget *parent = nullptr);

    ~KTitleWidget() override;

    /*!
     * \a widget Widget displayed on the title widget.
     */
    void setWidget(QWidget *widget);

    /*!
     * Returns the text displayed in the title
     * \sa setText()
     */
    QString text() const;

    /*!
     * Returns the text displayed in the comment below the title, if any
     * \sa setComment()
     */
    QString comment() const;

    /*!
     * Returns the icon displayed in the title
     * \sa setIcon()
     *
     * \since 5.72
     */
    QIcon icon() const;

    /*!
     * Returns the size of the icon displayed in the title
     * \sa setIconSize()
     *
     * \since 5.72
     */
    QSize iconSize() const;

    /*!
     * Sets this label's buddy to buddy.
     *
     * When the user presses the shortcut key indicated by the label in this
     * title widget, the keyboard focus is transferred to the label's buddy
     * widget.
     *
     * \a buddy the widget to activate when the shortcut key is activated
     */
    void setBuddy(QWidget *buddy);

    /*!
     * Get the current timeout value in milliseconds
     *
     * Returns timeout value in msecs
     */
    int autoHideTimeout() const;

    /*!
     * Returns the level of this title: it influences the font size following the guidelines in
     *         the \l {https://develop.kde.org/hig/text_and_labels/} {KDE HIG}.
     *         It also corresponds to the level api of Kirigami Heading for QML applications
     * \note The default level is 1.
     * \since 5.53
     */
    int level();

public Q_SLOTS:
    /*!
     * \a text Text displayed on the label. It can either be plain text or rich text.
     *
     * \a alignment Alignment of the text. Default is left and vertical centered.
     *
     * \sa text()
     */
    void setText(const QString &text, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

    /*!
     * \a text Text displayed on the label. It can either be plain text or rich text.
     *
     * \a type The sort of message it is; will also set the icon accordingly
     * \sa text()
     */
    void setText(const QString &text, MessageType type);

    /*!
     * \a comment Text displayed beneath the main title as a comment.
     *                It can either be plain text or rich text.
     *
     * \a type The sort of message it is.
     *
     * \sa comment()
     */
    void setComment(const QString &comment, MessageType type = PlainMessage);

    /*!
     * Set the icon to display in the header.
     *
     * \a icon the icon to display in the header.
     *
     * \a alignment alignment of the icon (default is right aligned).
     *
     * \since 5.63
     */
    void setIcon(const QIcon &icon, ImageAlignment alignment = ImageRight);

    /*!
     * \a type the type of message icon to display in the header
     *
     * \a alignment alignment of the icon (default is right aligned).
     *
     * \sa icon()
     *
     * \since 5.72
     */
    void setIcon(MessageType type, ImageAlignment alignment = ImageRight);

    /*!
     * Set the size of the icon to display in the header.
     *
     * \a iconSize the size of the icon, or an invalid QSize to reset to the default
     *
     * The default size is defined by the GUI style and its value for QStyle::PM_MessageBoxIconSize.
     *
     * \since 5.72
     */
    void setIconSize(const QSize &iconSize);

    /*!
     * Set the autohide timeout of the label
     *
     * Set value to 0 to disable autohide, which is the default.
     *
     * \a msecs timeout value in milliseconds
     */
    void setAutoHideTimeout(int msecs);

    /*!
     * Sets the level of this title, similar to HTML's h1 h2 h3...
     *
     * Follows the \l {https://develop.kde.org/hig/text_and_labels/} {KDE HIG}.
     *
     * \a level the level of the title, 1 is the biggest font and most important, descending
     * \since 5.53
     */
    void setLevel(int level);

protected:
    void changeEvent(QEvent *e) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    std::unique_ptr<class KTitleWidgetPrivate> const d;

    Q_DISABLE_COPY(KTitleWidget)
};

#endif
