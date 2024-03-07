
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

// md-pdf include.
#include "syntax.hpp"

// KF6SyntaxHighlighting include.
#include <state.h>

// Qt include.
#include <QPair>

// C++ included.
#include <utility>


//
// Syntax
//

Syntax::Syntax()
{
	const auto defs = m_repository.definitions();

	for( const auto & d : defs )
	{
		if( d.name() == QStringLiteral( "C++" ) )
		{
			m_definitions.insert( QStringLiteral( "c++" ), d );
			m_definitions.insert( QStringLiteral( "cpp" ), d );

		}
		else if( d.name() == QStringLiteral( "JavaScript" ) )
		{
			m_definitions.insert( QStringLiteral( "javascript" ), d );
			m_definitions.insert( QStringLiteral( "js" ), d );
		}
		else
			m_definitions.insert( d.name().toLower(), d );
	}

	const auto th = m_repository.themes();

	for( const auto & t : th )
		m_themes.insert( t.name(), t );
}

const KSyntaxHighlighting::Repository &
Syntax::repository() const
{
	return m_repository;
}

KSyntaxHighlighting::Definition
Syntax::definitionForName( const QString & name ) const
{
	static KSyntaxHighlighting::Definition defaultDefinition;

	if( m_definitions.contains( name.toLower() ) )
		return m_definitions[ name.toLower() ];
	else
		return defaultDefinition;
}

KSyntaxHighlighting::Theme
Syntax::themeForName( const QString & name ) const
{
	static KSyntaxHighlighting::Theme defaultTheme;

	if( m_themes.contains( name ) )
		return m_themes[ name ];
	else
		return defaultTheme;
}

void
Syntax::applyFormat( int offset, int length, const KSyntaxHighlighting::Format & format )
{
	m_currentColors.push_back( { m_currentLineNumber, offset, offset + length - 1, format } );
}

Syntax::Colors
Syntax::prepare( const QStringList & lines )
{
	KSyntaxHighlighting::State st;
	m_currentLineNumber = 0;
	m_currentColors.clear();

	for( const auto & s : std::as_const( lines ) )
	{
		st = highlightLine( s, st );
		++m_currentLineNumber;
	}

	return m_currentColors;
}
