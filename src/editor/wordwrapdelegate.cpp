/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "wordwrapdelegate.hpp"

// Qt include.
#include <QHeaderView>
#include <QPainter>


namespace MdEditor {

//
// WordWrapItemDelegate
//

WordWrapItemDelegate::WordWrapItemDelegate( QTreeView * parent )
	:	QStyledItemDelegate( parent )
	,	m_parent( parent )
{
}

QSize
WordWrapItemDelegate::sizeHint( const QStyleOptionViewItem & option,
	const QModelIndex & index ) const
{
	int level = 1;
	auto i = index;
	
	while( i.parent().isValid() )
	{
		++level;
		i = i.parent();
	}
	
	return option.fontMetrics.boundingRect(
		QRect( 0, 0,
			m_parent->header()->sectionSize( index.column() ) -
				option.decorationSize.width() * level * 2, 0 ),
		Qt::AlignLeft | Qt::TextWordWrap,
		index.data( Qt::DisplayRole ).toString() ).size();
}

void
WordWrapItemDelegate::paint( QPainter * painter,
	const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
	QStyledItemDelegate::paint( painter, option, index );

	painter->drawText( option.rect,
		Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignVCenter,
		index.data( Qt::DisplayRole ).toString() );
}

void
WordWrapItemDelegate::initStyleOption( QStyleOptionViewItem * option,
	const QModelIndex & index ) const
{
	QStyledItemDelegate::initStyleOption( option, index );
	option->text = QString();
}

} /* namespace MdEditor */
