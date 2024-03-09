
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
#include <QPlainTextEdit>
#include <QScopedPointer>

// md-editor include.
#include "colors.hpp"

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.hpp>
#include <md4qt/parser.hpp>


namespace MdEditor {

//
// Editor
//

struct EditorPrivate;

//! Markdown text editor.
class Editor
	:	public QPlainTextEdit
{
	Q_OBJECT

signals:
	void lineHovered( int lineNumber, const QPoint & pos );
	void hoverLeaved();

public:
	explicit Editor( QWidget * parent );
	~Editor() override;

	void setDocName( const QString & name );
	const QString & docName() const;

	void lineNumberAreaPaintEvent( QPaintEvent * event );
	int lineNumberAreaWidth();
	bool foundHighlighted() const;
	bool foundSelected() const;
	void applyColors( const Colors & colors );
	std::shared_ptr< MD::Document< MD::QStringTrait > > currentDoc() const;
	void applyFont( const QFont & f );

public slots:
	void showUnprintableCharacters( bool on );
	void showLineNumbers( bool on );
	void highlight( const QString & text, bool initCursor );
	void clearExtraSelections();
	void replaceCurrent( const QString & with );
	void replaceAll( const QString & with );
	void highlightCurrent();
	void clearHighlighting();
	void goToLine( int l );

private slots:
	void updateLineNumberAreaWidth( int newBlockCount );
	void highlightCurrentLine();
	void updateLineNumberArea( const QRect & rect, int dy );
	void onFindNext();
	void onFindPrev();
	void onContentChanged();
	void highlightSyntax( const Colors & colors,
		std::shared_ptr< MD::Document< MD::QStringTrait > > doc );

protected:
	void resizeEvent( QResizeEvent * event ) override;
	void keyPressEvent( QKeyEvent * event ) override;

protected:
	int lineNumber( const QPoint & p );

private:
	friend struct EditorPrivate;
	friend class LineNumberArea;
	friend class FindPrivate;

	Q_DISABLE_COPY( Editor )

	QScopedPointer< EditorPrivate > d;
}; // class Editor


//
// LineNumberArea
//

//! Line number area.
class LineNumberArea
	:	public QWidget
{
	Q_OBJECT

signals:
	void lineHovered( int lineNumber, const QPoint & pos );
	void hoverLeaved();

public:
    LineNumberArea( Editor * editor )
		:	QWidget( editor )
		,	codeEditor( editor )
    {
		setMouseTracking( true );
	}

    QSize sizeHint() const override
    {
        return QSize( codeEditor->lineNumberAreaWidth(), 0 );
    }

protected:
    void paintEvent( QPaintEvent * event ) override
    {
        codeEditor->lineNumberAreaPaintEvent( event );
    }

	void enterEvent( QEnterEvent * event ) override;
	void mouseMoveEvent( QMouseEvent * event ) override;
	void leaveEvent( QEvent * event ) override;

private:
	void onHover( const QPoint & p );

private:
    Editor * codeEditor;
	int lineNumber = -1;
}; // class LineNumberArea

} /* namespace MdEditor */
