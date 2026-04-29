/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_colors.h"

// shared include.
#include "syntax.h"

class KColorButton;

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
    //! Init code themes combo box.
    void initCodeThemes(QSharedPointer<MdShared::Syntax> syntax);

private slots:
    //! Link color changed.
    void linkColorChanged(const QColor &c);
    //! Text color changed.
    void textColorChanged(const QColor &c);
    //! Inline color changed.
    void inlineColorChanged(const QColor &c);
    //! HTML color changed.
    void htmlColorChanged(const QColor &c);
    //! Table's borders color changed.
    void tableColorChanged(const QColor &c);
    //! Code color changed.
    void codeColorChanged(const QColor &c);
    //! LaTeX Math injection color changed.
    void mathColorChanged(const QColor &c);
    //! Reference link color changed.
    void referenceColorChanged(const QColor &c);
    //! Color for special symbols changed.
    void specialColorChanged(const QColor &c);
    //! Enable/disable colors scheme.
    void colorsToggled(bool on);

private:
    Q_DISABLE_COPY(ColorsPage)

    //! UI component.
    Ui::ColorsPage m_ui;
    //! Colors scheme.
    Colors m_colors;
}; // class ColorsPage

} /* namespace MdEditor */
