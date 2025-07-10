/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "settings.h"
#include "colors.h"

// Qt include.
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>

namespace MdEditor
{

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(const Colors &c, const QFont &f, const Margins &m, bool enableSpelling, QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_colorsPage->colors() = c;
    m_ui.m_colorsPage->applyColors();
    m_ui.m_fontPage->initWithFont(f);

    m_ui.m_rightMarginValue->setValue(m.m_length);
    m_ui.m_rightMarginValue->setEnabled(m.m_enable);
    m_ui.m_rightMargin->setChecked(m.m_enable);

    connect(m_ui.buttonBox, &QDialogButtonBox::clicked, this, &SettingsDlg::onButtonclicked);
    connect(m_ui.m_menu, &QListWidget::currentRowChanged, this, &SettingsDlg::onMenu);
    connect(m_ui.m_stack, &QStackedWidget::currentChanged, this, &SettingsDlg::onPageChanged);
    connect(m_ui.m_rightMargin, &QCheckBox::checkStateChanged, this, &SettingsDlg::onEnableRightMargin);

    m_ui.m_menu->item(0)->setIcon(QIcon::fromTheme(QStringLiteral("fill-color"), QIcon(QStringLiteral(":/res/img/fill-color.png"))));
    m_ui.m_menu->item(1)->setIcon(
        QIcon::fromTheme(QStringLiteral("preferences-desktop-font"), QIcon(QStringLiteral(":/res/img/preferences-desktop-font.png"))));
    m_ui.m_menu->item(2)->setIcon(QIcon::fromTheme(QStringLiteral("document-properties"), QIcon(QStringLiteral(":/res/img/document-properties.png"))));
    m_ui.m_menu->item(3)->setIcon(QIcon::fromTheme(QStringLiteral("preferences-plugin"), QIcon(QStringLiteral(":/res/img/preferences-plugin.png"))));

    m_ui.m_spellingGroup->setChecked(enableSpelling);
}

const Colors &SettingsDlg::colors() const
{
    return m_ui.m_colorsPage->colors();
}

QFont SettingsDlg::currentFont() const
{
    return m_ui.m_fontPage->currentFont();
}

void SettingsDlg::onPageChanged(int idx)
{
    if (static_cast<QAbstractButton *>(m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults)) != nullptr) {
        m_ui.buttonBox->removeButton(m_ui.buttonBox->button(QDialogButtonBox::StandardButton::RestoreDefaults));
    }

    if (idx == 0 || idx == 2) {
        m_ui.buttonBox->addButton(QDialogButtonBox::StandardButton::RestoreDefaults);
    }
}

void SettingsDlg::onButtonclicked(QAbstractButton *btn)
{
    if (static_cast<QAbstractButton *>(m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults)) == btn) {
        switch (m_ui.m_stack->currentIndex()) {
        case 0:
            m_ui.m_colorsPage->resetDefaults();
            break;

        case 2:
            m_ui.m_spellingConfig->slotDefault();
            break;

        default:
            break;
        }
    } else if (static_cast<QAbstractButton *>(m_ui.buttonBox->button(QDialogButtonBox::Ok)) == btn) {
        m_ui.m_spellingConfig->save();
    }
}

void SettingsDlg::onMenu(int idx)
{
    if (idx != -1) {
        m_ui.m_stack->setCurrentIndex(idx);
    }
}

Margins SettingsDlg::editorMargins() const
{
    Margins m;
    m.m_enable = m_ui.m_rightMargin->isChecked();
    m.m_length = m_ui.m_rightMarginValue->value();

    return m;
}

bool
SettingsDlg::isSpellingEnabled() const
{
    return m_ui.m_spellingGroup->isChecked();
}

Sonnet::ConfigWidget *
SettingsDlg::sonnetConfigWidget() const
{
    return m_ui.m_spellingConfig;
}

void SettingsDlg::onEnableRightMargin(Qt::CheckState st)
{
    m_ui.m_rightMarginValue->setEnabled(st == Qt::Checked);
}

} /* namespace MdEditor */
