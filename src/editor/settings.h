/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QFont>

// md-editor include.
#include "colorsdlg.h"
#include "editor.h"

// shared include.
#include "dlg_filter_wheel.h"
#include "syntax.h"

class KPageWidgetItem;

namespace Sonnet
{
class ConfigWidget;
}

namespace MdShared
{
class PluginsPage;
}

namespace MdEditor
{

class EditorSettingsPage;
class FontPage;

//
// Settings
//

//! Settings of the editor application.
struct Settings {
    //! Highlighting colors.
    Colors m_colors;
    //! Editor's font.
    QFont m_font;
    //! Grayed area in editor settings.
    Margins m_margins;
    //! Is pelling enabled?
    bool m_enableSpelling = true;
    //! Configuration of plugins.
    MdShared::PluginsCfg m_pluginsCfg;
    //! Indent mode of the editor.
    Editor::IndentMode m_indentMode = Editor::IndentMode::Tabs;
    //! Count of spaces for indent in editor.
    int m_indentSpacesCount = 2;
    //! Is auto-list continuation enabled?
    bool m_isAutoListsEnabled = true;
    //! Don't auto add list after non-first block of list item (like on GitHub).
    bool m_githubBehaviour = false;
    //! Don't auto add list in code block in list item.
    bool m_dontUseAutoListInCodeBlock = true;
    //! Auto continue code blocks.
    bool m_isAutoCodeBlocksEnabled = true;
    //! Is auto-completion of internal links enabled?
    bool m_isLinksAutoCompletionEnabled = true;
    //! Is auto-completion of Emojies enabled?
    bool m_isEmojiAutoCompletionEnabled = true;
}; // struct Settings

bool operator!=(const Settings &s1,
                const Settings &s2);

//
// SettingsDlg
//

//! Settings dialog.
class SettingsDlg : public MdShared::DlgWheelFilter
{
    Q_OBJECT

public:
    SettingsDlg(const Settings &s,
                QSharedPointer<MdShared::Syntax> syntax,
                QWidget *parent);
    ~SettingsDlg() override = default;

    //! \return Settings.
    Settings settings() const;

    //! \return Sonnet configuration widget.
    Sonnet::ConfigWidget *sonnetConfigWidget() const;

private slots:
    void onPageChanged(KPageWidgetItem *current, KPageWidgetItem *before);
    void onButtonclicked(QAbstractButton *btn);

private:
    Q_DISABLE_COPY(SettingsDlg)

    EditorSettingsPage *m_editorPage = nullptr;
    ColorsPage *m_colorsPage = nullptr;
    FontPage *m_fontPage = nullptr;
    MdShared::PluginsPage *m_pluginsPage = nullptr;
    KPageWidgetItem *m_colorsPageItem = nullptr;
    KPageWidgetItem *m_editorPageItem = nullptr;
}; // class SettingsDlg

} /* namespace MdEditor */
