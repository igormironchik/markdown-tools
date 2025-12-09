/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colorsdlg.h"
#include "ui_colorsdlg.h"

// Qt include.
#include <QDialogButtonBox>
#include <QPushButton>

namespace MdEditor
{

//
// ColorsDialogPrivate
//

struct ColorsDialogPrivate {
    Ui::ColorsDialog m_ui;
}; // struct ColorsDialogPrivate

//
// ColorsDialog
//

ColorsDialog::ColorsDialog(const Colors &cols,
                           QSharedPointer<MdShared::Syntax> syntax,
                           QWidget *parent)
    : MdShared::DlgWheelFilter(parent)
    , m_d(new ColorsDialogPrivate)
{
    m_d->m_ui.setupUi(this);

    m_d->m_ui.m_page->colors() = cols;

    m_d->m_ui.m_page->initCodeThemes(syntax);

    m_d->m_ui.m_page->applyColors();

    connect(m_d->m_ui.buttonBox, &QDialogButtonBox::clicked, this, &ColorsDialog::clicked);

    installFilterForChildren(this);
}

ColorsDialog::~ColorsDialog()
{
}

const Colors &ColorsDialog::colors() const
{
    return m_d->m_ui.m_page->colors();
}

void ColorsDialog::clicked(QAbstractButton *btn)
{
    if (static_cast<QAbstractButton *>(m_d->m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults)) == btn) {
        m_d->m_ui.m_page->resetDefaults();
    }
}

} /* namespace MdEditor */
