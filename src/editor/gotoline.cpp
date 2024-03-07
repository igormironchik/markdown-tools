
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
#include "gotoline.hpp"
#include "ui_gotoline.h"
#include "editor.hpp"
#include "mainwindow.hpp"

// Qt include.
#include <QIntValidator>


namespace MdEditor {

//
// GoToLinePrivate
//

struct GoToLinePrivate {
	GoToLinePrivate( MainWindow * w, Editor * e, GoToLine * parent )
		:	q( parent )
		,	editor( e )
		,	window( w )
	{
	}

	void initUi()
	{
		ui.setupUi( q );

		auto intValidator = new QIntValidator( 1, 999999, ui.line );
		ui.line->setValidator( intValidator );

		QObject::connect( ui.line, &QLineEdit::returnPressed,
			q, &GoToLine::onEditingFinished );

		QObject::connect( ui.close, &QAbstractButton::clicked,
			q, &GoToLine::onClose );
	}

	GoToLine * q = nullptr;
	Editor * editor = nullptr;
	MainWindow * window = nullptr;
	Ui::GoToLine ui;
}; // struct FindPrivate


//
// GoToLine
//

GoToLine::GoToLine( MainWindow * window, Editor * editor, QWidget * parent )
	:	QFrame( parent )
	,	d( new GoToLinePrivate( window, editor, this ) )
{
	d->initUi();
}

GoToLine::~GoToLine()
{
}

QLineEdit *
GoToLine::line() const
{
	return d->ui.line;
}

void
GoToLine::setFocusOnLine()
{
	d->ui.line->setFocus();
	d->ui.line->selectAll();
}

void
GoToLine::onEditingFinished()
{
	d->editor->goToLine( d->ui.line->text().toInt() );

	hide();
}

void
GoToLine::onClose()
{
	hide();

	d->window->onToolHide();
}

} /* namespace MdEditor */
