/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kcharselect_h
#define kcharselect_h

#include <QString>
#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

class QFont;

/*!
 * \class KCharSelect
 * \inmodule KWidgetsAddons
 *
 * \brief Character selection widget.
 *
 * This widget allows the user to select a character of a
 * specified font and to browse Unicode information
 *
 * \image kcharselect.png "KCharSelect Widget"
 *
 * You can specify the font whose characters should be displayed via
 * setCurrentFont(). Using the Controls argument in the constructor
 * you can create a compact version of KCharSelect if there is not enough
 * space and if you don't need all features.
 *
 * KCharSelect displays one Unicode block at a time and provides
 * categorized access to them. Unicode character names and further details,
 * including cross references, are displayed. Additionally, there is a search
 * to find characters.
 *
 * By default, KCharSelect is restricted to Basic Multilingual Plane (BMP)
 * characters that QChar supports, i.e. characters with code points that
 * fit into a quint16 (U+0000..U+FFFF). API methods that have a QChar
 * argument can only be used for this default mode:
 *
 * To get the current selected character, use the currentChar()
 * method. You can set the character which should be displayed with
 * setCurrentChar().
 *
 * If you want the user to select and search characters from all planes,
 * i.e. characters U+0000..U+10FFFF, use setAllPlanesEnabled(true)
 * and use the \c uint based methods currentCodePoint() and
 * setCurrentCodePoint() instead.
 *
 * Since QString does not allow \c uint code points, you either must
 * use QString::fromUcs4() and QString::ToUcs4() to convert between
 * strings and code points, or manually do the surrogate pair handling
 * using QChar::requiresSurrogates() and friends.
 */
class KWIDGETSADDONS_EXPORT KCharSelect : public QWidget
{
    Q_OBJECT

    /*!
     * \property KCharSelect::currentFont
     */
    Q_PROPERTY(QFont currentFont READ currentFont WRITE setCurrentFont)

    /*!
     * \property KCharSelect::currentChar
     */
    Q_PROPERTY(QChar currentChar READ currentChar WRITE setCurrentChar)

    /*!
     * \property KCharSelect::currentCodePoint
     */
    Q_PROPERTY(uint currentCodePoint READ currentCodePoint WRITE setCurrentCodePoint NOTIFY currentCodePointChanged)

    /*!
     * \property KCharSelect::displayedChars
     */
    Q_PROPERTY(QList<QChar> displayedChars READ displayedChars)

    /*!
     * \property KCharSelect::displayedCodePoints
     */
    Q_PROPERTY(QList<uint> displayedCodePoints READ displayedCodePoints)

    /*!
     * \property KCharSelect::allPlanesEnabled
     */
    Q_PROPERTY(bool allPlanesEnabled READ allPlanesEnabled WRITE setAllPlanesEnabled DESIGNABLE true)

public:
    /*!
     * Flags to set the shown widgets
     *
     * \value SearchLine Shows the search widgets
     * \value FontCombo Shows the font combo box
     * \value FontSize Shows the font size spin box
     * \value BlockCombos Shows the category/block selection combo boxes
     * \value CharacterTable Shows the actual table
     * \value DetailBrowser Shows the detail browser
     * \value HistoryButtons Shows the Back/Forward buttons
     * \value AllGuiElements Shows everything
     */
    enum Control {
        SearchLine = 0x01,
        FontCombo = 0x02,
        FontSize = 0x04,
        BlockCombos = 0x08,
        CharacterTable = 0x10,
        DetailBrowser = 0x20,
        HistoryButtons = 0x40,
        AllGuiElements = 65535,
    };
    Q_DECLARE_FLAGS(Controls, Control)

    /*!
     * Constructor. \a controls can be used to show a custom set of widgets.
     *
     * \a parent the parent widget for this KCharSelect (see QWidget documentation)
     *
     * \a controls selects the visible controls on the KCharSelect widget
     *
     * \since 4.2
     */
    explicit KCharSelect(QWidget *parent, const Controls controls = AllGuiElements);

    /*!
     * Constructor. \a controls can be used to show a custom set of widgets.
     *
     * The widget uses the following actions:
     * \list
     * \li KStandardActions::find() (edit_find)
     * \li KStandardActions::back() (go_back)
     * \li KStandardActions::forward() (go_forward)
     * \endlist
     *
     * If you provide a KActionCollection, this will be populated with the above actions,
     * which you can then manually trigger or place in menus and toolbars.
     *
     * \a parent     the parent widget for this KCharSelect (see QWidget documentation)
     *
     * \a actionParent if this is not \c null, KCharSelect will place its actions into this
     *                     collection
     *
     * \a controls selects the visible controls on the KCharSelect widget
     *
     * \since 4.2
     */
    explicit KCharSelect(QWidget *parent, QObject *actionParent, const Controls controls = AllGuiElements);

    ~KCharSelect() override;

    QSize sizeHint() const override;

    /*!
     * Sets the allowed Unicode code planes. If \a all is \c false, then
     * only characters from the Basic Multilingual Plane (BMP) can be
     * selected, otherwise characters from all planes are allowed.
     *
     * For compatibility reasons, the default is \c false.
     *
     * If you enable support for all planes, you must use the functions
     * handling \c uint code points instead of QChar characters.
     *
     * \since 5.25
     */
    void setAllPlanesEnabled(bool all);

    /*!
     * Returns \c true, if characters from all Unicode code planes
     * can be selected.
     *
     * \since 5.25
     */
    bool allPlanesEnabled() const;

    /*!
     * Returns the currently selected character. If characters outside the
     * Basic Multilingual Plane (BMP) can be selected, use currentCodePoint
     * instead.
     * \sa currentCodePoint
     */
    QChar currentChar() const;

    /*!
     * Returns the Unicode code point of the currently selected character.
     *
     * \warning If you enabled support for all Unicode planes, you must use
     * QChar::requiresSurrogates() to check if the code point requires
     * conversion to a UTF-16 surrogate pair before converting it to QString.
     * You cannot convert a code point to a QChar.
     * \since 5.25
     */
    uint currentCodePoint() const;

    /*!
     * Returns the currently displayed font.
     */
    QFont currentFont() const;

    /*!
     * Returns a list of currently displayed characters. If characters outside the
     * Basic Multilingual Plane (BMP) can be selected, use displayedCodePoints
     * instead.
     *
     * Warning: this method can be a bit slow
     * \sa displayedCodePoints
     */
    QList<QChar> displayedChars() const;

    /*!
     * Returns a list of Unicode code points of the currently displayed characters.
     * \since 5.25
     */
    QList<uint> displayedCodePoints() const;

public Q_SLOTS:
    /*!
     * Highlights the character \a c. If the character is not displayed, the block is changed.
     *
     * \a c the character to highlight
     */
    void setCurrentChar(const QChar &c);

    /*!
     * Highlights the character with the specified \a codePoint. If the character is
     * outside the Basic Multilingual Plane (BMP), then you must enable support
     * for all planes for this to work.
     *
     * \a codePoint the Unicode code point of the character to highlight
     *
     * \sa allPlanesEnabled
     * \since 5.25
     */
    void setCurrentCodePoint(uint codePoint);

    /*!
     * Sets the font which is displayed to \a font
     *
     * \a font the display font for the widget
     */
    void setCurrentFont(const QFont &font);

Q_SIGNALS:
    /*!
     * A new font is selected or the font size changed.
     *
     * \a font the new font
     */
    void currentFontChanged(const QFont &font);
    /*!
     * The current character is changed.
     *
     * \a c the new character
     */
    void currentCharChanged(const QChar &c);
    /*!
     * The current character is changed.
     *
     * \a codePoint the Unicode code point of the new character
     * \since 5.25
     */
    void currentCodePointChanged(uint codePoint);
    /*!
     * The currently displayed characters are changed (search results or block).
     */
    void displayedCharsChanged();
    /*!
     * A character is selected to be inserted somewhere.
     *
     * \a c the selected character
     */
    void charSelected(const QChar &c);
    /*!
     * A character is selected to be inserted somewhere.
     *
     * \a codePoint the Unicode code point of the selected character
     * \since 5.25
     */
    void codePointSelected(uint codePoint);

private:
    KWIDGETSADDONS_NO_EXPORT void initWidget(const Controls, QObject *);

private:
    std::unique_ptr<class KCharSelectPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KCharSelect::Controls)

#endif
