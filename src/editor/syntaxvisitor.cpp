/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "syntaxvisitor.hpp"
#include "editor.hpp"
#include "colors.hpp"

// Qt include.
#include <QTextCursor>
#include <QTextEdit>
#include <QTextCharFormat>
#include <QTextBlock>

// C++ include.
#include <algorithm>


namespace MdEditor {

//
// PosRange
//

struct PosRange {
	long long int startColumn = -1;
	long long int startLine = -1;
	long long int endColumn = -1;
	long long int endLine = -1;
	MD::Item< MD::QStringTrait > * item = nullptr;
	std::vector< PosRange > children = {};
};

// Look at this equality operator like on rectangles intersection.
// If rectangles in text intersect then rectangles are equal.
bool operator == ( const PosRange & l, const PosRange & r )
{
	return ( l.startLine <= r.endLine && l.endLine >= r.startLine &&
		( l.startLine == r.endLine && l.endLine == r.startLine ?
			l.endColumn >= r.startColumn && l.startColumn <= r.endColumn :
			true ) );
}

bool operator < ( const PosRange & l, const PosRange & r )
{
	return ( l.endLine < r.startLine ||
		( l.endLine == r.startLine && l.endColumn < r.startColumn ) );
}


//
// SyntaxVisitorPrivate
//

struct SyntaxVisitorPrivate {
	SyntaxVisitorPrivate( Editor * e )
		:	editor( e )
	{
	}

	void clearFormats()
	{
		auto b = editor->document()->firstBlock();
		
		while( b.isValid() )
		{
			b.layout()->clearFormats();
			
			b = b.next();
		}

		formats.clear();
	}

	void applyFormats()
	{
		for( const auto & f : std::as_const( formats ) )
			f.block.layout()->setFormats( f.format );
	}
	
	void setFormat( const QTextCharFormat & format,
		const MD::WithPosition & pos )
	{
		setFormat( format, pos.startLine(), pos.startColumn(),
			pos.endLine(), pos.endColumn() )	;
	}

	void setFormat( const QTextCharFormat & format,
		long long int startLine, long long int startColumn,
		long long int endLine, long long int endColumn )
	{
		for( auto i = startLine; i <= endLine; ++i )
		{
			formats[ i ].block = editor->document()->findBlockByNumber( i );

			QTextLayout::FormatRange r;
			r.format = format;
			r.start = ( i == startLine ? startColumn : 0 );
			r.length = ( i == startLine ?
				( i == endLine ? endColumn - startColumn + 1 :
					formats[ i ].block.length() - startColumn ) :
				( i == endLine ? endColumn + 1 : formats[ i ].block.length() ) );

			formats[ i ].format.push_back( r );
		}
	}

	QFont styleFont( int opts ) const
	{
		auto f = font;

		if( opts & MD::ItalicText )
			f.setItalic( true );

		if( opts & MD::BoldText )
			f.setBold( true );

		if( opts & MD::StrikethroughText )
			f.setStrikeOut( true );

		return f;
	}
	
	PosRange * findInCache( std::vector< PosRange > & vec, const PosRange & pos ) const
	{
		const auto it = std::lower_bound( vec.begin(), vec.end(), pos );
		
		if( it != vec.end() && *it == pos )
		{
			if( !it->children.empty() )
			{
				auto nested = findInCache( it->children, pos );
				
				return ( nested ? nested : &(*it) );
			}
			else
				return &(*it);
		}
		else
			return nullptr;
	}
	
	void findAllInCache( const std::vector< PosRange > & vec,
		const PosRange & pos,
		std::vector< MD::Item< MD::QStringTrait >* > & res ) const
	{	
		const auto it = std::lower_bound( vec.begin(), vec.end(), pos );
		
		if( it != vec.end() && *it == pos )
		{
			res.push_back( it->item );
			
			if( !it->children.empty() )
				findAllInCache( it->children, pos, res );
		}
	}
	
	std::vector< MD::Item< MD::QStringTrait >* > findAllInCache( const MD::WithPosition & pos ) const
	{
		std::vector< MD::Item< MD::QStringTrait >* > res;
		
		PosRange tmp{ pos.startColumn(), pos.startLine(), pos.endColumn(), pos.endLine() };
		
		findAllInCache( cache, tmp, res );
		
		return res;
	}
	
	void insertInCache( const PosRange & item, bool sort = false )
	{
		if( !skipInCache )
		{
			auto pos = findInCache( cache, item );
			
			if( pos )
				pos->children.push_back( item );
			else
			{
				if( sort )
				{
					const auto it = std::upper_bound( cache.begin(), cache.end(), item );
					
					if( it != cache.end() )
						cache.insert( it, item );
					else
						cache.push_back( item );
				}
				else
					cache.push_back( item );
			}
		}
	}

	//! Editor.
	Editor * editor = nullptr;
	//! Document.
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc;
	//! Colors.
	Colors colors;

	struct Format {
		QTextBlock block;
		QList< QTextLayout::FormatRange > format;
	};

	//! Formats.
	QMap< int, Format > formats;
	//! Default font.
	QFont font;
	//! Cache.
	std::vector< PosRange > cache;
	//! Skip adding in cache.
	bool skipInCache = false;
}; // struct SyntaxVisitorPrivate


//
// SyntaxVisitor
//

SyntaxVisitor::SyntaxVisitor( Editor * editor )
	:	d( new SyntaxVisitorPrivate( editor ) )
{
}

SyntaxVisitor::~SyntaxVisitor()
{
}

std::vector< MD::Item< MD::QStringTrait >* >
SyntaxVisitor::findAllInCache( const MD::WithPosition & pos ) const
{
	return d->findAllInCache( pos );
}

void
SyntaxVisitor::setFont( const QFont & f )
{
	d->font = f;
}

void
SyntaxVisitor::clearHighlighting()
{
	d->clearFormats();
}

void
SyntaxVisitor::highlight( std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	const Colors & colors )
{
	d->clearFormats();
	d->cache.clear();

	d->doc = doc;
	d->colors = colors;

	if( d->doc )
	{
		MD::Visitor< MD::QStringTrait >::process( d->doc );

		for( auto it = d->doc->footnotesMap().cbegin(), last = d->doc->footnotesMap().cend();
			it != last; ++it )
		{
			onFootnote( it->second.get() );
		}
	}

	if( colors.enabled )
		d->applyFormats();
	else
		d->formats.clear();
}

void
SyntaxVisitor::onAddLineEnding()
{
}

void
SyntaxVisitor::onItemWithOpts( MD::ItemWithOpts< MD::QStringTrait > * i )
{
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );

	for( const auto & s : i->openStyles() )
		d->setFormat( special, s );
	
	for( const auto & s : i->closeStyles() )
		d->setFormat( special, s );
}

void
SyntaxVisitor::onText( MD::Text< MD::QStringTrait > * t )
{
	QTextCharFormat format;
	format.setForeground( d->colors.textColor );
	format.setFont( d->styleFont( t->opts() ) );

	d->setFormat( format, t->startLine(), t->startColumn(),
		t->endLine(), t->endColumn() );
	
	onItemWithOpts( t );
	
	PosRange r{ t->openStyles().empty() ? t->startColumn() : t->openStyles().front().startColumn(),
		t->startLine(),
		t->closeStyles().empty() ? t->endColumn() : t->closeStyles().back().endColumn(),
		t->endLine(), t };
	
	d->insertInCache( r );
}

void
SyntaxVisitor::onMath( MD::Math< MD::QStringTrait > * m )
{
	QTextCharFormat format;
	format.setForeground( d->colors.mathColor );

	d->setFormat( format, m->startLine(), m->startColumn(),
		m->endLine(), m->endColumn() );
	
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	
	if( m->startDelim().startColumn() != -1 )
		d->setFormat( special, m->startDelim() );
	
	if( m->endDelim().startColumn() != -1 )
		d->setFormat( special, m->endDelim() );
	
	if( m->syntaxPos().startColumn() != -1 )
		d->setFormat( special, m->syntaxPos() );
	
	onItemWithOpts( m );
	
	auto startColumn = m->startDelim().startColumn();
	auto startLine = m->startDelim().startLine();
	auto endColumn = m->endDelim().endColumn();
	auto endLine = m->endDelim().endLine();
	
	if( !m->openStyles().empty() )
	{
		startColumn = m->openStyles().front().startColumn();
		startLine = m->openStyles().front().startLine();
	}
	
	if( !m->closeStyles().empty() )
	{
		endColumn = m->closeStyles().back().endColumn();
		endLine = m->closeStyles().back().endLine();
	}
	
	PosRange r{ startColumn, startLine, endColumn, endLine, m };
	
	d->insertInCache( r );
}

void
SyntaxVisitor::onLineBreak( MD::LineBreak< MD::QStringTrait > * b )
{
}

void
SyntaxVisitor::onParagraph( MD::Paragraph< MD::QStringTrait > * p, bool wrap )
{
	PosRange r{ p->startColumn(), p->startLine(), p->endColumn(), p->endLine(), p };
	
	d->insertInCache( r );
	
	MD::Visitor< MD::QStringTrait >::onParagraph( p, wrap );
}

void
SyntaxVisitor::onHeading( MD::Heading< MD::QStringTrait > * h )
{
	QTextCharFormat format;
	format.setForeground( d->colors.headingColor );
	format.setFont( d->styleFont( MD::BoldText ) );
	
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	special.setFont( d->styleFont( MD::BoldText ) );

	d->setFormat( format, h->startLine(), h->startColumn(),
		h->endLine(), h->endColumn() );
	
	if( h->delim().startColumn() != -1 )
		d->setFormat( special, h->delim() );
	
	PosRange r{ h->startColumn(), h->startLine(), h->endColumn(), h->endLine(), h };
	
	d->insertInCache( r );
}

void
SyntaxVisitor::onCode( MD::Code< MD::QStringTrait > * c )
{
	QTextCharFormat format;
	format.setForeground( d->colors.codeColor );

	d->setFormat( format, c->startLine(), c->startColumn(),
		c->endLine(), c->endColumn() );
	
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	
	if( c->startDelim().startColumn() != -1 )
		d->setFormat( special, c->startDelim() );
	
	if( c->endDelim().startColumn() != -1 )
		d->setFormat( special, c->endDelim() );
	
	if( c->syntaxPos().startColumn() != -1 )
		d->setFormat( special, c->syntaxPos() );
	
	onItemWithOpts( c );
	
	auto startColumn = c->isFensedCode() ? c->startDelim().startColumn() : c->startColumn();
	auto startLine = c->isFensedCode() ? c->startDelim().startLine() : c->startLine();
	auto endColumn = c->isFensedCode() ? c->endDelim().endColumn() : c->endColumn();
	auto endLine = c->isFensedCode() ? c->endDelim().endLine() : c->endLine();
	
	PosRange r{ startColumn, startLine, endColumn, endLine, c };
	
	d->insertInCache( r );
}

void
SyntaxVisitor::onInlineCode( MD::Code< MD::QStringTrait > * c )
{
	QTextCharFormat format;
	format.setForeground( d->colors.inlineColor );

	d->setFormat( format, c->startLine(), c->startColumn(),
		c->endLine(), c->endColumn() );
	
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	
	if( c->startDelim().startColumn() != -1 )
		d->setFormat( special, c->startDelim() );
	
	if( c->endDelim().startColumn() != -1 )
		d->setFormat( special, c->endDelim() );
	
	onItemWithOpts( c );
	
	auto startColumn = c->startDelim().startColumn();
	auto startLine = c->startDelim().startLine();
	auto endColumn = c->endDelim().endColumn();
	auto endLine = c->endDelim().endLine();
	
	if( !c->openStyles().empty() )
	{
		startColumn = c->openStyles().front().startColumn();
		startLine = c->openStyles().front().startLine();
	}
	
	if( !c->closeStyles().empty() )
	{
		endColumn = c->closeStyles().back().endColumn();
		endLine = c->closeStyles().back().endLine();
	}
	
	PosRange r{ startColumn, startLine, endColumn, endLine, c };
	
	d->insertInCache( r );
}

void
SyntaxVisitor::onBlockquote( MD::Blockquote< MD::QStringTrait > * b )
{
	QTextCharFormat format;
	format.setForeground( d->colors.blockquoteColor );
	
	PosRange r{ b->startColumn(), b->startLine(), b->endColumn(), b->endLine(), b };
	
	d->insertInCache( r );

	MD::Visitor< MD::QStringTrait >::onBlockquote( b );
	
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	
	for( const auto & dd : b->delims() )
		d->setFormat( special, dd );
}

void
SyntaxVisitor::onList( MD::List< MD::QStringTrait > * l )
{
	bool first = true;
	
	PosRange r{ l->startColumn(), l->startLine(), l->endColumn(), l->endLine(), l };
	
	d->insertInCache( r );

	for( auto it = l->items().cbegin(), last = l->items().cend(); it != last; ++it )
	{
		if( (*it)->type() == MD::ItemType::ListItem )
		{
			onListItem( static_cast< MD::ListItem< MD::QStringTrait >* > ( it->get() ), first );

			first = false;
		}
	}
}

void
SyntaxVisitor::onListItem( MD::ListItem< MD::QStringTrait > * l, bool first )
{
	PosRange r{ l->startColumn(), l->startLine(), l->endColumn(), l->endLine(), l };
	
	d->insertInCache( r );
	
	QTextCharFormat format;
	format.setForeground( d->colors.listColor );
	format.setFont( d->font );

	MD::Visitor< MD::QStringTrait >::onListItem( l, first );
	
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	
	d->setFormat( special, l->delim() );
	
	if( l->taskDelim().startColumn() != -1 )
		d->setFormat( special, l->taskDelim() );
}

void
SyntaxVisitor::onTable( MD::Table< MD::QStringTrait > * t )
{
	PosRange r{ t->startColumn(), t->startLine(), t->endColumn(), t->endLine(), t };
	
	d->insertInCache( r );
	
	QTextCharFormat format;
	format.setForeground( d->colors.tableColor );

	d->setFormat( format, t->startLine(), t->startColumn(),
		t->endLine(), t->endColumn() );

	if( !t->isEmpty() )
	{
		int columns = 0;

		for( auto th = (*t->rows().cbegin())->cells().cbegin(),
			last = (*t->rows().cbegin())->cells().cend(); th != last; ++th )
		{
			onTableCell( th->get() );

			++columns;
		}

		for( auto r = std::next( t->rows().cbegin() ), rlast = t->rows().cend(); r != rlast; ++r )
		{
			int i = 0;

			for( auto c = (*r)->cells().cbegin(), clast = (*r)->cells().cend(); c != clast; ++c )
			{
				onTableCell( c->get() );

				++i;

				if( i == columns )
					break;
			}
		}
	}
}

void
SyntaxVisitor::onAnchor( MD::Anchor< MD::QStringTrait > * a )
{
}

void
SyntaxVisitor::onRawHtml( MD::RawHtml< MD::QStringTrait > * h )
{
	QTextCharFormat format;
	format.setForeground( d->colors.htmlColor );

	d->setFormat( format, h->startLine(), h->startColumn(),
		h->endLine(), h->endColumn() );
	
	PosRange r{ h->startColumn(), h->startLine(), h->endColumn(), h->endLine(), h };
	
	d->insertInCache( r );
}

void
SyntaxVisitor::onHorizontalLine( MD::HorizontalLine< MD::QStringTrait > * l )
{
	QTextCharFormat special;
	special.setForeground( d->colors.specialColor );
	
	d->setFormat( special, l->startLine(),
		l->startColumn(),
		l->endLine(),
		l->endColumn() );
	
	PosRange r{ l->startColumn(), l->startLine(), l->endColumn(), l->endLine(), l };
	
	d->insertInCache( r );
}

namespace /* anonymous */ {

std::shared_ptr< MD::Paragraph< MD::QStringTrait > >
copyParagraphAndApplyOpts( std::shared_ptr< MD::Paragraph< MD::QStringTrait > > p, int opts )
{
	auto tmp = std::static_pointer_cast< MD::Paragraph< MD::QStringTrait > > ( p->clone() );

	for( const auto & i : tmp->items() )
	{
		switch( i->type() )
		{
			case MD::ItemType::Text :
			{
				auto t = static_cast< MD::Text< MD::QStringTrait > * > ( i.get() );
				t->setOpts( opts | t->opts() );
			}
				break;

			case MD::ItemType::Link :
			{
				auto l = static_cast< MD::Link< MD::QStringTrait > * > ( i.get() );
				l->setOpts( opts | l->opts() );
				l->setP( copyParagraphAndApplyOpts( l->p(), opts ) );
			}
				break;

			case MD::ItemType::Code :
			{
				auto c = static_cast< MD::Code< MD::QStringTrait > * > ( i.get() );
				c->setOpts( opts | c->opts() );
			}
				break;

			default :
				break;
		}
	}

	return tmp;
}

} /* namespace anonymous */

void
SyntaxVisitor::onLink( MD::Link< MD::QStringTrait > * l )
{
	QTextCharFormat format;
	format.setForeground( d->colors.linkColor );
	format.setFont( d->styleFont( l->opts() ) );

	d->setFormat( format, l->startLine(), l->startColumn(),
		l->endLine(), l->endColumn() );
	
	auto startColumn = l->startColumn();
	auto startLine = l->startLine();
	auto endColumn = l->endColumn();
	auto endLine = l->endLine();
	
	if( !l->openStyles().empty() )
	{
		startColumn = l->openStyles().front().startColumn();
		startLine = l->openStyles().front().startLine();
	}
	
	if( !l->closeStyles().empty() )
	{
		endColumn = l->closeStyles().back().endColumn();
		endLine = l->closeStyles().back().endLine();
	}
	
	PosRange r{ startColumn, startLine, endColumn, endLine, l };
	
	d->insertInCache( r );

	if( l->p() )
	{
		d->skipInCache = true;
		const auto p = copyParagraphAndApplyOpts( l->p(), l->opts() );
		onParagraph( p.get(), true );
		d->skipInCache = false;
	}
	
	onItemWithOpts( l );
}

void
SyntaxVisitor::onImage( MD::Image< MD::QStringTrait > * i )
{
	QTextCharFormat format;
	format.setForeground( d->colors.linkColor );

	d->setFormat( format, i->startLine(), i->startColumn(),
		i->endLine(), i->endColumn() );
	
	auto startColumn = i->startColumn();
	auto startLine = i->startLine();
	auto endColumn = i->endColumn();
	auto endLine = i->endLine();
	
	if( !i->openStyles().empty() )
	{
		startColumn = i->openStyles().front().startColumn();
		startLine = i->openStyles().front().startLine();
	}
	
	if( !i->closeStyles().empty() )
	{
		endColumn = i->closeStyles().back().endColumn();
		endLine = i->closeStyles().back().endLine();
	}
	
	PosRange r{ startColumn, startLine, endColumn, endLine, i };
	
	d->insertInCache( r );

	if( i->p() )
	{
		d->skipInCache = true;
		onParagraph( i->p().get(), true );
		d->skipInCache = false;
	}
	
	onItemWithOpts( i );
}

void
SyntaxVisitor::onFootnoteRef( MD::FootnoteRef< MD::QStringTrait > * ref )
{
	if( d->doc->footnotesMap().find( ref->id() ) != d->doc->footnotesMap().cend() )
	{
		QTextCharFormat format;
		format.setForeground( d->colors.linkColor );
		format.setFont( d->styleFont( ref->opts() ) );

		d->setFormat( format, ref->startLine(), ref->startColumn(),
			ref->endLine(), ref->endColumn() );
	}
	else
	{
		QTextCharFormat format;
		format.setForeground( d->colors.textColor );
		format.setFont( d->styleFont( ref->opts() ) );

		d->setFormat( format, ref->startLine(), ref->startColumn(),
			ref->endLine(), ref->endColumn() );
	}
	
	auto startColumn = ref->startColumn();
	auto startLine = ref->startLine();
	auto endColumn = ref->endColumn();
	auto endLine = ref->endLine();
	
	if( !ref->openStyles().empty() )
	{
		startColumn = ref->openStyles().front().startColumn();
		startLine = ref->openStyles().front().startLine();
	}
	
	if( !ref->closeStyles().empty() )
	{
		endColumn = ref->closeStyles().back().endColumn();
		endLine = ref->closeStyles().back().endLine();
	}
	
	PosRange r{ startColumn, startLine, endColumn, endLine, ref };
	
	d->insertInCache( r );
	
	onItemWithOpts( ref );
}

void
SyntaxVisitor::onFootnote( MD::Footnote< MD::QStringTrait > * f )
{
	QTextCharFormat format;
	format.setForeground( d->colors.footnoteColor );

	d->setFormat( format, f->startLine(), f->startColumn(),
		f->endLine(), f->endColumn() );
	
	PosRange r{ f->startColumn(), f->startLine(), f->endColumn(), f->endLine(), f };
	
	d->insertInCache( r, true );

	MD::Visitor< MD::QStringTrait >::onFootnote( f );
}

} /* namespace MdEditor */
