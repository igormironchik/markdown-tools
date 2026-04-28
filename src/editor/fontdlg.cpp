/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "fontdlg.h"
#include "font.h"

// Qt include.
#include <QSize>

namespace MdEditor
{

//
// FontDlg
//

FontDlg::FontDlg(const QFont &f,
                 QWidget *parent)
    : MdShared::DlgWheelFilter(parent)
{
    setWindowTitle(tr("Font"));

    m_page = new FontPage(this);
    addPage(m_page, {});
    m_page->initWithFont(f);

    setFaceType(KPageDialog::Plain);

    installFilterForChildren(this);

    auto s = m_page->ui().m_scrollAreaWidgetContents->sizeHint();
    m_page->ui().m_scrollAreaWidgetContents->setMinimumWidth(s.width());
    m_page->ui().m_scrollAreaWidgetContents->setMinimumHeight(s.height());
    s.setHeight(s.height() + m_page->layout()->contentsMargins().top() + m_page->layout()->contentsMargins().bottom());
    s.setWidth(s.width() + m_page->layout()->contentsMargins().left() + m_page->layout()->contentsMargins().right());
    m_page->setMinimumWidth(s.width());
    m_page->setMinimumHeight(s.height());
}

QFont FontDlg::currentFont() const
{
    return m_page->currentFont();
}

} /* namespace MdEditor */
