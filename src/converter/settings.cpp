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

    auto s = m_ui.m_pluginsPage->ui().m_scrollAreaWidgetContents->sizeHint();
    m_ui.m_pluginsPage->ui().m_scrollAreaWidgetContents->setMinimumWidth(s.width());
    m_ui.m_pluginsPage->ui().m_scrollAreaWidgetContents->setMinimumHeight(s.height());
    s.setHeight(s.height()
                + m_ui.m_pluginsPage->layout()->contentsMargins().top()
                + m_ui.m_pluginsPage->layout()->contentsMargins().bottom());
    s.setWidth(s.width()
               + m_ui.m_pluginsPage->layout()->contentsMargins().left()
               + m_ui.m_pluginsPage->layout()->contentsMargins().right());
    m_ui.m_pluginsPage->setMinimumWidth(s.width());
    m_ui.m_pluginsPage->setMinimumHeight(s.height());
}

SettingsDlg::~SettingsDlg()
{
}

MdShared::PluginsCfg SettingsDlg::pluginsCfg() const
{
    return m_ui.m_pluginsPage->cfg();
}

QColor SettingsDlg::markColor() const
{
    return m_ui.m_markColor->color();
}

} /* namespace MdPdf */
