
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2023-2024 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
