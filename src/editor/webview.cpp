/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
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
