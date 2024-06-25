/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "colorsdlg.hpp"
#include "ui_colorsdlg.h"

// Qt include.
#include <QDialogButtonBox>
#include <QPushButton>


namespace MdEditor {

//
// ColorsDialogPrivate
//

struct ColorsDialogPrivate {
	Ui::ColorsDialog ui;
}; // struct ColorsDialogPrivate

//
// ColorsDialog
//

ColorsDialog::ColorsDialog( const Colors & cols, QWidget * parent )
	:	QDialog( parent )
	,	d( new ColorsDialogPrivate )
{
	d->ui.setupUi( this );

	d->ui.m_page->colors() = cols;

	d->ui.m_page->applyColors();

	connect( d->ui.buttonBox, &QDialogButtonBox::clicked,
		this, &ColorsDialog::clicked );
}

ColorsDialog::~ColorsDialog()
{
}

const Colors &
ColorsDialog::colors() const
{
	return d->ui.m_page->colors();
}

void
ColorsDialog::clicked( QAbstractButton * btn )
{
	if( static_cast< QAbstractButton* > ( d->ui.buttonBox->button(
		QDialogButtonBox::RestoreDefaults ) ) == btn )
			d->ui.m_page->resetDefaults();
}

} /* namespace MdEditor */
