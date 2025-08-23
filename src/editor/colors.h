/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_colors.h"

// shared include.
#include "syntax.h"

namespace MdEditor
{

//
// Colors
//

//! Color scheme.
struct Colors {
    //! Color for text.
    QColor m_textColor = QColor(0, 0, 128);
    //! Color for link.
    QColor m_linkColor = QColor(0, 128, 0);
    //! Color for inline code.
    QColor m_inlineColor = Qt::black;
    //! Color for HTML.
    QColor m_htmlColor = QColor(128, 0, 0);
    //! Color for table borders.
    QColor m_tableColor = Qt::black;
    //! Color for code.
    QColor m_codeColor = Qt::black;
    //! Color for LaTeX Math injections.
    QColor m_mathColor = QColor(128, 0, 0);
    //! Color for reference link.
    QColor m_referenceColor = QColor(128, 0, 0);
    //! Color for special symbols - delimiters...
    QColor m_specialColor = QColor(128, 0, 0);
    //! Theme for code blocks.
    QString m_codeTheme = QStringLiteral("GitHub Light");
    //! Is color scheme enabled?
    bool m_enabled = true;
    //! Is theme for code blocks enabled?
    bool m_codeThemeEnabled = true;
    //! Should code blocks background be drawn?
    bool m_drawCodeBackground = true;
}; // struct Colors

//! Not-equal operator for color schemes.
bool operator!=(const Colors &c1,
                const Colors &c2);

//
// ColorsPage
//

//! Settings page with colors.
class ColorsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ColorsPage(QWidget *parent = nullptr);
    ~ColorsPage() override = default;

    //! \return UI component.
    Ui::ColorsPage &ui();
    //! \return Reference for color scheme.
    Colors &colors();

public slots:
    //! Reset to defaults.
    void resetDefaults();
    //! Apply color scheme for settings page UI.
    void applyColors();
    //! Choose link color.
    void chooseLinkColor();
    //! Choose text color.
    void chooseTextColor();
    //! Choose inline color.
    void chooseInlineColor();
    //! Choose HTML color.
    void chooseHtmlColor();
    //! Choose table's borders color.
    void chooseTableColor();
    //! Choose code color.
    void chooseCodeColor();
    //! Choose LaTeX Math injection color.
    void chooseMathColor();
    //! Choose reference link color.
    void chooseReferenceColor();
    //! Choose color for special symbols.
    void chooseSpecialColor();
    //! Enable/disable colors scheme.
    void colorsToggled(bool on);
    //! Init code themes combo box.
    void initCodeThemes(std::shared_ptr<MdShared::Syntax> syntax);

private:
    //! Base implementation of color choosing.
    void chooseColor(MdShared::ColorWidget *w,
                     QColor &c);

private:
    Q_DISABLE_COPY(ColorsPage)

    //! UI component.
    Ui::ColorsPage m_ui;
    //! Colors scheme.
    Colors m_colors;
}; // class ColorsPage

} /* namespace MdEditor */
