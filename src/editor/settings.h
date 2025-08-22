/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QFont>

// md-editor include.
#include "colorsdlg.h"
#include "editor.h"
#include "ui_settings.h"

// shared include.
#include "syntax.h"

namespace Sonnet
{
class ConfigWidget;
}

namespace MdEditor
{

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
}; // struct Settings

bool operator!=(const Settings &s1,
                const Settings &s2);

//
// SettingsDlg
//

//! Settings dialog.
class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    SettingsDlg(const Settings &s,
                std::shared_ptr<MdShared::Syntax> syntax,
                QWidget *parent);
    ~SettingsDlg() override = default;

    //! \return Settings.
    Settings settings() const;

    //! \return Sonnet configuration widget.
    Sonnet::ConfigWidget *sonnetConfigWidget() const;

private slots:
    void onPageChanged(int idx);
    void onButtonclicked(QAbstractButton *btn);
    void onMenu(int idx);
    void onEnableRightMargin(Qt::CheckState st);
    void onIndentModeChanged(int index);

private:
    Q_DISABLE_COPY(SettingsDlg)

    Ui::SettingsDlg m_ui;
}; // class SettingsDlg

} /* namespace MdEditor */
