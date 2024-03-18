/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QString>
#include <QVector>
#include <QMap>

// KF6SyntaxHighlighting include.
#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>

namespace MdPdf {

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

} /* namespace MdPdf */
