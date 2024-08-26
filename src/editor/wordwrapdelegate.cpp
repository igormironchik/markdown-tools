/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "wordwrapdelegate.hpp"
#include "toc.hpp"

// Qt include.
#include <QHeaderView>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QSortFilterProxyModel>


namespace MdEditor {

static const QColor c_codeColor = QColor( 33, 122, 255 );
static const QColor c_codeBackgroundColor = QColor( 239, 239, 239 );

//
// WordWrapItemDelegate
//

WordWrapItemDelegate::WordWrapItemDelegate( QTreeView * parent, TocModel * model,
	QSortFilterProxyModel * sortModel )
	:	QStyledItemDelegate( parent )
	,	m_parent( parent )
	,	m_model( model )
	,	m_sortModel( sortModel )
{
}

QSize
WordWrapItemDelegate::sizeHint( const QStyleOptionViewItem & option,
	const QModelIndex & index ) const
{
	const auto w = QApplication::style()->pixelMetric( QStyle::PM_TreeViewIndentation );

	int level = 1;
	auto i = index;

	while( i.parent().isValid() )
	{
		++level;
		i = i.parent();
	}

	const auto & data = m_model->stringData( m_sortModel->mapToSource( index ) );
	const auto sw = option.fontMetrics.horizontalAdvance( QLatin1Char( ' ' ) );
	const auto width = m_parent->header()->sectionSize( index.column() ) - w * level;
	int x = 0;
	int height = option.fontMetrics.height();

	for( const auto & d : data )
	{
		for( const auto & text : d.text.split( QStringLiteral( " " ), Qt::SkipEmptyParts ) )
		{
			auto w = option.fontMetrics.boundingRect( text ).width();

			if( w + x > width )
			{
				if( w > width )
				{
					if( x > 0 )
						height += option.fontMetrics.height();

					QString str, tmpStr = text;

					while( !tmpStr.isEmpty() )
					{
						while( w > width )
						{
							str.prepend( tmpStr.back() );
							tmpStr.removeLast();

							w = option.fontMetrics.boundingRect( tmpStr ).width();
						}

						height += option.fontMetrics.height();
						tmpStr = str;
						str.clear();

						w = option.fontMetrics.boundingRect( tmpStr ).width();

						if( w <= width )
							break;
					}
				}
				else
					height += option.fontMetrics.height();

				x = 0;
			}

			x += w + sw;
		}
	}

	return { width, height };
}

void
WordWrapItemDelegate::paint( QPainter * painter,
	const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QStyledItemDelegate::paint( painter, option, index );

	const auto & data = m_model->stringData( m_sortModel->mapToSource( index ) );
	const auto sw = option.fontMetrics.horizontalAdvance( QLatin1Char( ' ' ) );

	auto x = option.rect.x();
	auto y = option.rect.y();
	int startCodeX, startCodeY, codeWidth;
	QVector< QRect > backgroundRects;
	QVector< QPair< QRect, StringData > > textRects;

	for( const auto & d : data )
	{
		if( d.code )
		{
			startCodeX = x;
			startCodeY = y;
		}

		codeWidth = 0;

		for( const auto & tt : d.text.split( QStringLiteral( " " ), Qt::SkipEmptyParts ) )
		{
			auto text = tt;

			auto w = option.fontMetrics.boundingRect( text ).width();

			if( w + x > option.rect.width() + option.rect.x() )
			{
				if( d.code && codeWidth > 0 )
					backgroundRects.append( { startCodeX, startCodeY, codeWidth, option.fontMetrics.height() } );

				if( w > option.rect.width() )
				{
					if( x > option.rect.x() )
					{
						y += option.fontMetrics.height();
						x = option.rect.x();
					}

					QString str, tmpStr = text;

					while( !tmpStr.isEmpty() )
					{
						while( w > option.rect.width() )
						{
							str.prepend( tmpStr.back() );
							tmpStr.removeLast();

							w = option.fontMetrics.boundingRect( tmpStr ).width();
						}

						textRects.append( std::make_pair( QRect( option.rect.x(), y,
								w, option.fontMetrics.height() ),
							StringData( tmpStr, d.code ) ) );

						if( d.code )
							backgroundRects.append( { option.rect.x(), y, w, option.fontMetrics.height() } );

						y += option.fontMetrics.height();

						tmpStr = str;
						str.clear();
						text = tmpStr;

						w = option.fontMetrics.boundingRect( tmpStr ).width();

						if( w <= option.rect.width() )
							break;
					}
				}

				if( x > option.rect.x() )
					y += option.fontMetrics.height();

				x = option.rect.x();
				startCodeX = x;
				startCodeY = y;
				codeWidth = 0;
			}
			else if( codeWidth > 0 )
				codeWidth += sw;

			if( d.code )
				codeWidth += w;

			textRects.append( std::make_pair( QRect( x, y, w, option.fontMetrics.height() ),
				StringData( text, d.code ) ) );

			x += w + sw;
		}

		if( d.code && codeWidth > 0 )
			backgroundRects.append( { startCodeX, startCodeY,
				codeWidth, option.fontMetrics.height() } );

		painter->setPen( Qt::NoPen );
		painter->setBrush( c_codeBackgroundColor );

		for( const auto & r : std::as_const( backgroundRects ) )
		{
			painter->drawRoundedRect( r.adjusted( 0, 1, 0, -1 ), 3, 3 );
		}

		for( const auto & p : std::as_const( textRects ) )
		{
			if( p.second.code )
				painter->setPen( c_codeColor );
			else
				painter->setPen( option.palette.color( QPalette::Text ) );

			painter->drawText( p.first, 0, p.second.text );
		}

		backgroundRects.clear();
		textRects.clear();
	}
}

void
WordWrapItemDelegate::initStyleOption( QStyleOptionViewItem * option,
	const QModelIndex & index ) const
{
	QStyledItemDelegate::initStyleOption( option, index );
	option->text = QString();
}

} /* namespace MdEditor */
