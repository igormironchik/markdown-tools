
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
#include "findweb.hpp"
#include "ui_findweb.h"
#include "mainwindow.hpp"
#include "webview.hpp"

// Qt include.
#include <QWebEngineFindTextResult>
#include <QPalette>


namespace MdEditor {

//
// FindWebPrivate
//

struct FindWebPrivate {
	FindWebPrivate( MainWindow * w, WebView * wb, FindWeb * parent )
		:	q( parent )
		,	web( wb )
		,	window( w )
	{
	}

	void initUi()
	{
		ui.setupUi( q );

		QObject::connect( ui.findEdit, &QLineEdit::textChanged,
			q, &FindWeb::onFindWebTextChanged );

		auto findPrevAction = new QAction( FindWeb::tr( "Find Previous" ), q );
		findPrevAction->setShortcutContext( Qt::ApplicationShortcut );
		findPrevAction->setShortcut( FindWeb::tr( "Shift+F4" ) );
		findPrevAction->setToolTip( FindWeb::tr( "Find Previous <small>Shift+F4</small>" ) );
		ui.findPrevBtn->setDefaultAction( findPrevAction );
		ui.findPrevBtn->setEnabled( false );

		auto findNextAction = new QAction( FindWeb::tr( "Find Next" ), q );
		findNextAction->setShortcutContext( Qt::ApplicationShortcut );
		findNextAction->setShortcut( FindWeb::tr( "F4" ) );
		findNextAction->setToolTip( FindWeb::tr( "Find Next <small>F4</small>" ) );
		ui.findNextBtn->setDefaultAction( findNextAction );
		ui.findNextBtn->setEnabled( false );

		textColor = ui.findEdit->palette().color( QPalette::Text );

		QObject::connect( findPrevAction, &QAction::triggered,
			q, &FindWeb::onFindPrev );
		QObject::connect( findNextAction, &QAction::triggered,
			q, &FindWeb::onFindNext );
		QObject::connect( ui.close, &QAbstractButton::clicked,
			q, &FindWeb::onClose );
	}

	FindWeb * q = nullptr;
	WebView * web = nullptr;
	MainWindow * window = nullptr;
	QColor textColor;
	Ui::FindWeb ui;
}; // struct FindWebPrivate


//
// FindWeb
//

FindWeb::FindWeb( MainWindow * window, WebView * web, QWidget * parent )
	:	QFrame( parent )
	,	d( new FindWebPrivate( window, web, this ) )
{
	d->initUi();
}

FindWeb::~FindWeb()
{
}

QLineEdit *
FindWeb::line() const
{
	return d->ui.findEdit;
}

void
FindWeb::onFindWebTextChanged( const QString & str )
{
	auto result = [&]( const QWebEngineFindTextResult & r )
	{
		QColor c = d->textColor;

		if( !r.numberOfMatches() )
			c = Qt::red;

		d->ui.findNextBtn->setEnabled( r.numberOfMatches() );
		d->ui.findPrevBtn->setEnabled( r.numberOfMatches() );
		d->ui.findNextBtn->defaultAction()->setEnabled( r.numberOfMatches() );
		d->ui.findPrevBtn->defaultAction()->setEnabled( r.numberOfMatches() );

		QPalette palette = d->ui.findEdit->palette();
		palette.setColor( QPalette::Text, c );
		d->ui.findEdit->setPalette( palette );
	};

	d->web->findText( str, QWebEnginePage::FindCaseSensitively, result );
}

void
FindWeb::setFindWebText( const QString & text )
{
	d->ui.findEdit->setText( text );

	setFocusOnFindWeb();
}

void
FindWeb::setFocusOnFindWeb()
{
	d->ui.findEdit->setFocus();
	d->ui.findEdit->selectAll();
	d->web->findText( d->ui.findEdit->text(), QWebEnginePage::FindCaseSensitively );
}

void
FindWeb::onClose()
{
	hide();

	d->window->onToolHide();
}

void
FindWeb::onFindNext()
{
	d->web->findText( d->ui.findEdit->text(), QWebEnginePage::FindCaseSensitively );
}

void
FindWeb::onFindPrev()
{
	d->web->findText( d->ui.findEdit->text(), QWebEnginePage::FindBackward |
		QWebEnginePage::FindCaseSensitively );
}

void
FindWeb::hideEvent( QHideEvent * event )
{
	d->web->findText( QString() );
	d->web->setFocus();
	d->web->triggerPageAction( QWebEnginePage::Unselect, true );

	QFrame::hideEvent( event );
}

} /* namespace MdEditor */
