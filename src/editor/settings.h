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

namespace Sonnet
{
class ConfigWidget;
}

namespace MdEditor
{

//
// SettingsDlg
//

//! Settings dialog.
class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    SettingsDlg(const Colors &c, const QFont &f, const Margins &m, bool enableSpelling,
                MdShared::PluginsCfg &pCfg, QWidget *parent);
    ~SettingsDlg() override = default;

    //! \return Colors scheme.
    const Colors &colors() const;
    //! \return Editor's font.
    QFont currentFont() const;
    //! \return Grayed editor's area settings.
    Margins editorMargins() const;
    //! \return Is spelling check enabled?
    bool isSpellingEnabled() const;
    //! \return Plugins configuration.
    MdShared::PluginsCfg pluginsCfg() const;

    //! \return Sonnet configuration widget.
    Sonnet::ConfigWidget *sonnetConfigWidget() const;

private slots:
    void onPageChanged(int idx);
    void onButtonclicked(QAbstractButton *btn);
    void onMenu(int idx);
    void onEnableRightMargin(Qt::CheckState st);

private:
    Q_DISABLE_COPY(SettingsDlg)

    Ui::SettingsDlg m_ui;
}; // class SettingsDlg

} /* namespace MdEditor */
