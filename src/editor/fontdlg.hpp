
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
#include <QDialog>
#include <QScopedPointer>
#include <QFont>


namespace MdEditor {

//
// FontDlg
//

struct FontDlgPrivate;

//! Font dialog.
class FontDlg
	:	public QDialog
{
	Q_OBJECT

public:
	FontDlg( const QFont & f, QWidget * parent );
	~FontDlg() override;

	QFont font() const;

private slots:
	void onShowOnlyMonospaced( int state );

private:
	friend struct FontDlgPrivate;

	Q_DISABLE_COPY( FontDlg )

	QScopedPointer< FontDlgPrivate > d;
}; // class FontDlg

} /* namespace MdEditor */
