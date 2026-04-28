/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colorsdlg.h"

// Qt include.
#include <QDialogButtonBox>
#include <QPushButton>

namespace MdEditor
{

//
// ColorsDialogPrivate
//

struct ColorsDialogPrivate {
    ColorsDialogPrivate(ColorsDialog *parent)
        : m_q(parent)
        , m_page(new ColorsPage(m_q))
    {
    }

    ColorsDialog *m_q = nullptr;
    ColorsPage *m_page = nullptr;
}; // struct ColorsDialogPrivate

//
// ColorsDialog
//

ColorsDialog::ColorsDialog(const Colors &cols,
                           QSharedPointer<MdShared::Syntax> syntax,
                           QWidget *parent)
    : MdShared::DlgWheelFilter(parent)
    , m_d(new ColorsDialogPrivate(this))
{
    setWindowTitle(tr("Colors"));

    m_d->m_page->colors() = cols;

    m_d->m_page->initCodeThemes(syntax);

    m_d->m_page->applyColors();

    addPage(m_d->m_page, {});

    buttonBox()->addButton(QDialogButtonBox::RestoreDefaults);

    connect(buttonBox(), &QDialogButtonBox::clicked, this, &ColorsDialog::clicked);

    installFilterForChildren(this);

    auto s = m_d->m_page->ui().m_scrollAreaWidgetContents->sizeHint();
    m_d->m_page->ui().m_scrollAreaWidgetContents->setMinimumWidth(s.width());
    m_d->m_page->ui().m_scrollAreaWidgetContents->setMinimumHeight(s.height());
    s.setHeight(s.height()
                + m_d->m_page->layout()->contentsMargins().top()
                + m_d->m_page->layout()->contentsMargins().bottom());
    s.setWidth(
        s.width() + m_d->m_page->layout()->contentsMargins().left() + m_d->m_page->layout()->contentsMargins().right());
    m_d->m_page->setMinimumWidth(s.width());
    m_d->m_page->setMinimumHeight(s.height());
}

ColorsDialog::~ColorsDialog()
{
}

const Colors &ColorsDialog::colors() const
{
    return m_d->m_page->colors();
}

void ColorsDialog::clicked(QAbstractButton *btn)
{
    if (static_cast<QAbstractButton *>(buttonBox()->button(QDialogButtonBox::RestoreDefaults)) == btn) {
        m_d->m_page->resetDefaults();
    }
}

} /* namespace MdEditor */
