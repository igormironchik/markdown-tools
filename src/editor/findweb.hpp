
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
