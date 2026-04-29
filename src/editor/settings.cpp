/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "settings.h"
#include "colors.h"
#include "editor_settings.h"
#include "font.h"

// md-shared include.
#include "plugins_page.h"

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
            || s1.m_isAutoCodeBlocksEnabled != s2.m_isAutoCodeBlocksEnabled
            || s1.m_isLinksAutoCompletionEnabled != s2.m_isLinksAutoCompletionEnabled
            || s1.m_isEmojiAutoCompletionEnabled != s2.m_isEmojiAutoCompletionEnabled);
}

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(const Settings &s,
                         QSharedPointer<MdShared::Syntax> syntax,
                         QWidget *parent)
    : MdShared::DlgWheelFilter(parent)
    , m_editorPage(new EditorSettingsPage(this))
    , m_colorsPage(new ColorsPage(this))
    , m_fontPage(new FontPage(this))
    , m_pluginsPage(new MdShared::PluginsPage(this))
{
    m_colorsPage->initCodeThemes(syntax);
    m_colorsPage->colors() = s.m_colors;
    m_colorsPage->applyColors();
    m_fontPage->initWithFont(s.m_font);

    m_editorPage->ui().m_rightMarginValue->setValue(s.m_margins.m_length);
    m_editorPage->ui().m_rightMarginValue->setEnabled(s.m_margins.m_enable);
    m_editorPage->ui().m_rightMargin->setChecked(s.m_margins.m_enable);
    m_editorPage->ui().m_tabsMode->setCurrentIndex(s.m_indentMode == Editor::IndentMode::Tabs ? 0 : 1);
    m_editorPage->ui().m_spacesAmount->setValue(s.m_indentSpacesCount);
    m_editorPage->ui().m_spacesAmount->setEnabled(s.m_indentMode != Editor::IndentMode::Tabs);
    m_editorPage->ui().m_autoListGroupBox->setChecked(s.m_isAutoListsEnabled);
    m_editorPage->ui().m_dontUseAutoListInCodeBlock->setChecked(s.m_dontUseAutoListInCodeBlock);
    m_editorPage->ui().m_githubBehaviour->setChecked(s.m_githubBehaviour);
    m_editorPage->ui().m_autoFormatCodeBlocks->setChecked(s.m_isAutoCodeBlocksEnabled);
    m_editorPage->ui().m_autoLinks->setChecked(s.m_isLinksAutoCompletionEnabled);
    m_editorPage->ui().m_autoEmoji->setChecked(s.m_isEmojiAutoCompletionEnabled);

    connect(buttonBox(), &QDialogButtonBox::clicked, this, &SettingsDlg::onButtonclicked);
    connect(this, &KPageDialog::currentPageChanged, this, &SettingsDlg::onPageChanged);

    m_editorPage->ui().m_spellingGroup->setChecked(s.m_enableSpelling);

    m_pluginsPage->setCfg(s.m_pluginsCfg);

    installFilterForChildren(this);

    m_colorsPageItem = addPage(m_colorsPage, tr("Colors"));
    auto fontPageItem = addPage(m_fontPage, tr("Font"));
    m_editorPageItem = addPage(m_editorPage, tr("Editor"));
    auto pluginsPageItem = addPage(m_pluginsPage, tr("Plugins"));

    m_colorsPageItem->setIcon(
        QIcon::fromTheme(QStringLiteral("fill-color"), QIcon(QStringLiteral(":/res/img/fill-color.png"))));
    fontPageItem->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-font"),
                                           QIcon(QStringLiteral(":/res/img/preferences-desktop-font.png"))));
    m_editorPageItem->setIcon(QIcon::fromTheme(QStringLiteral("document-properties"),
                                               QIcon(QStringLiteral(":/res/img/document-properties.png"))));
    pluginsPageItem->setIcon(QIcon::fromTheme(QStringLiteral("preferences-plugin"),
                                              QIcon(QStringLiteral(":/res/img/preferences-plugin.png"))));

    setFaceType(KPageDialog::List);
}

Settings SettingsDlg::settings() const
{
    Settings s;

    s.m_colors = m_colorsPage->colors();
    s.m_font = m_fontPage->currentFont();
    s.m_margins.m_enable = m_editorPage->ui().m_rightMargin->isChecked();
    s.m_margins.m_length = m_editorPage->ui().m_rightMarginValue->value();
    s.m_enableSpelling = m_editorPage->ui().m_spellingGroup->isChecked();
    s.m_pluginsCfg = m_pluginsPage->cfg();
    s.m_indentMode =
        (m_editorPage->ui().m_tabsMode->currentIndex() == 0 ? Editor::IndentMode::Tabs : Editor::IndentMode::Spaces);
    s.m_indentSpacesCount = m_editorPage->ui().m_spacesAmount->value();
    s.m_isAutoListsEnabled = m_editorPage->ui().m_autoListGroupBox->isChecked();
    s.m_githubBehaviour = m_editorPage->ui().m_githubBehaviour->isChecked();
    s.m_dontUseAutoListInCodeBlock = m_editorPage->ui().m_dontUseAutoListInCodeBlock->isChecked();
    s.m_isAutoCodeBlocksEnabled = m_editorPage->ui().m_autoFormatCodeBlocks->isChecked();
    s.m_isLinksAutoCompletionEnabled = m_editorPage->ui().m_autoLinks->isChecked();
    s.m_isEmojiAutoCompletionEnabled = m_editorPage->ui().m_autoEmoji->isChecked();

    return s;
}

void SettingsDlg::onPageChanged(KPageWidgetItem *current,
                                KPageWidgetItem *)
{
    if (static_cast<QAbstractButton *>(buttonBox()->button(QDialogButtonBox::RestoreDefaults)) != nullptr) {
        buttonBox()->removeButton(buttonBox()->button(QDialogButtonBox::StandardButton::RestoreDefaults));
    }

    if (current == m_colorsPageItem || current == m_editorPageItem) {
        buttonBox()->addButton(QDialogButtonBox::StandardButton::RestoreDefaults);
    }
}

void SettingsDlg::onButtonclicked(QAbstractButton *btn)
{
    if (static_cast<QAbstractButton *>(buttonBox()->button(QDialogButtonBox::RestoreDefaults)) == btn) {
        if (currentPage() == m_colorsPageItem) {
            m_colorsPage->resetDefaults();
        } else if (currentPage() == m_editorPageItem) {
            m_editorPage->ui().m_spellingConfig->slotDefault();
        }
    } else if (static_cast<QAbstractButton *>(buttonBox()->button(QDialogButtonBox::Ok)) == btn) {
        m_editorPage->ui().m_spellingConfig->save();
    }
}

Sonnet::ConfigWidget *SettingsDlg::sonnetConfigWidget() const
{
    return m_editorPage->ui().m_spellingConfig;
}

} /* namespace MdEditor */
