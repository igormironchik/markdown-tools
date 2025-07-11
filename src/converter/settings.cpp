/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "settings.h"

namespace MdPdf
{

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(const MdShared::PluginsCfg &pluginsCfg, QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_pluginsPage->setCfg(pluginsCfg);
}

SettingsDlg::~SettingsDlg()
{
}

MdShared::PluginsCfg SettingsDlg::pluginsCfg() const
{
    return m_ui.m_pluginsPage->cfg();
}

} /* namespace MdPdf */
