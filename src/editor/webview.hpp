/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWebEngineView>
#include <QScopedPointer>


namespace MdEditor {

//
// WebView
//

struct WebViewPrivate;

//! HTML preview.
class WebView
	:	public QWebEngineView
{
	Q_OBJECT

public:
	explicit WebView( QWidget * parent );
	~WebView() override;

private slots:
	void onSelectionChanged();
	void onCopy();

private:
	friend class WebViewPrivate;

	Q_DISABLE_COPY( WebView )

	QScopedPointer< WebViewPrivate > d;
}; // class WebView

} /* namespace MdEditor */
