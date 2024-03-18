/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "fontdlg.hpp"
#include "ui_fontdlg.h"


namespace MdEditor {

//
// FontDlgPrivate
//

struct FontDlgPrivate {
	FontDlgPrivate( FontDlg * parent )
		:	q( parent )
	{
	}

	void initUi( const QFont & font )
	{
		ui.setupUi( q );

		ui.fontComboBox->setCurrentFont( font );
		ui.fontSize->setValue( font.pointSize() );

		QObject::connect( ui.monospacedCheckBox, &QCheckBox::stateChanged,
			q, &FontDlg::onShowOnlyMonospaced );
	}

	FontDlg * q;
	Ui::FontDlg ui;
}; // struct FontDlgPrivate


//
// FontDlg
//

FontDlg::FontDlg( const QFont & f, QWidget * parent )
	:	QDialog( parent )
	,	d( new FontDlgPrivate( this ) )
{
	d->initUi( f );
}

FontDlg::~FontDlg()
{
}

QFont
FontDlg::font() const
{
	QFont f = d->ui.fontComboBox->currentFont();
	f.setPointSize( d->ui.fontSize->value() );

	return f;
}

void
FontDlg::onShowOnlyMonospaced( int state )
{
	if( state == Qt::Checked )
		d->ui.fontComboBox->setFontFilters( QFontComboBox::MonospacedFonts );
	else
		d->ui.fontComboBox->setFontFilters( QFontComboBox::AllFonts );
}

} /* namespace MdEditor */
