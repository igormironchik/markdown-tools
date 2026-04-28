/*
    SPDX-FileCopyrightText: 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999 Mario Weilguni <mweilguni@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef K_FONT_CHOOSER_H
#define K_FONT_CHOOSER_H

#include <QStringList>
#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

class QFont;

/*!
 * \class KFontChooser
 * \inmodule KWidgetsAddons
 *
 * \brief A font selection widget.
 *
 * While KFontChooser as an ordinary widget can be embedded in
 * custom dialogs and therefore is very flexible, in most cases
 * it is preferable to use the convenience functions in
 * QFontDialog.
 *
 * \image kfontchooser.png "KFontChooser Widget"
 *
 * \sa KFontRequester
 */
class KWIDGETSADDONS_EXPORT KFontChooser : public QWidget
{
    Q_OBJECT

    /*!
     * \property KFontChooser::font
     */
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontSelected USER true)

    /*!
     * \property KFontChooser::color
     */
    Q_PROPERTY(QColor color READ color WRITE setColor)

    /*!
     * \property KFontChooser::backgroundColor
     */
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

    /*!
     * \property KFontChooser::sampleText
     */
    Q_PROPERTY(QString sampleText READ sampleText WRITE setSampleText)

public:
    /*!
     * Displayed columns.
     *
     * \value FamilyList Identifies the family (leftmost) list.
     * \value StyleList Identifies the style (center) list.
     * \value SizeList Identifies the size (rightmost) list.
     */
    enum FontColumn {
        FamilyList = 0x01,
        StyleList = 0x02,
        SizeList = 0x04,
    };

    /*!
     * Flags for selecting which font attributes to change
     *
     * \value NoFontDiffFlags No flags set
     * \value FontDiffFamily Identifies a requested change in the font family.
     * \value FontDiffStyle Identifies a requested change in the font style.
     * \value FontDiffSize Identifies a requested change in the font size.
     * \value AllFontDiffs
     */
    enum FontDiff {
        NoFontDiffFlags = 0,
        FontDiffFamily = 1,
        FontDiffStyle = 2,
        FontDiffSize = 4,
        AllFontDiffs = FontDiffFamily | FontDiffStyle | FontDiffSize,
    };
    Q_DECLARE_FLAGS(FontDiffFlags, FontDiff)

    /*!
     * Flags for selecting what is displayed in the widget.
     *
     * \value NoDisplayFlags No flags set
     * \value FixedFontsOnly Only show monospaced/fixed-width fonts, *excluding proportional fonts, (the checkbox to toggle showing only monospaced fonts is not
     * shown in this case)
     * \value DisplayFrame = Show a visual frame around the chooser
     * \value ShowDifferences Display the font differences interfaces
     */
    enum DisplayFlag {
        NoDisplayFlags = 0,
        FixedFontsOnly = 1,
        DisplayFrame = 2,
        ShowDifferences = 4,
    };
    Q_DECLARE_FLAGS(DisplayFlags, DisplayFlag)

    /*!
     * Constructs a font picker widget.
     *
     * \a parent the parent widget
     *
     * \since 5.86
     */
    explicit KFontChooser(QWidget *parent = nullptr);

    /*!
     * Create a font picker widget.
     *
     * \a flags a combination of OR-ed values from the \c KFontChooser::DisplayFlags enum,
     * the default is \c DisplayFonts::NoDisplayFlags
     *
     * \a parent the parent widget, if not nullptr the windowing system will use it to position
     * the chooser widget relative to it
     *
     * \since 5.86
     */
    explicit KFontChooser(DisplayFlags flags, QWidget *parent = nullptr);

    ~KFontChooser() override;

    /*!
     * Enables or disables a column (family, style, size) in the widget.
     *
     * Use this function if your application does not need or support all font properties.
     *
     * \a column specify the column(s) to enable/disable, an OR-ed combination of
     *  KFontChooser::FontColumn enum values
     *
     * \a state if \a false the columns are disabled, and vice-versa
     */
    void enableColumn(int column, bool state);

    /*!
     * Sets the currently selected font in the widget.
     *
     * \a font the font to select
     *
     * \a onlyFixed if \c true, the font list will only display fixed-width fonts,
     * otherwise all fonts are displayed. The default is \c false.
     */
    void setFont(const QFont &font, bool onlyFixed = false);

    /*!
     * Returns the bitmask corresponding to the attributes the user wishes to change.
     */
    FontDiffFlags fontDiffFlags() const;

    /*!
     * Returns the currently selected font in the chooser.
     */
    QFont font() const;

    /*!
     * Sets the color to use for the font in the preview area.
     */
    void setColor(const QColor &col);

    /*!
     * Returns the color currently used for the font in the preview
     * area (default: the text color of the active color group).
     */
    QColor color() const;

    /*!
     * Sets the background color to use in the preview area.
     */
    void setBackgroundColor(const QColor &col);

    /*!
     * Returns the background color currently used in the preview area
     * (default: the base color of the active colorgroup)
     */
    QColor backgroundColor() const;

    /*!
     * Returns the current text in the sample text input area.
     */
    QString sampleText() const;

    /*!
     * Sets the sample text in the preview area; this is useful if you
     * want to use text in your native language.
     *
     * \a text the new sample text (it will replace the current text)
     */
    void setSampleText(const QString &text);

    /*!
     * If \a visible is \c true the preview area will be shown, and vice-versa
     * is it's \c false.
     */
    void setSampleBoxVisible(bool visible);

    /*!
     * The selection criteria for the font families shown in the dialog.
     *
     * \value FixedWidthFonts If set, only show fixed fixed-width (monospace) fonts.
     * \value ScalableFonts If set, only show scalable fonts. Certain configurations allow bitmap fonts to remain unscaled and thus these fonts have limited
     * number of sizes.
     * \value SmoothScalableFonts If set, only show smooth scalable fonts. This will return only non-bitmap fonts which are scalable to any size requested.
     * Setting this option means the \c ScalableFonts flag is ignored.
     */
    enum FontListCriteria {
        FixedWidthFonts = 0x01,
        ScalableFonts = 0x02,
        SmoothScalableFonts = 0x04
    };

    /*!
     * Returns a list of font faimly name strings filtered based on \a fontListCriteria.
     *
     * \a fontListCriteria specifies the criteria used to select fonts to add to
     * the list, a combination of OR-ed values from KFontChooser::FontListCriteria
     *
     * \since 5.86
     */
    static QStringList createFontList(uint fontListCriteria);

    /*!
     * Uses \a fontList to fill the font family list in the widget.
     *
     * You can create a custom list of fonts using the static createFontList(uint
     * criteria) to only include fonts that meet certain criteria (e.g. only
     * smooth-scalable fonts).
     *
     * \sa KFontChooser::createFontList(uint), KFontChooser::FontListCriteria
     *
     * Note that if \a fontList is empty, the font list in the chooser will show
     * all the available fonts on the system.
     * \since 5.86
     */
    void setFontListItems(const QStringList &fontList);

    /*!
     * Sets the minimum number of items that should be visible in the
     * child list widgets; this number will be used to compute and set
     * the minimum heights for those widgets.
     *
     * \since 5.86
     */
    void setMinVisibleItems(int visibleItems);

    QSize sizeHint(void) const override;

Q_SIGNALS:
    /*!
     * Emitted when the selected font changes.
     */
    void fontSelected(const QFont &font);

private:
    std::unique_ptr<class KFontChooserPrivate> const d;

    Q_DISABLE_COPY(KFontChooser)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFontChooser::DisplayFlags)

#endif
