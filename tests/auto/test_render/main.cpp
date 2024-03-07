
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

#include <src/converter/renderer.hpp>
#include <src/converter/syntax.hpp>

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/parser.hpp>

#include <test_const.hpp>

#include <QObject>
#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QVector>
#include <QFontDatabase>

//
// TestRender
//

class TestRender final
	:	public QObject
{
	Q_OBJECT

private slots:
	//! Init tests.
	void initTestCase();
	//! Test footnotes rendering.
	void testFootnotes();
	//! Test table with images.
	void testTableWithImage();
	//! Test table with text.
	void testTableWithText();
	//! Test blockquotes.
	void testBlockquote();
	//! Complex test.
	void testComplex();
	//! Test image in text.
	void testImageInText();
	//! Footnote with table.
	void testFootnoteWithTable();
	//! Complex footnote.
	void testComplexFootnote();
	//! One more complex test.
	void testComplex3();

	//! Test footnotes rendering.
	void testFootnotesBigFont();
	//! Test table with images.
	void testTableWithImageBigFont();
	//! Test table with text.
	void testTableWithTextBigFont();
	//! Test blockquotes.
	void testBlockquoteBigFont();
	//! Complex test.
	void testComplexBigFont();
	//! Test image in text.
	void testImageInTextBigFont();
	//! Footnote with table.
	void testFootnoteWithTableBigFont();
	//! Complex footnote.
	void testComplexFootnoteBigFont();
	//! One more complex test.
	void testComplex3BigFont();
	//! Very long footnote.
	void testVeryLongFootnote();
	//! Very long footnote.
	void testVeryLongFootnoteBigFont();

	//! Test code.
	void testCode();
	//! Test complex 2.
	void testComplex2();

	//! Test task list.
	void testTaskList();
	//! Test task list.
	void testTaskListBigFont();

	//! Test link with inline content.
	void testLinks();
	//! Test link with inline content.
	void testLinksBigFont();

	//! Test math.
	void testMath();
	//! Test math.
	void testMathBigFont();
}; // class TestRender

//! Prepare test data or do actual test?
static const bool c_printData = false;

struct TestRendering {
	static void
	testRendering( const QString & fileName, const QString & suffix,
		const QVector< DrawPrimitive > & data, double textFontSize, double codeFontSize )
	{
		MD::Parser< MD::QStringTrait > parser;

		auto doc = parser.parse( c_folder + QStringLiteral( "/../../manual/" ) + fileName, true );

		RenderOpts opts;
		opts.m_borderColor = QColor( 81, 81, 81 );
		opts.m_linkColor = QColor( 33, 122, 255 );
		opts.m_bottom = 50.0;
		opts.m_syntax = std::make_shared< Syntax > ();
		opts.m_syntax->setTheme( opts.m_syntax->themeForName( QStringLiteral( "GitHub Light" ) ) );
		opts.m_codeFont = QStringLiteral( "Courier New" );
		opts.m_codeFontSize = codeFontSize;
		opts.m_left = 50.0;
		opts.m_linkColor = QColor( 33, 122, 255 );
		opts.m_right = 50.0;
		opts.m_textFont = QStringLiteral( "Droid Serif" );
		opts.m_textFontSize = textFontSize;
		opts.m_mathFont = QStringLiteral( "Droid Serif" );
		opts.m_mathFontSize = textFontSize;
		opts.m_top = 50.0;
		opts.m_dpi = 150;

		opts.testData = data;
		opts.printDrawings = c_printData;
		opts.testDataFileName = c_folder + QStringLiteral( "/" ) + fileName + suffix +
			QStringLiteral( ".data" );

		PdfRenderer render;

		render.render( QStringLiteral( "./" ) + fileName + suffix + QStringLiteral( ".pdf" ),
			doc, opts );
		render.renderImpl();

		if( render.isError() )
			QFAIL( "Rendering of Markdown failed. Test aborted." );
	}
};

DrawPrimitive::Type
toType( const QString & t )
{
	if( t == QStringLiteral( "Text" ) )
		return DrawPrimitive::Type::Text;
	else if( t == QStringLiteral( "MultilineText" ) )
		return DrawPrimitive::Type::MultilineText;
	else if( t == QStringLiteral( "Line" ) )
		return DrawPrimitive::Type::Line;
	else if( t == QStringLiteral( "Rectangle" ) )
		return DrawPrimitive::Type::Rectangle;
	else if( t == QStringLiteral( "Image" ) )
		return DrawPrimitive::Type::Image;
	else
		return DrawPrimitive::Type::Unknown;
}

QString
readQuotedString( QTextStream & s, int length )
{
	QString ret;
	QChar c;

	while( c != QLatin1Char( '"' ) )
		s >> c;

	for( int i = 0; i < length; ++i )
	{
		s >> c;
		ret.append( c );
	}

	s >> c;

	return ret;
}

QVector< DrawPrimitive >
loadTestData( const QString & fileName, const QString & suffix )
{
	QVector< DrawPrimitive > data;

	QFile file( c_folder + QStringLiteral( "/" ) + fileName + suffix + QStringLiteral( ".data" ) );

	if( !file.open( QIODevice::ReadOnly ) )
		return data;

	QTextStream stream( &file );

	while( !stream.atEnd() )
	{
		auto str = stream.readLine();

		if( str.isEmpty() )
			break;

		QTextStream s( &str );
		DrawPrimitive d;

		QString type;
		s >> type;
		d.type = toType( type );

		int length = 0;
		s >> length;

		d.text = readQuotedString( s, length );

		s >> d.x;
		s >> d.y;
		s >> d.x2;
		s >> d.y2;
		s >> d.width;
		s >> d.height;
		s >> d.xScale;
		s >> d.yScale;

		data.append( d );
	}

	file.close();

	return data;
}

void
doTest( const QString & fileName, const QString & suffix,
	double textFontSize, double codeFontSize )
{
	QVector< DrawPrimitive > data;

	if( !c_printData )
	{
		data = loadTestData( fileName, suffix );

		if( data.isEmpty() )
			QFAIL( "Failed to load test data." );
	}

	TestRendering::testRendering( fileName, suffix, data, textFontSize, codeFontSize );
}

void
TestRender::initTestCase()
{
	QFontDatabase::addApplicationFont( c_font );
	QFontDatabase::addApplicationFont( c_italicFont );
	QFontDatabase::addApplicationFont( c_boldFont );
	QFontDatabase::addApplicationFont( c_boldItalicFont );
	QFontDatabase::addApplicationFont( c_monoFont );
	QFontDatabase::addApplicationFont( c_monoItalicFont );
	QFontDatabase::addApplicationFont( c_monoBoldFont );
	QFontDatabase::addApplicationFont( c_monoBoldItalicFont );
}

void
TestRender::testFootnotes()
{
	doTest( QStringLiteral( "footnotes.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testTableWithImage()
{
	doTest( QStringLiteral( "table.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testTableWithText()
{
	doTest( QStringLiteral( "table2.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testBlockquote()
{
	doTest( QStringLiteral( "blockquote.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testComplex()
{
	doTest( QStringLiteral( "complex.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testImageInText()
{
	doTest( QStringLiteral( "image_in_text.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testFootnoteWithTable()
{
	doTest( QStringLiteral( "footnote2.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testComplexFootnote()
{
	doTest( QStringLiteral( "footnote3.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testComplex3()
{
	doTest( QStringLiteral( "complex3.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testFootnotesBigFont()
{
	doTest( QStringLiteral( "footnotes.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testTableWithImageBigFont()
{
	doTest( QStringLiteral( "table.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testTableWithTextBigFont()
{
	doTest( QStringLiteral( "table2.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testBlockquoteBigFont()
{
	doTest( QStringLiteral( "blockquote.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testComplexBigFont()
{
	doTest( QStringLiteral( "complex.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testImageInTextBigFont()
{
	doTest( QStringLiteral( "image_in_text.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testFootnoteWithTableBigFont()
{
	doTest( QStringLiteral( "footnote2.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testComplexFootnoteBigFont()
{
	doTest( QStringLiteral( "footnote3.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testComplex3BigFont()
{
	doTest( QStringLiteral( "complex3.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testCode()
{
	doTest( QStringLiteral( "code.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testComplex2()
{
	doTest( QStringLiteral( "complex2.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testLinks()
{
	doTest( QStringLiteral( "links.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testLinksBigFont()
{
	doTest( QStringLiteral( "links.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testTaskList()
{
	doTest( QStringLiteral( "tasklist.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testTaskListBigFont()
{
	doTest( QStringLiteral( "tasklist.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testMath()
{
//	doTest( QStringLiteral( "math.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testMathBigFont()
{
//	doTest( QStringLiteral( "math.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void
TestRender::testVeryLongFootnote()
{
	doTest( QStringLiteral( "footnotes4.md" ), QString(), 8.0, 8.0 );
}

void
TestRender::testVeryLongFootnoteBigFont()
{
	doTest( QStringLiteral( "footnotes4.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

QTEST_MAIN( TestRender )

#include "main.moc"
