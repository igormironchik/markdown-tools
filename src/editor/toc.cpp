/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "toc.hpp"


namespace MdEditor {

//
// TocData
//

struct TocData {
	TocData( const QString & t, long long int l, int v, TocData * p = nullptr )
		:	text( t )
		,	line( l )
		,	level( v )
		,	parent( p )
	{
	}
	
	QString text = {};
	long long int line = -1;
	int level = -1;
	TocData * parent = nullptr;
	std::vector< std::shared_ptr< TocData > > children;
}; // struct TocData


//
// TocModelPrivate
//

struct TocModelPrivate {
	TocModelPrivate( TocModel * parent )
		:	q( parent )
	{
	}
	
	//! Parent.
	TocModel * q;
	//! Model's data.
	std::vector< std::shared_ptr< TocData > > data;
}; // struct TocModelPrivate


//
// TocModel
//

TocModel::TocModel( QObject * parent )
	:	QAbstractItemModel( parent )
	,	d( new TocModelPrivate( this ) )
{
}

TocModel::~TocModel()
{
}

void
TocModel::addTopLevelItem( const QString & text, long long int line,
	int level )
{
	beginInsertRows( QModelIndex(), d->data.size(), d->data.size() );
	d->data.push_back( std::make_shared< TocData >( text, line, level ) );
	endInsertRows();
}

void
TocModel::addChildItem( const QModelIndex & parent, const QString & text,
	long long int line, int level )
{
	auto data = static_cast< TocData* > ( parent.internalPointer() );
	
	beginInsertRows( parent, data->children.size(), data->children.size() );
	data->children.push_back( std::make_shared< TocData >( text, line, level, data ) ) ;
	endInsertRows();
}

void
TocModel::clear()
{
	beginResetModel();
	d->data.clear();
	endResetModel();
}

int
TocModel::level( const QModelIndex & index ) const
{
	return static_cast< TocData* > ( index.internalPointer() )->level;
}

int
TocModel::lineNumber( const QModelIndex & index ) const
{
	return static_cast< TocData* > ( index.internalPointer() )->line;
}
	
int
TocModel::rowCount( const QModelIndex & parent ) const
{
	if( !parent.isValid() )
		return d->data.size();
	else
		return static_cast< TocData* > ( parent.internalPointer() )->children.size();
}

int
TocModel::columnCount( const QModelIndex & parent ) const
{
	Q_UNUSED( parent )

	return 1;
}

QVariant
TocModel::data( const QModelIndex & index, int role ) const
{
	if( role == Qt::DisplayRole )
		return static_cast< TocData* > ( index.internalPointer() )->text;
	else
		return QVariant();
}

bool
TocModel::setData( const QModelIndex & index, const QVariant & value, int role )
{
	const int column = index.column();
	const int row = index.row();

	if( role == Qt::DisplayRole )
		static_cast< TocData* > ( index.internalPointer() )->text = value.toString();

	emit dataChanged( index, index );

	return true;
}

Qt::ItemFlags
TocModel::flags( const QModelIndex & index ) const
{
	return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
}

QVariant
TocModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	Q_UNUSED( section )
	Q_UNUSED( orientation )
	Q_UNUSED( role )
	
	return QVariant();
}

QModelIndex
TocModel::index( int row, int column,
	const QModelIndex & parent ) const
{
	if( !parent.isValid() )	
		return createIndex( row, column, d->data[ row ].get() );
	else
	{
		auto data = static_cast< TocData* >( parent.internalPointer() );
		
		return createIndex( row, column, data->children[ row ].get() );
	}
}

QModelIndex
TocModel::parent( const QModelIndex & index ) const
{
	auto data = static_cast< TocData* > ( index.internalPointer() );
	
	if( data->parent )
	{
		int row = -1;
		
		if( data->parent->parent )
			row = std::distance( data->parent->parent->children.cbegin(),
				std::find_if( data->parent->parent->children.cbegin(),
					data->parent->parent->children.cend(),
					[data] ( const auto & dd ) { return ( data->parent->line == dd->line ); } ) );
		else
			row = std::distance( d->data.cbegin(),
				std::find_if( d->data.cbegin(), d->data.cend(),
					[data] ( const auto & dd ) { return ( data->parent->line == dd->line ); } ) );
		
		return createIndex( row, 0, data->parent );
	}
	else
		return QModelIndex();
}

} /* namespace MdEditor */
