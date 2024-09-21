/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.h>
#include <md4qt/poscache.h>

// Qt include.
#include <QScopedPointer>

// C++ include.
#include <vector>


namespace MdEditor {

class Editor;
struct Colors;

//
// SyntaxVisitor
//

struct SyntaxVisitorPrivate;

//! Markdown syntax highlighter.
class SyntaxVisitor
	:	public MD::PosCache< MD::QStringTrait >
{
public:
	explicit SyntaxVisitor( Editor * editor );
	~SyntaxVisitor() override;

	void highlight( std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
		const Colors & colors );
	void setFont( const QFont & f );
	void clearHighlighting();

protected:
	void onReferenceLink( MD::Link< MD::QStringTrait > * l ) override;
	void onText( MD::Text< MD::QStringTrait > * t ) override;
	void onMath( MD::Math< MD::QStringTrait > * m ) override;
	void onHeading( MD::Heading< MD::QStringTrait > * h ) override;
	void onCode( MD::Code< MD::QStringTrait > * c ) override;
	void onInlineCode( MD::Code< MD::QStringTrait > * c ) override;
	void onBlockquote( MD::Blockquote< MD::QStringTrait > * b ) override;
	void onTable( MD::Table< MD::QStringTrait > * t ) override;
	void onRawHtml( MD::RawHtml< MD::QStringTrait > * h ) override;
	void onHorizontalLine( MD::HorizontalLine< MD::QStringTrait > * l ) override;
	void onLink( MD::Link< MD::QStringTrait > * l ) override;
	void onImage( MD::Image< MD::QStringTrait > * i ) override;
	void onFootnoteRef( MD::FootnoteRef< MD::QStringTrait > * ref ) override;
	void onFootnote( MD::Footnote< MD::QStringTrait > * f ) override;
	void onListItem( MD::ListItem< MD::QStringTrait > * l, bool first ) override;

private:
	void onItemWithOpts( MD::ItemWithOpts< MD::QStringTrait > * i );

private:
	Q_DISABLE_COPY( SyntaxVisitor )

	QScopedPointer< SyntaxVisitorPrivate > d;
}; // class SyntaxVisitor

} /* namespace MdEditor */
