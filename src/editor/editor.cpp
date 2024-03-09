
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
#include "editor.hpp"
#include "syntaxvisitor.hpp"

// Qt include.
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>

// C++ include.
#include <functional>
#include <utility>


namespace MdEditor {

//
// EditorPrivate
//

struct EditorPrivate {
	EditorPrivate( Editor * parent )
		:	q( parent )
		,	syntax( parent )
	{
	}

	void initUi()
	{
		lineNumberArea = new LineNumberArea( q );

		QObject::connect( q, &Editor::cursorPositionChanged,
			q, &Editor::highlightCurrentLine );
		QObject::connect( q, &QPlainTextEdit::textChanged,
			q, &Editor::onContentChanged );

		q->showLineNumbers( true );
		q->applyFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );
		q->updateLineNumberAreaWidth( 0 );
		q->highlightCurrentLine();
		q->showUnprintableCharacters( true );
		q->setFocusPolicy( Qt::ClickFocus );
		q->setCenterOnScroll( true );
	}

	void setExtraSelections()
	{
		QList< QTextEdit::ExtraSelection > tmp = syntaxHighlighting;
		tmp << extraSelections;
		tmp.prepend( currentLine );

		q->setExtraSelections( tmp );
	}

	Editor * q = nullptr;
	LineNumberArea * lineNumberArea = nullptr;
	QString docName;
	bool showLineNumberArea = true;
	QList< QTextEdit::ExtraSelection > extraSelections;
	QList< QTextEdit::ExtraSelection > syntaxHighlighting;
	QTextEdit::ExtraSelection currentLine;
	QString highlightedText;
	Colors colors;
	std::shared_ptr< MD::Document< MD::QStringTrait > > currentDoc;
	SyntaxVisitor syntax;
}; // struct EditorPrivate


//
// Editor
//

Editor::Editor( QWidget * parent )
	:	QPlainTextEdit( parent )
	,	d( new EditorPrivate( this ) )
{
	d->initUi();
}

Editor::~Editor()
{
}

bool
Editor::foundHighlighted() const
{
	return !d->extraSelections.isEmpty();
}

bool
Editor::foundSelected() const
{
	const auto c = textCursor();

	for( const auto & s : std::as_const( d->extraSelections ) )
	{
		if( c.position() == s.cursor.position() )
		{
			if( c.hasSelection() &&
				c.selectionStart() == s.cursor.selectionStart() &&
				c.selectionEnd() == s.cursor.selectionEnd() )
					return true;
			else
				return false;
		}
	}

	return false;
}

void
Editor::applyColors( const Colors & colors )
{
	d->colors = colors;

	if( d->colors.enabled )
		onContentChanged();
	else
		d->syntax.clearHighlighting();

	viewport()->update();
}

std::shared_ptr< MD::Document< MD::QStringTrait > >
Editor::currentDoc() const
{
	return d->currentDoc;
}

void
Editor::applyFont( const QFont & f )
{
	setFont( f );

	d->syntax.setFont( f );

	highlightSyntax( d->colors, d->currentDoc );
}

void
Editor::setDocName( const QString & name )
{
	d->docName = name;
}

const QString &
Editor::docName() const
{
	return d->docName;
}

int
Editor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax( 1, blockCount() );

	while( max >= 10 )
	{
		max /= 10;
		++digits;
	}

	if( digits < 2 )
		digits = 2;

	int space = 3 + fontMetrics().horizontalAdvance( QLatin1Char( '9' ) ) * digits;

	return space;
}

void
Editor::updateLineNumberAreaWidth( int /* newBlockCount */ )
{
	if( d->showLineNumberArea )
		setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
	else
		setViewportMargins( 0, 0, 0, 0 );
}

void
Editor::updateLineNumberArea( const QRect & rect, int dy )
{
    if( dy )
        d->lineNumberArea->scroll( 0, dy );
    else
        d->lineNumberArea->update( 0, rect.y(), d->lineNumberArea->width(), rect.height() );

    if( rect.contains( viewport()->rect() ) )
        updateLineNumberAreaWidth( 0 );
}

void
Editor::resizeEvent( QResizeEvent * e )
{
	QPlainTextEdit::resizeEvent( e );

	QRect cr = contentsRect();
	d->lineNumberArea->setGeometry( QRect( cr.left(), cr.top(),
		lineNumberAreaWidth(), cr.height() ) );
}

void
Editor::highlightCurrentLine()
{
	static const QColor lineColor = QColor( Qt::yellow ).lighter( 180 );

	d->currentLine.format.setBackground( lineColor );
	d->currentLine.format.setProperty( QTextFormat::FullWidthSelection, true );
	d->currentLine.cursor = textCursor();
	d->currentLine.cursor.clearSelection();

	d->setExtraSelections();
}

void
Editor::lineNumberAreaPaintEvent( QPaintEvent * event )
{
	QPainter painter( d->lineNumberArea );
	painter.fillRect( event->rect(), Qt::lightGray );

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound( blockBoundingGeometry( block ).translated( contentOffset() ).top() );
	int bottom = top + qRound( blockBoundingRect( block ).height() );

	while( block.isValid() && top <= event->rect().bottom() )
	{
		if( block.isVisible() && bottom >= event->rect().top() )
		{
			QString number = QString::number( blockNumber + 1 );
			painter.setPen( Qt::black );
			painter.drawText( 0, top, d->lineNumberArea->width(), fontMetrics().height(),
				Qt::AlignRight, number );
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound( blockBoundingRect( block ).height() );
		++blockNumber;
	}
}

int
Editor::lineNumber( const QPoint & p )
{
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound( blockBoundingGeometry( block ).translated( contentOffset() ).top() );
	int bottom = top + qRound( blockBoundingRect( block ).height() );

	while( block.isValid() && top <= p.y() )
	{
		if( block.isVisible() && bottom >= p.y() )
			return blockNumber;

		block = block.next();
		top = bottom;
		bottom = top + qRound( blockBoundingRect( block ).height() );
		++blockNumber;
	}

	return -1;
}

void
LineNumberArea::enterEvent( QEnterEvent * event )
{
	onHover( event->position().toPoint() );

	event->ignore();
}

void
LineNumberArea::mouseMoveEvent( QMouseEvent * event )
{
	onHover( event->position().toPoint() );

	event->ignore();
}

void
LineNumberArea::leaveEvent( QEvent * event )
{
	lineNumber = -1;

	emit hoverLeaved();

	event->ignore();
}

void
LineNumberArea::onHover( const QPoint & p )
{
	const auto ln = codeEditor->lineNumber( p );

	if( ln != lineNumber )
	{
		lineNumber = ln;

		emit lineHovered( lineNumber, mapToGlobal( QPoint( width(), p.y() ) ) );
	}
}

void
Editor::showUnprintableCharacters( bool on )
{
	if( on )
	{
		QTextOption opt;
		opt.setFlags( QTextOption::ShowTabsAndSpaces );

		document()->setDefaultTextOption( opt );
	}
	else
		document()->setDefaultTextOption( {} );

	setTabStopDistance( fontMetrics().horizontalAdvance( QLatin1Char( ' ' ) ) * 4 );
}

void
Editor::showLineNumbers( bool on )
{
	if( on )
	{
		connect( this, &Editor::blockCountChanged,
			this, &Editor::updateLineNumberAreaWidth );
		connect( this, &Editor::updateRequest,
			this, &Editor::updateLineNumberArea );
		connect( d->lineNumberArea, &LineNumberArea::lineHovered,
			this, &Editor::lineHovered );
		connect( d->lineNumberArea, &LineNumberArea::hoverLeaved,
			this, &Editor::hoverLeaved );

		d->lineNumberArea->show();
		d->showLineNumberArea = true;
	}
	else
	{
		disconnect( this, &Editor::blockCountChanged,
			this, &Editor::updateLineNumberAreaWidth );
		disconnect( this, &Editor::updateRequest,
			this, &Editor::updateLineNumberArea );
		disconnect( d->lineNumberArea, &LineNumberArea::lineHovered,
			this, &Editor::lineHovered );
		disconnect( d->lineNumberArea, &LineNumberArea::hoverLeaved,
			this, &Editor::hoverLeaved );

		d->lineNumberArea->hide();
		d->showLineNumberArea = false;
	}

	updateLineNumberAreaWidth( 0 );
}

namespace /* anonymous */ {

template< class Iterator, class C = std::less<> >
bool markSelection( Iterator first, Iterator last, QTextCursor c, Editor * e, C cmp = C{} )
{
	for( ; first != last; ++first )
	{
		if( cmp( c.position(), first->cursor.position() ) )
		{
			c.setPosition( first->cursor.selectionStart() );
			c.setPosition( first->cursor.selectionEnd(), QTextCursor::KeepAnchor );
			e->setTextCursor( c );

			return true;
		}
	}

	return false;
}

} /* namespace anonymous */

void
Editor::highlight( const QString & text, bool initCursor )
{
	d->highlightedText = text;

	d->extraSelections.clear();

	if( !text.isEmpty() )
	{
		QTextCursor c( document() );

		static const QColor color = QColor( Qt::yellow );

		while( !c.isNull() )
		{
			QTextEdit::ExtraSelection s;

			s.format.setBackground( color );
			s.cursor = document()->find( text, c, QTextDocument::FindCaseSensitively );

			if( !s.cursor.isNull() )
				d->extraSelections.append( s );

			c = s.cursor;
		}
	}

	d->setExtraSelections();

	if( !d->extraSelections.isEmpty() && initCursor )
	{
		if( !markSelection( d->extraSelections.cbegin(), d->extraSelections.cend(),
			QTextCursor( firstVisibleBlock() ), this ) )
		{
			markSelection( d->extraSelections.crbegin(), d->extraSelections.crend(),
				QTextCursor( firstVisibleBlock() ), this, std::greater<> {} );
		}
	}
}

void
Editor::onFindNext()
{
	if( !d->extraSelections.isEmpty() )
	{		
		if( !markSelection( d->extraSelections.cbegin(), d->extraSelections.cend(),
			textCursor(), this ) )
		{
			auto s = d->extraSelections.at( 0 ).cursor;
			auto c = textCursor();
			c.setPosition( s.selectionStart() );
			c.setPosition( s.selectionEnd(), QTextCursor::KeepAnchor );
			setTextCursor( c );
		}
	}
}

void
Editor::onFindPrev()
{
	if( !d->extraSelections.isEmpty() )
	{
		auto c = textCursor();

		if( !markSelection( d->extraSelections.crbegin(), d->extraSelections.crend(),
			textCursor(), this, std::greater<> {} ) )
		{
			auto s = d->extraSelections.at( d->extraSelections.size() - 1 ).cursor;
			auto c = textCursor();
			c.setPosition( s.selectionStart() );
			c.setPosition( s.selectionEnd(), QTextCursor::KeepAnchor );
			setTextCursor( c );
		}
	}
}

void
Editor::clearExtraSelections()
{
	d->highlightedText.clear();
	d->extraSelections.clear();

	d->setExtraSelections();
}

void
Editor::replaceCurrent( const QString & with )
{
	if( foundSelected() )
	{
		auto c = textCursor();
		c.beginEditBlock();
		c.removeSelectedText();
		c.insertText( with );
		c.endEditBlock();
	}
}

void
Editor::replaceAll( const QString & with )
{
	if( foundHighlighted() )
	{
		disconnect( this, &QPlainTextEdit::textChanged, this, &Editor::onContentChanged );

		QTextCursor editCursor( document() );

		editCursor.beginEditBlock();

		QTextCursor found = editCursor;

		while( !found.isNull() )
		{
			found = document()->find( d->highlightedText, editCursor,
				QTextDocument::FindCaseSensitively );

			if( !found.isNull() )
			{
				editCursor.setPosition( found.selectionStart() );
				editCursor.setPosition( found.selectionEnd(), QTextCursor::KeepAnchor );

				editCursor.removeSelectedText();
				editCursor.insertText( with );
			}
		}

		editCursor.endEditBlock();

		clearExtraSelections();
	}

	connect( this, &QPlainTextEdit::textChanged, this, &Editor::onContentChanged );
}

void
Editor::onContentChanged()
{
	auto md = toPlainText();
	QTextStream stream( &md );

	MD::Parser< MD::QStringTrait > parser;

	d->currentDoc = parser.parse( stream, d->docName );

	if( d->colors.enabled )
		highlightSyntax( d->colors, d->currentDoc );

	highlightCurrent();
}

void
Editor::highlightCurrent()
{
	highlight( d->highlightedText, false );
}

void
Editor::clearHighlighting()
{
	clearExtraSelections();
}

void
Editor::goToLine( int l )
{
	QTextBlock block = document()->begin();
	int blockNumber = block.blockNumber() + 1;

	while( block.isValid() && blockNumber < l )
	{
		block = block.next();
		++blockNumber;
	}

	if( !block.isValid() )
		block = document()->lastBlock();

	auto cursor = textCursor();
	cursor.setPosition( block.position() );
	setTextCursor( cursor );

	ensureCursorVisible();

	setFocus();
}

void
Editor::highlightSyntax( const Colors & colors,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc )
{
	d->syntax.highlight( doc, colors );
}

void
Editor::keyPressEvent( QKeyEvent * event )
{
	auto c = textCursor();
	
	if( c.hasSelection() )
	{
		switch( event->key() )
		{
			case Qt::Key_Tab :
			{
				const auto ss = c.selectionStart();
				const auto se = c.selectionEnd();
				c.setPosition( ss );
				const auto start = c.blockNumber();
				c.setPosition( se, QTextCursor::KeepAnchor );
				const auto end = c.blockNumber();
				
				c.beginEditBlock();
				
				for( auto i = start; i <= end; ++i )
				{
					QTextCursor add( document()->findBlockByNumber( i ) );
					add.insertText( QStringLiteral( "\t" ) );
				}
				
				c.endEditBlock();
			}
				break;
				
			case Qt::Key_Backtab :
			{
				const auto ss = c.selectionStart();
				const auto se = c.selectionEnd();
				c.setPosition( ss );
				const auto start = c.blockNumber();
				c.setPosition( se, QTextCursor::KeepAnchor );
				const auto end = c.blockNumber();
				
				c.beginEditBlock();
				
				for( auto i = start; i <= end; ++i )
				{
					QTextCursor del( document()->findBlockByNumber( i ) );
					
					if( del.block().text().startsWith( QStringLiteral( "\t" ) ) )
						del.deleteChar();
				}
				
				c.endEditBlock();
			}
				break;
				
			default:
				QPlainTextEdit::keyPressEvent( event );
		}
	}
	else
		QPlainTextEdit::keyPressEvent( event );
}

} /* namespace MdEditor */
