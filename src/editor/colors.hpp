/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_colors.h"

namespace MdEditor
{

//
// Colors
//

//! Color scheme.
struct Colors {
    QColor m_textColor = QColor(0, 0, 128);
    QColor m_linkColor = QColor(0, 128, 0);
    QColor m_inlineColor = Qt::black;
    QColor m_htmlColor = QColor(128, 0, 0);
    QColor m_tableColor = Qt::black;
    QColor m_codeColor = Qt::black;
    QColor m_mathColor = QColor(128, 0, 0);
    QColor m_referenceColor = QColor(128, 0, 0);
    QColor m_specialColor = QColor(128, 0, 0);
    bool m_enabled = true;
}; // struct Colors

bool operator!=(const Colors &c1, const Colors &c2);

//
// ColorsPage
//

//! Page with colors.
class ColorsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ColorsPage(QWidget *parent = nullptr);
    ~ColorsPage() override = default;

    Ui::ColorsPage &ui();
    Colors &colors();

public slots:
    void resetDefaults();
    void applyColors();
    void chooseLinkColor();
    void chooseTextColor();
    void chooseInlineColor();
    void chooseHtmlColor();
    void chooseTableColor();
    void chooseCodeColor();
    void chooseMathColor();
    void chooseReferenceColor();
    void chooseSpecialColor();
    void colorsToggled(bool on);

private:
    void chooseColor(MdShared::ColorWidget *w, QColor &c);

private:
    Q_DISABLE_COPY(ColorsPage)

    Ui::ColorsPage m_ui;
    Colors m_colors;
}; // class ColorsPage

} /* namespace MdEditor */
