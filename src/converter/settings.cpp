/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "settings.h"

namespace MdPdf
{

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(const MdShared::PluginsCfg &pluginsCfg,
                         const QColor &markColor,
                         QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_pluginsPage->setCfg(pluginsCfg);
    m_ui.m_markColor->setColor(markColor);

    connect(m_ui.m_markColor, &KColorButton::changed, this, &SettingsDlg::markColorChanged);
}

SettingsDlg::~SettingsDlg()
{
}

MdShared::PluginsCfg SettingsDlg::pluginsCfg() const
{
    return m_ui.m_pluginsPage->cfg();
}

const QColor &SettingsDlg::markColor() const
{
    return m_ui.m_markColor->color();
}

void SettingsDlg::markColorChanged(const QColor &c)
{
    m_ui.m_markColor->setColor(c);
}

} /* namespace MdPdf */
