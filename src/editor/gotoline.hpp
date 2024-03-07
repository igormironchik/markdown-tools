
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
// GoToLine
//

struct GoToLinePrivate;
class Editor;
class MainWindow;

//! Go to line widget.
class GoToLine
	:	public QFrame
{
	Q_OBJECT

public:
	GoToLine( MainWindow * window, Editor * editor, QWidget * parent );
	~GoToLine() override;

	QLineEdit * line() const;

public slots:
	void setFocusOnLine();

private slots:
	void onEditingFinished();
	void onClose();

private:
	friend struct GoToLinePrivate;

	Q_DISABLE_COPY( GoToLine )

	QScopedPointer< GoToLinePrivate > d;
}; // class GoToLine

} /* namespace MdEditor */
