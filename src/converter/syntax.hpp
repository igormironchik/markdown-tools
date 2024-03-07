
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2019-2024 Igor Mironchik

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

#ifndef MD_PDF_SYNTAX_HPP_INCLUDED
#define MD_PDF_SYNTAX_HPP_INCLUDED

// Qt include.
#include <QString>
#include <QVector>
#include <QMap>

// KF6SyntaxHighlighting include.
#include <abstracthighlighter.h>
#include <repository.h>
#include <theme.h>
#include <definition.h>
#include <format.h>


//
// Syntax
//

//! Syntax highlighters.
class Syntax final
	:	public KSyntaxHighlighting::AbstractHighlighter
{
public:
	Syntax();
	~Syntax() override = default;

	//! Color for the text.
	struct Color {
		//! Index of line.
		qsizetype line;
		//! Start position of text.
		qsizetype startPos;
		//! End position of text.
		qsizetype endPos;
		//! Format.
		KSyntaxHighlighting::Format format;
	}; // struct Color

	//! Vector of colored text auxiliary structs.
	using Colors = QVector< Color >;

	void applyFormat( int offset, int length, const KSyntaxHighlighting::Format & format ) override;

	//! \return Vector of colored text auxiliary structs.
	Colors prepare( const QStringList & lines );

	KSyntaxHighlighting::Definition definitionForName( const QString & name ) const;
	KSyntaxHighlighting::Theme themeForName( const QString & name ) const;

	const KSyntaxHighlighting::Repository & repository() const;

private:
	int m_currentLineNumber = 0;
	Colors m_currentColors;
	KSyntaxHighlighting::Repository m_repository;
	QMap< QString, KSyntaxHighlighting::Definition > m_definitions;
	QMap< QString, KSyntaxHighlighting::Theme > m_themes;
}; // class Syntax

#endif // MD_PDF_SYNTAX_HPP_INCLUDED
