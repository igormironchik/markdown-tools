/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colors.h"

// Qt include.
#include <QComboBox>

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
            || c1.m_specialColor != c2.m_specialColor
            || c1.m_codeTheme != c2.m_codeTheme
            || c1.m_codeThemeEnabled != c2.m_codeThemeEnabled
            || c1.m_drawCodeBackground != c2.m_drawCodeBackground);
}

//
// ColorsPage
//

ColorsPage::ColorsPage(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    connect(m_ui.linkColor, &KColorButton::changed, this, &ColorsPage::linkColorChanged);
    connect(m_ui.textColor, &KColorButton::changed, this, &ColorsPage::textColorChanged);
    connect(m_ui.inlineColor, &KColorButton::changed, this, &ColorsPage::inlineColorChanged);
    connect(m_ui.htmlColor, &KColorButton::changed, this, &ColorsPage::htmlColorChanged);
    connect(m_ui.tableColor, &KColorButton::changed, this, &ColorsPage::tableColorChanged);
    connect(m_ui.codeColor, &KColorButton::changed, this, &ColorsPage::codeColorChanged);
    connect(m_ui.colors, &QGroupBox::toggled, this, &ColorsPage::colorsToggled);
    connect(m_ui.specialColor, &KColorButton::changed, this, &ColorsPage::specialColorChanged);
    connect(m_ui.referenceColor, &KColorButton::changed, this, &ColorsPage::referenceColorChanged);
    connect(m_ui.mathColor, &KColorButton::changed, this, &ColorsPage::mathColorChanged);
}

Ui::ColorsPage &ColorsPage::ui()
{
    return m_ui;
}

Colors &ColorsPage::colors()
{
    m_colors.m_codeTheme = m_ui.m_codeTheme->currentText();
    m_colors.m_codeThemeEnabled = m_ui.m_codeThemeGroupBox->isChecked();
    m_colors.m_drawCodeBackground = m_ui.m_drawCodeBackground->isChecked();

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

    m_ui.m_codeThemeGroupBox->setChecked(m_colors.m_codeThemeEnabled);
    m_ui.m_codeTheme->setCurrentText(m_colors.m_codeTheme);
    m_ui.m_drawCodeBackground->setChecked(m_colors.m_drawCodeBackground);
}

void ColorsPage::linkColorChanged(const QColor &c)
{
    m_colors.m_linkColor = c;
}

void ColorsPage::textColorChanged(const QColor &c)
{
    m_colors.m_textColor = c;
}

void ColorsPage::inlineColorChanged(const QColor &c)
{
    m_colors.m_inlineColor = c;
}

void ColorsPage::htmlColorChanged(const QColor &c)
{
    m_colors.m_htmlColor = c;
}

void ColorsPage::tableColorChanged(const QColor &c)
{
    m_colors.m_tableColor = c;
}

void ColorsPage::codeColorChanged(const QColor &c)
{
    m_colors.m_codeColor = c;
}

void ColorsPage::mathColorChanged(const QColor &c)
{
    m_colors.m_mathColor = c;
}

void ColorsPage::referenceColorChanged(const QColor &c)
{
    m_colors.m_referenceColor = c;
}

void ColorsPage::specialColorChanged(const QColor &c)
{
    m_colors.m_specialColor = c;
}

void ColorsPage::colorsToggled(bool on)
{
    m_colors.m_enabled = on;
}

void ColorsPage::initCodeThemes(QSharedPointer<MdShared::Syntax> syntax)
{
    QStringList themeNames;
    const auto themes = syntax->repository().themes();

    for (const auto &t : themes) {
        themeNames.push_back(t.name());
    }

    themeNames.sort(Qt::CaseInsensitive);

    m_ui.m_codeTheme->addItems(themeNames);
    m_ui.m_codeTheme->setCurrentText(QStringLiteral("GitHub Light"));
}

} /* namespace MdEditor */
