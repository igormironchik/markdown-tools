/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colors.h"

// Qt include.
#include <QColorDialog>

namespace MdEditor
{

bool operator!=(const Colors &c1,
                const Colors &c2)
{
    return (c1.m_enabled != c2.m_enabled
            || c1.m_inlineColor != c2.m_inlineColor
            || c1.m_linkColor != c2.m_linkColor
            || c1.m_textColor != c2.m_textColor
            || c1.m_htmlColor != c2.m_htmlColor
            || c1.m_tableColor != c2.m_tableColor
            || c1.m_codeColor != c2.m_codeColor
            || c1.m_enabled != c2.m_enabled
            || c1.m_mathColor != c2.m_mathColor
            || c1.m_referenceColor != c2.m_referenceColor
            || c1.m_specialColor != c2.m_specialColor);
}

//
// ColorsPage
//

ColorsPage::ColorsPage(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    connect(m_ui.linkColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseLinkColor);
    connect(m_ui.textColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseTextColor);
    connect(m_ui.inlineColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseInlineColor);
    connect(m_ui.htmlColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseHtmlColor);
    connect(m_ui.tableColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseTableColor);
    connect(m_ui.codeColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseCodeColor);
    connect(m_ui.colors, &QGroupBox::toggled, this, &ColorsPage::colorsToggled);
    connect(m_ui.specialColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseSpecialColor);
    connect(m_ui.referenceColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseReferenceColor);
    connect(m_ui.mathColor, &MdShared::ColorWidget::clicked, this, &ColorsPage::chooseMathColor);
}

Ui::ColorsPage &ColorsPage::ui()
{
    return m_ui;
}

Colors &ColorsPage::colors()
{
    return m_colors;
}

void ColorsPage::resetDefaults()
{
    m_colors = {};

    applyColors();
}

void ColorsPage::applyColors()
{
    m_ui.colors->setChecked(m_colors.m_enabled);

    m_ui.inlineColor->setColor(m_colors.m_inlineColor);
    m_ui.linkColor->setColor(m_colors.m_linkColor);
    m_ui.textColor->setColor(m_colors.m_textColor);
    m_ui.htmlColor->setColor(m_colors.m_htmlColor);
    m_ui.tableColor->setColor(m_colors.m_tableColor);
    m_ui.codeColor->setColor(m_colors.m_codeColor);
    m_ui.mathColor->setColor(m_colors.m_mathColor);
    m_ui.referenceColor->setColor(m_colors.m_referenceColor);
    m_ui.specialColor->setColor(m_colors.m_specialColor);
}

void ColorsPage::chooseColor(MdShared::ColorWidget *w,
                             QColor &c)
{
    QColorDialog dlg(c, this);

    if (dlg.exec() == QDialog::Accepted) {
        w->setColor(dlg.currentColor());
        c = dlg.currentColor();
    }
}

void ColorsPage::chooseLinkColor()
{
    chooseColor(m_ui.linkColor, m_colors.m_linkColor);
}

void ColorsPage::chooseTextColor()
{
    chooseColor(m_ui.textColor, m_colors.m_textColor);
}

void ColorsPage::chooseInlineColor()
{
    chooseColor(m_ui.inlineColor, m_colors.m_inlineColor);
}

void ColorsPage::chooseHtmlColor()
{
    chooseColor(m_ui.htmlColor, m_colors.m_htmlColor);
}

void ColorsPage::chooseTableColor()
{
    chooseColor(m_ui.tableColor, m_colors.m_tableColor);
}

void ColorsPage::chooseCodeColor()
{
    chooseColor(m_ui.codeColor, m_colors.m_codeColor);
}

void ColorsPage::chooseMathColor()
{
    chooseColor(m_ui.mathColor, m_colors.m_mathColor);
}

void ColorsPage::chooseReferenceColor()
{
    chooseColor(m_ui.referenceColor, m_colors.m_referenceColor);
}

void ColorsPage::chooseSpecialColor()
{
    chooseColor(m_ui.specialColor, m_colors.m_specialColor);
}

void ColorsPage::colorsToggled(bool on)
{
    m_colors.m_enabled = on;
}

} /* namespace MdEditor */
