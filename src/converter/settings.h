/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>

// shared include.
#include "plugins_page.h"

// md-pdf include.
#include "ui_settings.h"

namespace MdPdf
{

//
// SettingsDlg
//

class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDlg(const MdShared::PluginsCfg &pluginsCfg,
                         const QColor &markColor,
                         QWidget *parent = nullptr);
    ~SettingsDlg() override;

    MdShared::PluginsCfg pluginsCfg() const;
    const QColor &markColor() const;

private slots:
    void onMarkColorChoose();

private:
    Ui::SettingsDlg m_ui;
}; // class SettingsDlg

} /* namespace MdPdf */
