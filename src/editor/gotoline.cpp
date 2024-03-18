/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
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
