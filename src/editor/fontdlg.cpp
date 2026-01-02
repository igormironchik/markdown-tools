/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "fontdlg.h"

namespace MdEditor
{

//
// FontDlg
//

FontDlg::FontDlg(const QFont &f,
                 QWidget *parent)
    : MdShared::DlgWheelFilter(parent)
{
    m_ui.setupUi(this);
    m_ui.m_page->initWithFont(f);

    adjustSize();

    installFilterForChildren(this);
}

QFont FontDlg::currentFont() const
{
    return m_ui.m_page->currentFont();
}

} /* namespace MdEditor */
