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

bool operator!=(const Settings &s1,
                const Settings &s2)
{
    return (s1.m_colors != s2.m_colors
            || s1.m_font != s2.m_font
            || s1.m_margins != s2.m_margins
            || s1.m_enableSpelling != s2.m_enableSpelling
            || s1.m_pluginsCfg != s2.m_pluginsCfg
            || s1.m_indentMode != s2.m_indentMode
            || s1.m_indentSpacesCount != s2.m_indentSpacesCount
            || s1.m_isAutoListsEnabled != s2.m_isAutoListsEnabled
            || s1.m_githubBehaviour != s2.m_githubBehaviour
            || s1.m_dontUseAutoListInCodeBlock != s2.m_dontUseAutoListInCodeBlock
            || s1.m_isAutoCodeBlocksEnabled != s2.m_isAutoCodeBlocksEnabled);
}

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(const Settings &s,
                         std::shared_ptr<MdShared::Syntax> syntax,
                         QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_colorsPage->initCodeThemes(syntax);
    m_ui.m_colorsPage->colors() = s.m_colors;
    m_ui.m_colorsPage->applyColors();
    m_ui.m_fontPage->initWithFont(s.m_font);

    m_ui.m_rightMarginValue->setValue(s.m_margins.m_length);
    m_ui.m_rightMarginValue->setEnabled(s.m_margins.m_enable);
    m_ui.m_rightMargin->setChecked(s.m_margins.m_enable);
    m_ui.m_tabsMode->setCurrentIndex(s.m_indentMode == Editor::IndentMode::Tabs ? 0 : 1);
    m_ui.m_spacesAmount->setValue(s.m_indentSpacesCount);
    m_ui.m_spacesAmount->setEnabled(s.m_indentMode != Editor::IndentMode::Tabs);
    m_ui.m_autoListGroupBox->setChecked(s.m_isAutoListsEnabled);
    m_ui.m_dontUseAutoListInCodeBlock->setChecked(s.m_dontUseAutoListInCodeBlock);
    m_ui.m_githubBehaviour->setChecked(s.m_githubBehaviour);
    m_ui.m_autoFormatCodeBlocks->setChecked(s.m_isAutoCodeBlocksEnabled);

    connect(m_ui.buttonBox, &QDialogButtonBox::clicked, this, &SettingsDlg::onButtonclicked);
    connect(m_ui.m_menu, &QListWidget::currentRowChanged, this, &SettingsDlg::onMenu);
    connect(m_ui.m_stack, &QStackedWidget::currentChanged, this, &SettingsDlg::onPageChanged);
    connect(m_ui.m_rightMargin, &QCheckBox::checkStateChanged, this, &SettingsDlg::onEnableRightMargin);
    connect(m_ui.m_tabsMode, &QComboBox::currentIndexChanged, this, &SettingsDlg::onIndentModeChanged);

    m_ui.m_menu->item(0)->setIcon(
        QIcon::fromTheme(QStringLiteral("fill-color"), QIcon(QStringLiteral(":/res/img/fill-color.png"))));
    m_ui.m_menu->item(1)->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font"),
                                                   QIcon(QStringLiteral(":/res/img/preferences-desktop-font.png"))));
    m_ui.m_menu->item(2)->setIcon(QIcon::fromTheme(QStringLiteral("document-properties"),
                                                   QIcon(QStringLiteral(":/res/img/document-properties.png"))));
    m_ui.m_menu->item(3)->setIcon(QIcon::fromTheme(QStringLiteral("preferences-plugin"),
                                                   QIcon(QStringLiteral(":/res/img/preferences-plugin.png"))));

    m_ui.m_spellingGroup->setChecked(s.m_enableSpelling);

    m_ui.m_pluginsPage->setCfg(s.m_pluginsCfg);
}

Settings SettingsDlg::settings() const
{
    Settings s;

    s.m_colors = m_ui.m_colorsPage->colors();
    s.m_font = m_ui.m_fontPage->currentFont();
    s.m_margins.m_enable = m_ui.m_rightMargin->isChecked();
    s.m_margins.m_length = m_ui.m_rightMarginValue->value();
    s.m_enableSpelling = m_ui.m_spellingGroup->isChecked();
    s.m_pluginsCfg = m_ui.m_pluginsPage->cfg();
    s.m_indentMode = (m_ui.m_tabsMode->currentIndex() == 0 ? Editor::IndentMode::Tabs : Editor::IndentMode::Spaces);
    s.m_indentSpacesCount = m_ui.m_spacesAmount->value();
    s.m_isAutoListsEnabled = m_ui.m_autoListGroupBox->isChecked();
    s.m_githubBehaviour = m_ui.m_githubBehaviour->isChecked();
    s.m_dontUseAutoListInCodeBlock = m_ui.m_dontUseAutoListInCodeBlock->isChecked();
    s.m_isAutoCodeBlocksEnabled = m_ui.m_autoFormatCodeBlocks->isChecked();

    return s;
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

Sonnet::ConfigWidget *SettingsDlg::sonnetConfigWidget() const
{
    return m_ui.m_spellingConfig;
}

void SettingsDlg::onEnableRightMargin(Qt::CheckState st)
{
    m_ui.m_rightMarginValue->setEnabled(st == Qt::Checked);
}

void SettingsDlg::onIndentModeChanged(int index)
{
    m_ui.m_spacesAmount->setEnabled(index != 0);
}

} /* namespace MdEditor */
