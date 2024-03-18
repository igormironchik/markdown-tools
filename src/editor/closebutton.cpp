/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
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
