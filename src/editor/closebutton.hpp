
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
#include <QAbstractButton>
#include <QScopedPointer>


namespace MdEditor {

//
// CloseButton
//

struct CloseButtonPrivate;

//! Close button.
class CloseButton
	:	public QAbstractButton
{
	Q_OBJECT

public:
	explicit CloseButton( QWidget * parent );
	~CloseButton() override;

	QSize sizeHint() const override;

protected:
	void paintEvent( QPaintEvent * e ) override;
	void enterEvent( QEnterEvent * event ) override;
	void leaveEvent( QEvent * event ) override;

private:
	friend struct CloseButtonPrivate;

	Q_DISABLE_COPY( CloseButton )

	QScopedPointer< CloseButtonPrivate > d;
}; // class CloseButton

} /* namespace MdEditor */
