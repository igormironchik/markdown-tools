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


namespace MdEditor {

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
		for( const auto & f : std::as_const( formats ) )
			f.block.layout()->clearFormats();

		formats.clear();
	}

	void applyFormats()
	{
		for( const auto & f : std::as_const( formats ) )
			f.block.layout()->setFormats( f.format );
	}

	long long int blockquoteOffset( const QString & s ) const
	{
		auto findBlockquote = [] ( const QString & s, long long int p ) -> long long int
		{
			while( p < s.length() && s[ p ].isSpace() )
				++p;

			if( p < s.length() && s[ p ] == QLatin1Char( '>' ) )
				return p;

			return -1;
		};

		long long int pos = 0, delta = 0, stack = blockquoteStackSize;

		while( ( pos = findBlockquote( s, pos ) ) != -1 )
		{
			--stack;
			++pos;
			delta = pos;

			if( !stack )
				break;
		}

		return delta;
	}

	void setFormat( const QTextCharFormat & format,
		long long int startLine, long long int startColumn,
		long long int endLine, long long int endColumn )
	{
		for( auto i = startLine; i <= endLine; ++i )
		{
			formats[ i ].block = editor->document()->findBlockByNumber( i );

			long long int delta = 0;

			if( blockquoteStackSize )
				delta = blockquoteOffset( formats[ i ].block.text() );

			QTextLayout::FormatRange r;
			r.format = format;
			r.start = ( i == startLine ? startColumn : delta );
			r.length = ( i == startLine ?
				( i == endLine ? endColumn - startColumn + 1 :
					formats[ i ].block.length() - startColumn ) :
				( i == endLine ? endColumn + 1 - delta : formats[ i ].block.length() - delta ) );

			formats[ i ].format.push_back( r );
		}
	}

	QFont styleFont( int opts )
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
	//! Blockquote stack counter.
	int blockquoteStackSize = 0;
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
SyntaxVisitor::clearFormats()
{
	d->formats.clear();
}

void
SyntaxVisitor::highlight( std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	const Colors & colors )
{
	d->clearFormats();

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

	d->applyFormats();
}

void
SyntaxVisitor::onAddLineEnding()
{
}

void
SyntaxVisitor::onText( MD::Text< MD::QStringTrait > * t )
{
	QTextCharFormat format;
	format.setForeground( d->colors.textColor );
	format.setFont( d->styleFont( t->opts() ) );

	d->setFormat( format, t->startLine(), t->startColumn(),
		t->endLine(), t->endColumn() );
}

void
SyntaxVisitor::onMath( MD::Math< MD::QStringTrait > * m )
{
	QTextCharFormat format;
	format.setForeground( d->colors.mathColor );

	d->setFormat( format, m->startLine(), m->startColumn(),
		m->endLine(), m->endColumn() );
}

void
SyntaxVisitor::onLineBreak( MD::LineBreak< MD::QStringTrait > * b )
{
}

void
SyntaxVisitor::onHeading( MD::Heading< MD::QStringTrait > * h )
{
	QTextCharFormat format;
	format.setForeground( d->colors.headingColor );
	format.setFont( d->styleFont( MD::BoldText ) );

	d->setFormat( format, h->startLine(), h->startColumn(),
		h->endLine(), h->endColumn() );
}

void
SyntaxVisitor::onCode( MD::Code< MD::QStringTrait > * c )
{
	QTextCharFormat format;
	format.setForeground( d->colors.codeColor );

	d->setFormat( format, c->startLine(), c->startColumn(),
		c->endLine(), c->endColumn() );
}

void
SyntaxVisitor::onInlineCode( MD::Code< MD::QStringTrait > * c )
{
	QTextCharFormat format;
	format.setForeground( d->colors.inlineColor );

	d->setFormat( format, c->startLine(), c->startColumn(),
		c->endLine(), c->endColumn() );
}

void
SyntaxVisitor::onBlockquote( MD::Blockquote< MD::QStringTrait > * b )
{
	QTextCharFormat format;
	format.setForeground( d->colors.blockquoteColor );

	d->setFormat( format, b->startLine(), b->startColumn(),
		b->endLine(), b->endColumn() );

	++d->blockquoteStackSize;

	MD::Visitor< MD::QStringTrait >::onBlockquote( b );

	--d->blockquoteStackSize;
}

void
SyntaxVisitor::onList( MD::List< MD::QStringTrait > * l )
{
	bool first = true;

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
	QTextCharFormat format;
	format.setForeground( d->colors.listColor );
	format.setFont( d->font );

	d->setFormat( format, l->startLine(), l->startColumn(),
		l->endLine(), l->endColumn() );

	MD::Visitor< MD::QStringTrait >::onListItem( l, first );
}

void
SyntaxVisitor::onTable( MD::Table< MD::QStringTrait > * t )
{
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
}

void
SyntaxVisitor::onHorizontalLine( MD::HorizontalLine< MD::QStringTrait > * l )
{
}

namespace /* anonymous */ {
void
copyPositions( MD::Item< MD::QStringTrait > * from,
	MD::Item< MD::QStringTrait > * to )
{
	to->setStartColumn( from->startColumn() );
	to->setStartLine( from->startLine() );
	to->setEndColumn( from->endColumn() );
	to->setEndLine( from->endLine() );
}

std::shared_ptr< MD::Paragraph< MD::QStringTrait > >
copyParagraphAndApplyOpts( std::shared_ptr< MD::Paragraph< MD::QStringTrait > > p, int opts )
{
	auto tmp = std::make_shared< MD::Paragraph< MD::QStringTrait > > ();
	copyPositions( p.get(), tmp.get() );

	for( const auto & i : p->items() )
	{
		switch( i->type() )
		{
			case MD::ItemType::Text :
			{
				auto from = static_cast< MD::Text< MD::QStringTrait > * > ( i.get() );
				auto t = std::make_shared< MD::Text< MD::QStringTrait > > ();
				copyPositions( i.get(), t.get() );
				t->setText( from->text() );
				t->setSpaceBefore( from->isSpaceBefore() );
				t->setSpaceAfter( from->isSpaceAfter() );
				t->setOpts( opts | from->opts() );
				tmp->appendItem( t );
			}
				break;

			case MD::ItemType::Link :
			{
				auto from = static_cast< MD::Link< MD::QStringTrait > * > ( i.get() );
				auto l = std::make_shared< MD::Link< MD::QStringTrait > > ();
				copyPositions( i.get(), l.get() );
				l->setText( from->text() );
				l->setImg( from->img() );
				l->setUrl( from->url() );
				l->setOpts( opts | from->opts() );
				l->setP( copyParagraphAndApplyOpts( from->p(), opts ) );
				tmp->appendItem( l );
			}
				break;

			case MD::ItemType::Code :
			{
				auto from = static_cast< MD::Code< MD::QStringTrait > * > ( i.get() );
				auto c = std::make_shared< MD::Code< MD::QStringTrait > > (
					from->text(), false, from->isInlined() );
				copyPositions( i.get(), c.get() );
				c->setSyntax( from->syntax() );
				c->setOpts( opts | from->opts() );
				tmp->appendItem( c );
			}
				break;

			default :
				tmp->appendItem( i );
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

	if( l->p() )
	{
		const auto p = copyParagraphAndApplyOpts( l->p(), l->opts() );
		onParagraph( p.get(), true );
	}
}

void
SyntaxVisitor::onImage( MD::Image< MD::QStringTrait > * i )
{
	QTextCharFormat format;
	format.setForeground( d->colors.linkColor );

	d->setFormat( format, i->startLine(), i->startColumn(),
		i->endLine(), i->endColumn() );

	if( i->p() )
		onParagraph( i->p().get(), true );
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
}

void
SyntaxVisitor::onFootnote( MD::Footnote< MD::QStringTrait > * f )
{
	QTextCharFormat format;
	format.setForeground( d->colors.footnoteColor );

	d->setFormat( format, f->startLine(), f->startColumn(),
		f->endLine(), f->endColumn() );

	MD::Visitor< MD::QStringTrait >::onFootnote( f );
}

} /* namespace MdEditor */
