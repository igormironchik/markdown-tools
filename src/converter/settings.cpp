/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "settings.h"

// Qt include.
#include <QColorDialog>

namespace MdPdf
{

//
// SettingsDlg
//

SettingsDlg::SettingsDlg(const MdShared::PluginsCfg &pluginsCfg, const QColor &markColor, QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_ui.m_pluginsPage->setCfg(pluginsCfg);
    m_ui.m_markColor->setColor(markColor);

    connect(m_ui.m_markColor, &MdShared::ColorWidget::clicked, this, &SettingsDlg::onMarkColorChoose);
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

void SettingsDlg::onMarkColorChoose()
{
    QColorDialog dlg(m_ui.m_markColor->color(), this);

    if (QDialog::Accepted == dlg.exec()) {
        m_ui.m_markColor->setColor(dlg.currentColor());
    }
}

} /* namespace MdPdf */
