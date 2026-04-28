/*
    SPDX-FileCopyrightText: 2003 Nadeem Hasan <nhasan@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFONTREQUESTER_H
#define KFONTREQUESTER_H

#include <QFont>
#include <QString>
#include <QWidget>
#include <memory>

#include <kwidgetsaddons_export.h>

class QLabel;
class QPushButton;

/*!
 * \class KFontRequester
 * \inmodule KWidgetsAddons
 *
 * \brief This class provides a widget with a lineedit and a button, which invokes
 * a font dialog (KFontChooserDialog).
 *
 * The lineedit provides a preview of the selected font. The preview text can
 * be customized. You can also have the font dialog show only the fixed fonts.
 *
 * \image kfontrequester.png "KFontRequester"
 */
class KWIDGETSADDONS_EXPORT KFontRequester : public QWidget
{
    Q_OBJECT

    /*!
     * \property KFontRequester::title
     */
    Q_PROPERTY(QString title READ title WRITE setTitle)

    /*!
     * \property KFontRequester::sampleText
     */
    Q_PROPERTY(QString sampleText READ sampleText WRITE setSampleText)

    /*!
     * \property KFontRequester::font
     */
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontSelected USER true)

public:
    /*!
     * Constructs a font requester widget.
     *
     * \a parent The parent widget.
     *
     * \a onlyFixed Only display fonts which have fixed-width character
     *        sizes.
     */
    explicit KFontRequester(QWidget *parent = nullptr, bool onlyFixed = false);

    ~KFontRequester() override;

    /*!
     * Returns the currently selected font in the requester.
     */
    QFont font() const;

    /*!
     * Returns \c true if only fixed fonts are displayed.
     */
    bool isFixedOnly() const;

    /*!
     * Returns the current text in the sample text input area.
     */
    QString sampleText() const;

    /*!
     * Returns the current title of the widget.
     */
    QString title() const;

    /*!
     * Returns pointer to the label used for preview.
     */
    QLabel *label() const;

    /*!
     * Returns pointer to the pushbutton in the widget.
     */
    QPushButton *button() const;

    /*!
     * Sets the currently selected font in the requester.
     *
     * \a font The font to select.
     *
     * \a onlyFixed Display only fixed-width fonts in the font dialog
     * if \c true, or vice-versa.
     */
    virtual void setFont(const QFont &font, bool onlyFixed = false);

    /*!
     * Sets the sample text.
     *
     * Normally you should not change this
     * text, but it can be better to do this if the default text is
     * too large for the edit area when using the default font of your
     * application. Default text is current font name and size. Setting
     * the text to QString() will restore the default.
     *
     * \a text The new sample text. The current will be removed.
     */
    virtual void setSampleText(const QString &text);

    /*!
     * Set the title for the widget that will be used in the tooltip and
     * what's this text.
     *
     * \a title The title to be set.
     */
    virtual void setTitle(const QString &title);

Q_SIGNALS:
    /*!
     * Emitted when a new \a font has been selected in the underlying dialog
     */
    void fontSelected(const QFont &font);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    friend class KFontRequesterPrivate;
    std::unique_ptr<class KFontRequesterPrivate> const d;

    Q_DISABLE_COPY(KFontRequester)
};

#endif // KFONTREQUESTER_H
