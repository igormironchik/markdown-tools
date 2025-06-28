/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "font.h"

// Qt include.
#include <QCheckBox>

namespace MdEditor
{

//
// FontPage
//

FontPage::FontPage(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    QObject::connect(m_ui.monospacedCheckBox, &QCheckBox::checkStateChanged, this, &FontPage::onShowOnlyMonospaced);
}

Ui::FontPage &FontPage::ui()
{
    return m_ui;
}

QFont FontPage::currentFont() const
{
    QFont f = m_ui.fontComboBox->currentFont();
    f.setPointSize(m_ui.fontSize->value());

    return f;
}

void FontPage::initWithFont(const QFont &f)
{
    m_ui.fontComboBox->setCurrentFont(f);
    m_ui.fontSize->setValue(f.pointSize());
}

void FontPage::onShowOnlyMonospaced(Qt::CheckState state)
{
    if (state == Qt::Checked) {
        m_ui.fontComboBox->setFontFilters(QFontComboBox::MonospacedFonts);
    } else {
        m_ui.fontComboBox->setFontFilters(QFontComboBox::AllFonts);
    }
}

} /* namespace MdEditor */
