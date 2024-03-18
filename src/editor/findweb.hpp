/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QFrame>
#include <QScopedPointer>


QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE


namespace MdEditor {

//
// FindWeb
//

struct FindWebPrivate;
class MainWindow;
class WebView;

//! FindWeb/replace widget.
class FindWeb
	:	public QFrame
{
	Q_OBJECT

public:
	FindWeb( MainWindow * window, WebView * web, QWidget * parent );
	~FindWeb() override;

	QLineEdit * line() const;

public slots:
	void setFindWebText( const QString & text );
	void setFocusOnFindWeb();

private slots:
	void onFindWebTextChanged( const QString & str );
	void onClose();
	void onFindPrev();
	void onFindNext();

protected:
	void hideEvent( QHideEvent * event ) override;

private:
	friend struct FindWebPrivate;

	Q_DISABLE_COPY( FindWeb )

	QScopedPointer< FindWebPrivate > d;
}; // class FindWeb

} /* namespace MdEditor */
