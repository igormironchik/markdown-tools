
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
#include "closebutton.hpp"

// Qt include.
#include <QEnterEvent>
#include <QPainter>
#include <QPixmap>
#include <QPalette>


namespace MdEditor {

//
// CloseButtonPrivate
//

struct CloseButtonPrivate {
	CloseButtonPrivate( CloseButton * parent )
		:	q( parent )
	{
	}

	void initUi()
	{
		q->setCheckable( false );

		activePixmap = QPixmap( QStringLiteral( ":/res/img/dialog-close.png" ) );

		auto source = activePixmap.toImage();
		QImage target = QImage( source.width(), source.height(), QImage::Format_ARGB32 );

		for( int x = 0; x < source.width(); ++x )
		{
			for( int y = 0; y < source.height(); ++y )
			{
				const auto g = qGray( source.pixel( x, y ) );
				target.setPixelColor( x, y, QColor( g, g, g, source.pixelColor( x, y ).alpha() ) );
			}
		}

		inactivePixmap = QPixmap::fromImage( target );

		q->setFocusPolicy( Qt::NoFocus );
	}

	CloseButton * q = nullptr;
	bool hovered = false;
	QPixmap activePixmap;
	QPixmap inactivePixmap;
}; // struct CloseButtonPrivate


//
// CloseButton
//

CloseButton::CloseButton( QWidget * parent )
	:	QAbstractButton( parent )
	,	d( new CloseButtonPrivate( this ) )
{
	d->initUi();
}

CloseButton::~CloseButton()
{
}

QSize
CloseButton::sizeHint() const
{
	return { 16, 16 };
}

void
CloseButton::paintEvent( QPaintEvent * e )
{
	QPainter p( this );

	if( d->hovered )
		p.drawPixmap( rect(), d->activePixmap );
	else
		p.drawPixmap( rect(), d->inactivePixmap );
}

void
CloseButton::enterEvent( QEnterEvent * event )
{
	d->hovered = true;

	update();

	event->accept();
}

void
CloseButton::leaveEvent( QEvent * event )
{
	d->hovered = false;

	update();

	event->accept();
}

} /* namespace MdEditor */
