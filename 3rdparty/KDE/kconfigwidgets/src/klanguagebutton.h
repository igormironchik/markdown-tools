/*
    klangbutton.h - Button with language selection drop down menu.
                    Derived from the KLangCombo class by Hans Petter Bieker.

    SPDX-FileCopyrightText: 1999-2003 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2001 Martijn Klingens <klingens@kde.org>
    SPDX-FileCopyrightText: 2007 David Jarvie <djarvie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLANGUAGEBUTTON_H
#define KLANGUAGEBUTTON_H

#include "kconfigwidgets_export.h"
#include <QWidget>
#include <memory>

class QAction;
class KLanguageButtonPrivate;

/*!
 * \class KLanguageButton
 * \inmodule KConfigWidgets
 *
 * \brief KLanguageButton is a pushbutton which allows a language to be selected from
 * a popup list.
 *
 * Languages are identified by their ISO 639-1 codes, e.g. en, pt_BR.
 *
 * \image klanguagebutton.png "KDE Language Selection Widget"
 */
class KCONFIGWIDGETS_EXPORT KLanguageButton : public QWidget
{
    Q_OBJECT

public:
    /*!
     * Constructs a button whose text is determined by the current language
     * in the popup list.
     *
     * \a parent the parent of the button
     */
    explicit KLanguageButton(QWidget *parent = nullptr);

    /*!
     * Constructs a button with static text.
     *
     * \a text the text of the button
     *
     * \a parent the parent of the button
     */
    explicit KLanguageButton(const QString &text, QWidget *parent = nullptr);

    ~KLanguageButton() override;

    /*!
     * Sets the locale to display language names. By default, QLocale::system().name() is used.
     *
     * \a locale locale to use
     */
    void setLocale(const QString &locale);

    /*!
     * Sets a static button text.
     *
     * \a text button text
     */
    void setText(const QString &text);

    /*!
     * Specifies whether language codes should be shown alongside language names
     * in the popup. Calling this method does not affect any previously
     * inserted language texts, so it should normally be called before
     * populating the list.
     *
     * \a show \c true to show codes, \c false to hide codes
     */
    void showLanguageCodes(bool show);

    /*!
     * Load all known languages into the popup list.
     *
     * The current language in the list is set to the default language for the
     * current locale (as modified by setLocale()).
     */
    void loadAllLanguages();

    /*!
     * Inserts a language into the combo box.
     *
     * Normally the display name of the language is obtained automatically, but
     * if either the language code does not exist, or there are special display
     * requirements, the name of the language can be specified in \a name.
     *
     * \a languageCode the code for the language
     *
     * \a name language name. If empty, the name is obtained automatically
     *
     * \a index the insertion position, or -1 to insert in alphabetical order
     */
    void insertLanguage(const QString &languageCode, const QString &name = QString(), int index = -1);

    /*!
     * Inserts a separator item into the combo box. A negative index will append the item.
     *
     * \a index the insertion position
     */
    void insertSeparator(int index = -1);

    /*!
     * Returns the number of items in the combo box.
     */
    int count() const;

    /*!
     * Removes all combobox items.
     */
    void clear();

    /*!
     * Returns the language code of the combobox's current item.
     */
    QString current() const;

    /*!
     * Checks whether the specified language is in the popup list.
     *
     * \a languageCode the language's code
     */
    bool contains(const QString &languageCode) const;

    /*!
     * Sets a given language to be the current item.
     *
     * \a languageCode the language's code
     */
    void setCurrentItem(const QString &languageCode);

Q_SIGNALS:
    /*!
     * This signal is emitted when a new item is activated.
     *
     * \a languageCode code of the activated language
     */
    void activated(const QString &languageCode);
    /*!
     * This signal is emitted when a new item is highlighted.
     *
     * \a languageCode code of the highlighted language
     */
    void highlighted(const QString &languageCode);

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    KCONFIGWIDGETS_NO_EXPORT void slotTriggered(QAction *action);
    KCONFIGWIDGETS_NO_EXPORT void slotHovered(QAction *action);

private:
    friend class KLanguageButtonPrivate;
    std::unique_ptr<KLanguageButtonPrivate> const d;
};

#endif
