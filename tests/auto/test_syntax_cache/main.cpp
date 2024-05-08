/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// doctest include.
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

// test include.
#include "test_data_path.hpp"

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.hpp>
#include <md4qt/parser.hpp>

// md-editor include.
#include "syntaxvisitor.hpp"
#include "editor.hpp"
#include "colors.hpp"

// Qt include.
#include <QFile>
#include <QApplication>

MdEditor::Editor * g_editor = nullptr;

void prepareTest( const QString & fileName )
{
	QFile f( c_testDataPath + fileName );
	
	if( f.open( QIODevice::ReadOnly ) )
	{
		g_editor->setPlainText( f.readAll() );
		f.close();
		
		MD::Parser< MD::QStringTrait > p;
		auto doc = p.parse( c_testDataPath + fileName );
		
		g_editor->syntaxHighlighter().highlight( doc, {} );
	}
	else
		REQUIRE( false );
}

TEST_CASE( "001" )
{
	prepareTest( QStringLiteral( "001.md" ) );
	REQUIRE( g_editor->syntaxHighlighter().findAllInCache( { 0, 0, 0, 0 } ).empty() );
	REQUIRE( g_editor->syntaxHighlighter().findAllInCache( { 1, 1, 1, 1 } ).empty() );
}

/*
This is just a text!

*/
TEST_CASE( "002" )
{
	prepareTest( QStringLiteral( "002.md" ) );
	auto items = g_editor->syntaxHighlighter().findAllInCache( { 0, 0, 0, 0 } );
	REQUIRE( items.size() == 2 );
	REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
	REQUIRE( items.at( 1 )->type() == MD::ItemType::Text );
	REQUIRE( g_editor->syntaxHighlighter().findAllInCache( { 0, 1, 0, 1 } ).empty() );
}

/*
  
Paragraph 1.

Paragraph 2.

*/
TEST_CASE( "003" )
{
	prepareTest( QStringLiteral( "003.md" ) );
	REQUIRE( g_editor->syntaxHighlighter().findAllInCache( { 0, 0, 0, 0 } ).empty() );
	
	for( int i = 0; i < 2; ++i )
	{
		auto items = g_editor->syntaxHighlighter().findAllInCache( { 0, 1 + i * 2, 0, 1 + i * 2 } );
		REQUIRE( items.size() == 2 );
		REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
		REQUIRE( items.at( 1 )->type() == MD::ItemType::Text );
		auto t = static_cast< MD::Text< MD::QStringTrait >* > ( items.at( 1 ) );
		REQUIRE( t->text() == QStringLiteral( "Paragraph %1." ).arg( QString::number( i + 1 ) ) );
	}
}

/*
Code in the `text`.

*/
TEST_CASE( "012" )
{
	prepareTest( QStringLiteral( "012.md" ) );
	
	{
		auto items = g_editor->syntaxHighlighter().findAllInCache( { 0, 0, 0, 0 } );
		REQUIRE( items.size() == 2 );
		REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
		REQUIRE( items.at( 1 )->type() == MD::ItemType::Text );
	}
	
	{
		auto items = g_editor->syntaxHighlighter().findAllInCache( { 12, 0, 12, 0 } );
		REQUIRE( items.size() == 2 );
		REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
		REQUIRE( items.at( 1 )->type() == MD::ItemType::Code );
	}
	
	{
		auto items = g_editor->syntaxHighlighter().findAllInCache( { 13, 0, 13, 0 } );
		REQUIRE( items.size() == 2 );
		REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
		REQUIRE( items.at( 1 )->type() == MD::ItemType::Code );
	}
	
	{
		auto items = g_editor->syntaxHighlighter().findAllInCache( { 0, 0, 17, 0 } );
		REQUIRE( items.size() == 2 );
		REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
		REQUIRE( items.at( 1 )->type() == MD::ItemType::Text );
	}
	
	{
		auto items = g_editor->syntaxHighlighter().findAllInCache( { 18, 0, 18, 0 } );
		REQUIRE( items.size() == 2 );
		REQUIRE( items.at( 0 )->type() == MD::ItemType::Paragraph );
		REQUIRE( items.at( 1 )->type() == MD::ItemType::Text );
	}
}

int main( int argc, char ** argv )
{
	QApplication app( argc, argv );
	
	MdEditor::Editor editor( nullptr );
	g_editor = &editor;
	
	doctest::Context context;
	context.applyCommandLine( argc, argv );
	
	return context.run();
}
