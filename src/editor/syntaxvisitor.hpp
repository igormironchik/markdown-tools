
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

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.hpp>
#include <md4qt/visitor.hpp>

// Qt include.
#include <QScopedPointer>


namespace MdEditor {

class Editor;
struct Colors;

//
// SyntaxVisitor
//

struct SyntaxVisitorPrivate;

//! Markdown syntax highlighter.
class SyntaxVisitor
	:	public MD::Visitor< MD::QStringTrait >
{
public:
	explicit SyntaxVisitor( Editor * editor );
	~SyntaxVisitor() override;

	void highlight( std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
		const Colors & colors );
	void setFont( const QFont & f );
	void clearHighlighting();

protected:
	void onAddLineEnding() override;
	void onText( MD::Text< MD::QStringTrait > * t ) override;
	void onMath( MD::Math< MD::QStringTrait > * m ) override;
	void onLineBreak( MD::LineBreak< MD::QStringTrait > * b ) override;
	void onHeading( MD::Heading< MD::QStringTrait > * h ) override;
	void onCode( MD::Code< MD::QStringTrait > * c ) override;
	void onInlineCode( MD::Code< MD::QStringTrait > * c ) override;
	void onBlockquote( MD::Blockquote< MD::QStringTrait > * b ) override;
	void onList( MD::List< MD::QStringTrait > * l ) override;
	void onTable( MD::Table< MD::QStringTrait > * t ) override;
	void onAnchor( MD::Anchor< MD::QStringTrait > * a ) override;
	void onRawHtml( MD::RawHtml< MD::QStringTrait > * h ) override;
	void onHorizontalLine( MD::HorizontalLine< MD::QStringTrait > * l ) override;
	void onLink( MD::Link< MD::QStringTrait > * l ) override;
	void onImage( MD::Image< MD::QStringTrait > * i ) override;
	void onFootnoteRef( MD::FootnoteRef< MD::QStringTrait > * ref ) override;
	void onFootnote( MD::Footnote< MD::QStringTrait > * f ) override;
	void onListItem( MD::ListItem< MD::QStringTrait > * l, bool first ) override;

private:
	Q_DISABLE_COPY( SyntaxVisitor )

	QScopedPointer< SyntaxVisitorPrivate > d;
}; // class SyntaxVisitor

} /* namespace MdEditor */
