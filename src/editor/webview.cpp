
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
#include "webview.hpp"

// Qt include.
#include <QContextMenuEvent>
#include <QApplication>
#include <QClipboard>


namespace MdEditor {

//
// WebViewPrivate
//

struct WebViewPrivate {
	WebViewPrivate( WebView * parent )
		:	q( parent )
	{
	}

	void initUi()
	{
		copyAction = new QAction( WebView::tr( "Copy" ), q );
		q->addAction( copyAction );
		copyAction->setEnabled( false );
		q->setContextMenuPolicy( Qt::ActionsContextMenu );
		q->setFocusPolicy( Qt::ClickFocus );

		QObject::connect( copyAction, &QAction::triggered,
			q, &WebView::onCopy );
		QObject::connect( q, &QWebEngineView::selectionChanged,
			q, &WebView::onSelectionChanged );
	}

	WebView * q;
	QAction * copyAction = nullptr;
}; // struct WebViewPrivate


//
// WebView
//

WebView::WebView( QWidget * parent )
	:	QWebEngineView( parent )
	,	d( new WebViewPrivate( this ) )
{
	d->initUi();
}

WebView::~WebView()
{
}

void
WebView::onSelectionChanged()
{
	d->copyAction->setEnabled( hasSelection() );
}

void
WebView::onCopy()
{
	QApplication::clipboard()->setText( selectedText() );
}

} /* namespace MdEditor */
