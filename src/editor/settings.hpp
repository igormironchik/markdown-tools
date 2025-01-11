/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QFont>

// md-editor include.
#include "colorsdlg.hpp"
#include "editor.hpp"
#include "ui_settings.h"

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
    SettingsDlg(const Colors &c, const QFont &f, const Margins &m, bool enableSpelling, QWidget *parent);
    ~SettingsDlg() override = default;

    const Colors &colors() const;
    QFont currentFont() const;
    Margins editorMargins() const;
    bool isSpellingEnabled() const;

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
