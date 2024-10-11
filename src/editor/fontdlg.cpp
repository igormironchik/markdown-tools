/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "fontdlg.hpp"

namespace MdEditor
{

//
// FontDlg
//

FontDlg::FontDlg(const QFont &f, QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_ui.m_page->initWithFont(f);

    adjustSize();
}

QFont FontDlg::currentFont() const
{
    return m_ui.m_page->currentFont();
}

} /* namespace MdEditor */
