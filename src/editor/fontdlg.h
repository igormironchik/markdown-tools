/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QFont>
#include <QScopedPointer>

#include "ui_fontdlg.h"

namespace MdEditor
{

//
// FontDlg
//

//! Font dialog.
class FontDlg : public QDialog
{
    Q_OBJECT

public:
    FontDlg(const QFont &f, QWidget *parent);
    ~FontDlg() override = default;

    //! \return Current font.
    QFont currentFont() const;

private:
    Q_DISABLE_COPY(FontDlg)

    //! UI component.
    Ui::FontDlg m_ui;
}; // class FontDlg

} /* namespace MdEditor */
