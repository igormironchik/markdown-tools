
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
#include "renderer.hpp"
#include "const.hpp"
#include "podofo_paintdevice.hpp"

#ifdef MD_PDF_TESTING
#include <test_const.hpp>
#include <QtTest/QtTest>
#endif // MD_PDF_TESTING

// Qt include.
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QThread>
#include <QBuffer>
#include <QTemporaryFile>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QRegularExpression>

// Magick++ include.
#include <Magick++.h>

// JKQtPlotter include.
#include <jkqtmathtext/jkqtmathtext.h>

// C++ include.
#include <cmath>
#include <utility>
#include <functional>


//
// PdfRendererError
//

//! Internal exception.
class PdfRendererError {
public:
	explicit PdfRendererError( const QString & reason )
		:	m_what( reason )
	{
	}

	const QString & what() const noexcept
	{
		return m_what;
	}

private:
	QString m_what;
}; // class PdfRendererError


//
// PdfAuxData
//

double
PdfAuxData::topY( int page ) const
{
	if( !drawFootnotes )
		return coords.pageHeight - coords.margins.top;
	else
		return topFootnoteY( page );
}

int
PdfAuxData::currentPageIndex() const
{
	if( !drawFootnotes )
		return currentPageIdx;
	else
		return footnotePageIdx;
}

double
PdfAuxData::topFootnoteY( int page ) const
{
	if( reserved.contains( page ) )
		return reserved[ page ];
	else
		return 0.0;
}

double
PdfAuxData::currentPageAllowedY() const
{
	return allowedY( currentPageIdx );
}

double
PdfAuxData::allowedY( int page ) const
{
	if( !drawFootnotes )
	{
		if( reserved.contains( page ) )
			return reserved[ page ];
		else
			return coords.margins.bottom;
	}
	else
		return coords.margins.bottom;
}

void
PdfAuxData::freeSpaceOn( int page )
{
	if( !drawFootnotes )
	{
		if( reserved.contains( page ) )
		{
			double r = reserved[ page ];
			reserved.remove( page );

			if( page == footnotePageIdx )
				footnotePageIdx = page + 1;

			while( reserved.contains( ++page ) )
			{
				const double tmp = reserved[ page ];
				reserved[ page ] = r;
				r = tmp;
			}

			reserved[ page ] = r;
		}
	}
}

void
PdfAuxData::drawText( double x, double y, const char * text,
	Font * font, double size, double scale, bool strikeout )
{
	firstOnPage = false;

#ifndef MD_PDF_TESTING
	(*painters)[ currentPainterIdx ]->TextObject.Begin();
	(*painters)[ currentPainterIdx ]->TextObject.MoveTo( x, y );
	(*painters)[ currentPainterIdx ]->TextState.SetFont( *font, size );
	(*painters)[ currentPainterIdx ]->TextState.SetFontScale( scale );
	const auto st = (*painters)[ currentPainterIdx ]->TextState;
	(*painters)[ currentPainterIdx ]->TextObject.AddText( text );
	(*painters)[ currentPainterIdx ]->TextObject.End();

	if( strikeout )
	{
		(*painters)[ currentPainterIdx ]->Save();

		(*painters)[ currentPainterIdx ]->GraphicsState.SetLineWidth( font->GetStrikeThroughThickness( st ) );

		(*painters)[ currentPainterIdx ]->DrawLine( x,
			y + font->GetStrikeThroughPosition( st ),
			x + font->GetStringLength( text, st ),
			y + font->GetStrikeThroughPosition( st ) );

		(*painters)[ currentPainterIdx ]->Restore();
	}
#else
	if( printDrawings )
	{
		const auto s = PdfRenderer::createQString( text );

		(*drawingsStream) << QStringLiteral(
			"Text %1 \"%2\" %3 %4 0.0 0.0 0.0 0.0 0.0 0.0\n" )
				.arg( QString::number( s.length() ), s,
					QString::number( x, 'f', 16 ),
					QString::number( y, 'f', 16 ) );
	}
	else
	{
		(*painters)[ currentPainterIdx ]->TextObject.Begin();
		(*painters)[ currentPainterIdx ]->TextObject.MoveTo( x, y );
		(*painters)[ currentPainterIdx ]->TextState.SetFont( *font, size );
		(*painters)[ currentPainterIdx ]->TextState.SetFontScale( scale );
		const auto st = (*painters)[ currentPainterIdx ]->TextState;
		(*painters)[ currentPainterIdx ]->TextObject.AddText( text );
		(*painters)[ currentPainterIdx ]->TextObject.End();

		if( strikeout )
		{
			(*painters)[ currentPainterIdx ]->Save();

			(*painters)[ currentPainterIdx ]->GraphicsState.SetLineWidth( font->GetStrikeThroughThickness( st ) );

			(*painters)[ currentPainterIdx ]->DrawLine( x,
				y + font->GetStrikeThroughPosition( st ),
				x + font->GetStringLength( text, st ),
				y + font->GetStrikeThroughPosition( st ) );

			(*painters)[ currentPainterIdx ]->Restore();
		}

		if( QTest::currentTestFailed() )
			self->terminate();

		int pos = testPos++;
		QCOMPARE( DrawPrimitive::Type::Text, testData.at( pos ).type );
		QCOMPARE( PdfRenderer::createQString( text ), testData.at( pos ).text );
		QCOMPARE( x, testData.at( pos ).x );
		QCOMPARE( y, testData.at( pos ).y );
	}
#endif // MD_PDF_TESTING
}

void
PdfAuxData::drawImage( double x, double y, Image * img, double xScale, double yScale )
{
	firstOnPage = false;

#ifndef MD_PDF_TESTING
	(*painters)[ currentPainterIdx ]->DrawImage( *img, x, y, xScale, yScale );
#else
	if( printDrawings )
		(*drawingsStream) << QStringLiteral(
			"Image 0 \"\" %2 %3 0.0 0.0 0.0 0.0 %4 %5\n" )
				.arg( QString::number( x, 'f', 16 ), QString::number( y, 'f', 16 ),
					QString::number( xScale, 'f', 16 ), QString::number( yScale, 'f', 16 ) );
	else
	{
		(*painters)[ currentPainterIdx ]->DrawImage( *img, x, y, xScale, yScale );

		if( QTest::currentTestFailed() )
			self->terminate();

		int pos = testPos++;
		QCOMPARE( x, testData.at( pos ).x );
		QCOMPARE( y, testData.at( pos ).y );
		QCOMPARE( xScale, testData.at( pos ).xScale );
		QCOMPARE( yScale, testData.at( pos ).yScale );
	}
#endif // MD_PDF_TESTING
}

void
PdfAuxData::drawLine( double x1, double y1, double x2, double y2 )
{
#ifndef MD_PDF_TESTING
	(*painters)[ currentPainterIdx ]->DrawLine( x1, y1, x2, y2 );
#else
	if( printDrawings )
		(*drawingsStream) << QStringLiteral(
			"Line 0 \"\" %1 %2 %3 %4 0.0 0.0 0.0 0.0\n" )
				.arg( QString::number( x1, 'f', 16 ), QString::number( y1, 'f', 16 ),
					QString::number( x2, 'f', 16 ), QString::number( y2, 'f', 16 ) );
	else
	{
		(*painters)[ currentPainterIdx ]->DrawLine( x1, y1, x2, y2 );

		if( QTest::currentTestFailed() )
			self->terminate();

		int pos = testPos++;
		QCOMPARE( x1, testData.at( pos ).x );
		QCOMPARE( y1, testData.at( pos ).y );
		QCOMPARE( x2, testData.at( pos ).x2 );
		QCOMPARE( y2, testData.at( pos ).y2 );
	}
#endif // MD_PDF_TESTING
}

void
PdfAuxData::save( const QString & fileName )
{
#ifndef MD_PDF_TESTING
	doc->Save( fileName.toLocal8Bit().data() );
#else
	if( !printDrawings )
		doc->Save( fileName.toLocal8Bit().data() );
#endif // MD_PDF_TESTING
}

void
PdfAuxData::drawRectangle( double x, double y, double width, double height, PoDoFo::PdfPathDrawMode m )
{
#ifndef MD_PDF_TESTING
	(*painters)[ currentPainterIdx ]->DrawRectangle( x, y, width, height, m );
#else
	if( printDrawings )
		(*drawingsStream) << QStringLiteral(
			"Rectangle 0 \"\" %1 %2 0.0 0.0 %3 %4 0.0 0.0\n" )
				.arg( QString::number( x, 'f', 16 ), QString::number( y, 'f', 16 ),
					QString::number( width, 'f', 16 ), QString::number( height, 'f', 16 ) );
	else
	{
		(*painters)[ currentPainterIdx ]->DrawRectangle( x, y, width, height, m );

		if( QTest::currentTestFailed() )
			self->terminate();

		int pos = testPos++;
		QCOMPARE( x, testData.at( pos ).x );
		QCOMPARE( y, testData.at( pos ).y );
		QCOMPARE( width, testData.at( pos ).width );
		QCOMPARE( height, testData.at( pos ).height );
	}
#endif // MD_PDF_TESTING
}

void
PdfAuxData::setColor( const QColor & c )
{
	colorsStack.push( c );

	(*painters)[ currentPainterIdx ]->GraphicsState.SetFillColor( Color( c.redF(), c.greenF(), c.blueF() ) );
	(*painters)[ currentPainterIdx ]->GraphicsState.SetStrokeColor( Color( c.redF(), c.greenF(), c.blueF() ) );
}

void
PdfAuxData::restoreColor()
{
	if( colorsStack.size() > 1 )
		colorsStack.pop();

	repeatColor();
}

void
PdfAuxData::repeatColor()
{
	const auto & c = colorsStack.top();

	(*painters)[ currentPainterIdx ]->GraphicsState.SetFillColor( Color( c.redF(), c.greenF(), c.blueF() ) );
	(*painters)[ currentPainterIdx ]->GraphicsState.SetStrokeColor( Color( c.redF(), c.greenF(), c.blueF() ) );
}

double
PdfAuxData::imageWidth( const QByteArray & image )
{
	auto pdfImg = doc->CreateImage();
	pdfImg->LoadFromBuffer( { image.data(), static_cast< size_t > ( image.size() ) } );

	return std::round( (double) pdfImg->GetWidth() / (double) dpi * 72.0 );
}

double
PdfAuxData::stringWidth( Font * font, double size, double scale, const String & s ) const
{
	PoDoFo::PdfTextState st;
	st.FontSize = size * scale;

	return font->GetStringLength( s, st );
}

double
PdfAuxData::lineSpacing( Font * font, double size, double scale ) const
{
	PoDoFo::PdfTextState st;
	st.FontSize = size * scale;

	return font->GetLineSpacing( st );
}

double
PdfAuxData::fontAscent( Font * font, double size, double scale ) const
{
	PoDoFo::PdfTextState st;
	st.FontSize = size * scale;

	return font->GetAscent( st );
}

double
PdfAuxData::fontDescent( Font * font, double size, double scale ) const
{
	PoDoFo::PdfTextState st;
	st.FontSize = size * scale;

	return font->GetDescent( st );
}


//
// PdfRenderer::CustomWidth
//

double
PdfRenderer::CustomWidth::firstItemHeight() const
{
	if( !m_width.isEmpty() )
		return m_width.constFirst().height;
	else
		return 0.0;
}

void
PdfRenderer::CustomWidth::calcScale( double lineWidth )
{
	double w = 0.0;
	double sw = 0.0;
	double ww = 0.0;
	double h = 0.0;
	double d = 0.0;

	for( int i = 0, last = m_width.size(); i < last; ++i )
	{
		if( m_width.at( i ).height > h )
		{
			h = m_width.at( i ).height;
			d = m_width.at( i ).descent;
		}

		w += m_width.at( i ).width;

		if( m_width.at( i ).isSpace )
			sw += m_width.at( i ).width;
		else
			ww += m_width.at( i ).width;

		if( m_width.at( i ).isNewLine )
		{
			if( m_width.at( i ).shrink )
			{
				auto ss = ( lineWidth - ww ) / sw;

				while( ww + sw * ss > lineWidth )
					ss -= 0.001;

				m_scale.append( 100.0 * ss );
			}
			else
				m_scale.append( 100.0 );

			m_height.append( h );
			m_descent.append( d );
			m_images.append( m_width.at( i ).isImage );

			w = 0.0;
			sw = 0.0;
			ww = 0.0;
			h = 0.0;
			d = 0.0;
		}
	}
}


//
// PdfRenderer::CellItem
//

double
PdfRenderer::CellItem::width( PdfAuxData & pdfData, PdfRenderer * render, double scale ) const
{
	auto * f = render->createFont( font.family, font.bold, font.italic,
		font.size, pdfData.doc, scale, pdfData );

	if( !word.isEmpty() )
		return pdfData.stringWidth( f, font.size, scale, createUtf8String( word ) );
	else if( !image.isNull() )
		return pdfData.imageWidth( image );
	else if( !url.isEmpty() )
		return pdfData.stringWidth( f, font.size, scale, createUtf8String( url ) );
	else if( !footnote.isEmpty() )
		return pdfData.stringWidth( f, font.size * c_footnoteScale, scale, createUtf8String( footnote ) );
	else
		return 0.0;
}


//
// PdfRenderer::CellData
//

void
PdfRenderer::CellData::heightToWidth( double lineHeight, double spaceWidth, double scale,
	PdfAuxData & pdfData, PdfRenderer * render )
{
	height = 0.0;

	bool newLine = true;

	double w = 0.0;

	bool addMargin = false;

	for( auto it = items.cbegin(), last = items.cend(); it != last; ++it )
	{
		if( it->image.isNull() )
		{
			addMargin = false;

			if( newLine )
			{
				height += lineHeight;
				newLine = false;
				w = 0.0;
			}

			w += it->width( pdfData, render, scale );

			if( w >= width )
			{
				newLine = true;
				continue;
			}

			double sw = spaceWidth;

			if( it != items.cbegin() && it->font != ( it - 1 )->font )
				sw = pdfData.stringWidth( render->createFont( it->font.family,
						it->font.bold, it->font.italic, it->font.size, pdfData.doc, scale, pdfData ),
					it->font.size, scale, String( " " ) );

			if( it + 1 != last && !( it + 1 )->footnote.isEmpty() )
				sw = 0.0;

			if( it + 1 != last )
			{
				if( w + sw + ( it + 1 )->width( pdfData, render, scale ) > width )
					newLine = true;
				else
				{
					w += sw;
					newLine = false;
				}
			}
		}
		else
		{
			auto pdfImg = pdfData.doc->CreateImage();
			pdfImg->LoadFromBuffer( { it->image.data(),
				static_cast< size_t > ( it->image.size() ) } );

			const double iWidth = std::round( (double) pdfImg->GetWidth() /
				(double) pdfData.dpi * 72.0 );
			const double iHeight = std::round( (double) pdfImg->GetHeight() /
				(double) pdfData.dpi * 72.0 );

			if( iWidth > width )
				height += iHeight / ( iWidth / width ) * scale;
			else
				height += iHeight * scale;

			newLine = true;

			if( !addMargin )
				addMargin = true;
			else
				continue;
		}

		if( addMargin )
			height += c_tableMargin;
	}
}


//
// PdfRenderer
//

PdfRenderer::PdfRenderer()
	:	m_terminate( false )
	,	m_footnoteNum( 1 )
#ifdef MD_PDF_TESTING
	,	m_isError( false )
#endif
{
	connect( this, &PdfRenderer::start, this, &PdfRenderer::renderImpl,
		Qt::QueuedConnection );
}

void
PdfRenderer::render( const QString & fileName,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	const RenderOpts & opts, bool testing )
{
	m_fileName = fileName;
	m_doc = doc;
	m_opts = opts;

	if( !testing )
		emit start();
}

void
PdfRenderer::terminate()
{
	QMutexLocker lock( &m_mutex );

	m_terminate = true;

#ifdef MD_PDF_TESTING
	QFAIL( "Test terminated." );
#endif
}

#ifdef MD_PDF_TESTING
bool
PdfRenderer::isError() const
{
	return m_isError;
}
#endif

void
PdfRenderer::renderImpl()
{
	PdfAuxData pdfData;

	try
	{
		const int itemsCount = m_doc->items().size();

		emit progress( 0 );
		emit status( tr( "Rendering PDF..." ) );

		Document document;
		std::vector< std::shared_ptr< Painter > > painters;

		pdfData.doc = &document;
		pdfData.painters = &painters;

		pdfData.coords.margins.left = m_opts.m_left;
		pdfData.coords.margins.right = m_opts.m_right;
		pdfData.coords.margins.top = m_opts.m_top;
		pdfData.coords.margins.bottom = m_opts.m_bottom;
		pdfData.dpi = m_opts.m_dpi;
		pdfData.syntax = m_opts.m_syntax;

		pdfData.colorsStack.push( Qt::black );

		pdfData.md = m_doc;

#ifdef MD_PDF_TESTING
		pdfData.fonts[ QStringLiteral( "Droid Serif" ) ] = c_font;
		pdfData.fonts[ QStringLiteral( "Droid Serif Bold" ) ] = c_boldFont;
		pdfData.fonts[ QStringLiteral( "Droid Serif Italic" ) ] = c_italicFont;
		pdfData.fonts[ QStringLiteral( "Droid Serif Bold Italic" ) ] = c_boldItalicFont;
		pdfData.fonts[ QStringLiteral( "Courier New" ) ] = c_monoFont;
		pdfData.fonts[ QStringLiteral( "Courier New Italic" ) ] = c_monoItalicFont;
		pdfData.fonts[ QStringLiteral( "Courier New Bold" ) ] = c_monoBoldFont;
		pdfData.fonts[ QStringLiteral( "Courier New Bold Italic" ) ] = c_monoBoldItalicFont;

		pdfData.self = this;

		if( m_opts.printDrawings )
		{
			pdfData.printDrawings = true;

			pdfData.drawingsFile.reset( new QFile( m_opts.testDataFileName ) );
			if( !pdfData.drawingsFile->open( QIODevice::WriteOnly ) )
				QFAIL( "Unable to open file for dump drawings." );

			pdfData.drawingsStream.reset( new QTextStream( pdfData.drawingsFile.get() ) );
		}
		else
			pdfData.testData = m_opts.testData;
#endif // MD_PDF_TESTING

		int itemIdx = 0;

		pdfData.extraInFootnote = pdfData.lineSpacing(
			createFont( m_opts.m_textFont, false, false, m_opts.m_textFontSize,
				pdfData.doc, 1.0, pdfData ), m_opts.m_textFontSize, 1.0 ) / 3.0;

		createPage( pdfData );

		for( auto it = m_doc->items().cbegin(), last = m_doc->items().cend(); it != last; ++it )
		{
			switch( (*it)->type() )
			{
				case MD::ItemType::Anchor :
					pdfData.anchors.push_back(
						static_cast< MD::Anchor< MD::QStringTrait >* > ( it->get() )->label() );

				default:
					break;
			}
		}

		for( auto it = m_doc->items().cbegin(), last = m_doc->items().cend(); it != last; ++it )
		{
			++itemIdx;

			{
				QMutexLocker lock( &m_mutex );

				if( m_terminate )
					break;
			}

			switch( (*it)->type() )
			{
				case MD::ItemType::Heading :
					drawHeading( pdfData, m_opts,
						static_cast< MD::Heading< MD::QStringTrait >* > ( it->get() ),
						m_doc, 0.0,
						// If there is another item after heading we need to know its min
						// height to glue heading with it.
						( it + 1 != last ?
							minNecessaryHeight( pdfData, m_opts, *( it + 1 ), m_doc, 0.0,
								1.0 ) :
							0.0 ), CalcHeightOpt::Unknown, 1.0 );
					break;

				case MD::ItemType::Paragraph :
					drawParagraph( pdfData, m_opts,
						static_cast< MD::Paragraph< MD::QStringTrait >* > ( it->get() ),
						m_doc, 0.0, true, CalcHeightOpt::Unknown, 1.0 );
					break;

				case MD::ItemType::Code :
					drawCode( pdfData, m_opts,
						static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
						m_doc, 0.0, CalcHeightOpt::Unknown, 1.0 );
					break;

				case MD::ItemType::Blockquote :
					drawBlockquote( pdfData, m_opts,
						static_cast< MD::Blockquote< MD::QStringTrait >* > ( it->get() ),
						m_doc, 0.0, CalcHeightOpt::Unknown, 1.0 );
					break;

				case MD::ItemType::List :
				{
					auto * list = static_cast< MD::List< MD::QStringTrait >* > ( it->get() );
					const auto bulletWidth = maxListNumberWidth( list );

					drawList( pdfData, m_opts, list, m_doc, bulletWidth );
				}
					break;

				case MD::ItemType::Table :
					drawTable( pdfData, m_opts,
						static_cast< MD::Table< MD::QStringTrait >* > ( it->get() ),
						m_doc, 0.0, CalcHeightOpt::Unknown, 1.0 );
					break;

				case MD::ItemType::PageBreak :
				{
					if( itemIdx < itemsCount )
						createPage( pdfData );
				}
					break;

				case MD::ItemType::Anchor :
				{
					auto * a = static_cast< MD::Anchor< MD::QStringTrait >* > ( it->get() );
					m_dests.insert( a->label(),
						std::make_shared< Destination > ( *pdfData.page,
							pdfData.coords.margins.left,
							pdfData.coords.pageHeight - pdfData.coords.margins.top, 0.0 ) );
					pdfData.currentFile = a->label();
				}
					break;

				default :
					break;
			}

			emit progress( static_cast< int > ( static_cast< double > (itemIdx) /
				static_cast< double > (itemsCount) * 100.0 ) );
		}

		if( !m_footnotes.isEmpty() )
		{
			pdfData.drawFootnotes = true;
			pdfData.coords.x = pdfData.coords.margins.left;
			pdfData.coords.y = pdfData.topFootnoteY( pdfData.reserved.firstKey() ) -
				pdfData.extraInFootnote;

			pdfData.currentPainterIdx = pdfData.reserved.firstKey();
			pdfData.footnotePageIdx = pdfData.reserved.firstKey();

			drawHorizontalLine( pdfData, m_opts );

			for( const auto & f : std::as_const( m_footnotes ) )
				drawFootnote( pdfData, m_opts, m_doc, f.first, f.second.get(),
					CalcHeightOpt::Unknown );
		}

		resolveLinks( pdfData );

		finishPages( pdfData );

		emit status( tr( "Saving PDF..." ) );

		pdfData.save( m_fileName );

		emit done( m_terminate );

#ifdef MD_PDF_TESTING
		if( m_opts.printDrawings )
			pdfData.drawingsFile->close();

		if( pdfData.testPos != pdfData.testData.size() )
			m_isError = true;
#endif // MD_PDF_TESTING
	}
	catch( const PoDoFo::PdfError & e )
	{
		const auto & cs = e.GetCallStack();
		QString msg;

		for( const auto & i : cs )
		{
			auto filepath = i.GetFilePath();

			if( !filepath.empty() )
				msg.append( QStringLiteral( " Error Source : " ) );
				msg.append( QString::fromUtf8( filepath ) );
				msg.append( QStringLiteral( ": " ) );
				msg.append( QString::number( i.GetLine() ) );
				msg.append( QStringLiteral( "\n" ) );

			if( !i.GetInformation().empty() )
				msg.append( QStringLiteral( "Information: " ) );
				msg.append( QString::fromUtf8( i.GetInformation() ) );
				msg.append( QStringLiteral( "\n" ) );
		}

		handleException( pdfData,
			QString::fromLatin1( "Error during drawing PDF:\n%1" ).arg( msg ) );
	}
	catch( const PdfRendererError & e )
	{
		handleException( pdfData, e.what() );
	}
	catch( const std::exception & e )
	{
		handleException( pdfData, QString::fromLatin1( "Error during drawing PDF: %1" )
			.arg( e.what() ) );
	}
	catch( ... )
	{
		handleException( pdfData, QStringLiteral( "Error during drawing PDF." ) );
	}

	try {
		clean();
	}
	catch( const std::exception & e )
	{
#ifdef MD_PDF_TESTING
		m_isError = true;
#endif
		emit error( QString::fromLatin1( "Error freeing memory after all operations: %1" )
			.arg( e.what() ) );
	}

	deleteLater();
}

void
PdfRenderer::handleException( PdfAuxData & pdfData, const QString & msg )
{
#ifdef MD_PDF_TESTING
	m_isError = true;
#endif

	const auto fullMsg = QStringLiteral( "%1\n\nError occured in the \"%2\" file, "
		"between start position %3 on line %4 and end position %5 on line %6." )
			.arg( msg )
			.arg( pdfData.currentFile )
			.arg( pdfData.startPos + 1 )
			.arg( pdfData.startLine + 1 )
			.arg( pdfData.endPos + 1 )
			.arg( pdfData.endLine + 1 );

	emit error( fullMsg );
}

void
PdfRenderer::finishPages( PdfAuxData & pdfData )
{
	for( const auto & p: *pdfData.painters )
		p->FinishDrawing();
}

void
PdfRenderer::clean()
{
	m_dests.clear();
	m_unresolvedLinks.clear();
	m_unresolvedFootnotesLinks.clear();
}

double
PdfRenderer::rowHeight( const QVector< QVector< CellData > > & table, int row )
{
	double h = 0.0;

	for( auto it = table.cbegin(), last = table.cend(); it != last; ++it )
	{
		if( (*it)[ row ].height > h )
			h = (*it)[ row ].height;
	}

	return  h;
}

void
PdfRenderer::resolveLinks( PdfAuxData & pdfData )
{
	for( auto it = m_unresolvedLinks.cbegin(), last = m_unresolvedLinks.cend(); it != last; ++it )
	{
		if( m_dests.contains( it.key() ) )
		{
			for( const auto & r : std::as_const( it.value() ) )
			{
				auto & page = pdfData.doc->GetPages().GetPageAt( r.second );
				auto & annot = page.GetAnnotations().CreateAnnot< PoDoFo::PdfAnnotationLink >(
					Rect( r.first.x(), r.first.y(), r.first.width(), r.first.height() ) );
				annot.SetBorderStyle( 0.0, 0.0, 0.0 );
				annot.SetDestination( m_dests.value( it.key() ) );
			}
		}
#ifdef MD_PDF_TESTING
		else
		{
			terminate();

			QFAIL( "Unresolved link." );
		}
#endif // MD_PDF_TESTING
	}

	for( auto it = m_unresolvedFootnotesLinks.cbegin(),
		last = m_unresolvedFootnotesLinks.cend(); it != last; ++it )
	{
		if( m_dests.contains( it.key() ) )
		{
			auto & page = pdfData.doc->GetPages().GetPageAt( it.value().second );
			auto & annot = page.GetAnnotations().CreateAnnot< PoDoFo::PdfAnnotationLink >(
				Rect( it.value().first.x(), it.value().first.y(),
					it.value().first.width(), it.value().first.height() ) );
			annot.SetBorderStyle( 0.0, 0.0, 0.0 );
			annot.SetDestination( m_dests.value( it.key() ) );
		}
#ifdef MD_PDF_TESTING
		else
		{
			terminate();

			QFAIL( "Unresolved footnote link." );
		}
#endif // MD_PDF_TESTING
	}
}

Font *
PdfRenderer::createFont( const QString & name, bool bold, bool italic, double size,
	Document * doc, double scale, const PdfAuxData & pdfData )
{
	PoDoFo::PdfFontSearchParams params;
	params.Style = PoDoFo::PdfFontStyle::Regular;
	if( bold ) params.Style.value() |= PoDoFo::PdfFontStyle::Bold;
	if( italic ) params.Style.value() |= PoDoFo::PdfFontStyle::Italic;

#ifdef MD_PDF_TESTING
	const QString internalName = name + ( bold ? QStringLiteral( " Bold" ) : QString() ) +
		( italic ? QStringLiteral( " Italic" ) : QString() );

	auto & font = doc->GetFonts().GetOrCreateFont(
		pdfData.fonts[ internalName ].toLocal8Bit().data() );

	return &font;
#else
	Q_UNUSED( pdfData )

	auto * font = doc->GetFonts().SearchFont( name.toLocal8Bit().data(), params );

	if( !font )
		throw PdfRendererError( tr( "Unable to create font: %1. Please choose another one.\n\n"
			"This application uses PoDoFo C++ library to create PDF. And not all fonts supported by Qt "
			"are supported by PoDoFo. I'm sorry for the inconvenience." )
				.arg( name ) );

	return font;
#endif // MD_PDF_TESTING
}

bool
PdfRenderer::isFontCreatable( const QString & name )
{
	Document doc;

	PoDoFo::PdfFontSearchParams params;
	params.Style = PoDoFo::PdfFontStyle::Regular;

	auto font = doc.GetFonts().SearchFont( name.toLocal8Bit().data(), params );

	return ( font != nullptr );
}

void
PdfRenderer::createPage( PdfAuxData & pdfData )
{
	std::function< void ( PdfAuxData & ) > create;

	create = [&create] ( PdfAuxData & pdfData )
	{
		pdfData.page = &pdfData.doc->GetPages().CreatePage(
			Page::CreateStandardPageSize( PoDoFo::PdfPageSize::A4 ) );

		if( !pdfData.page )
			throw PdfRendererError( QLatin1String( "Oops, can't create empty page in PDF.\n\n"
				"This is very strange, it should not appear ever, but it is. "
				"I'm sorry for the inconvenience." ) );

		pdfData.firstOnPage = true;

		auto painter = std::make_shared< Painter > ();
		painter->SetCanvas( *pdfData.page );

		(*pdfData.painters).push_back( painter );
		pdfData.currentPainterIdx = pdfData.painters->size() - 1;

		pdfData.coords = { { pdfData.coords.margins.left, pdfData.coords.margins.right,
				pdfData.coords.margins.top, pdfData.coords.margins.bottom },
			pdfData.page->GetRect().Width,
			pdfData.page->GetRect().Height,
			pdfData.coords.margins.left, pdfData.page->GetRect().Height -
				pdfData.coords.margins.top };

		++pdfData.currentPageIdx;

		const auto topY = pdfData.topFootnoteY( pdfData.currentPageIdx );

		if( pdfData.coords.pageHeight - pdfData.coords.margins.top -
			topY - pdfData.extraInFootnote < pdfData.lineHeight )
				create( pdfData );
	};

	if( !pdfData.drawFootnotes )
		create( pdfData );
	else
	{
		auto it = pdfData.reserved.find( ++pdfData.footnotePageIdx );

		if( it == pdfData.reserved.cend() )
			it = pdfData.reserved.upperBound( pdfData.footnotePageIdx );

		if( it != pdfData.reserved.cend() )
			pdfData.footnotePageIdx = it.key();
		else
			pdfData.footnotePageIdx = pdfData.currentPageIdx + 1;

		if( pdfData.footnotePageIdx <= pdfData.currentPageIdx )
		{
			pdfData.currentPainterIdx = pdfData.footnotePageIdx;
			pdfData.coords.x = pdfData.coords.margins.left;
			pdfData.coords.y = pdfData.topFootnoteY( pdfData.footnotePageIdx );
		}
		else
		{
			create( pdfData );

			pdfData.coords.y = pdfData.topFootnoteY( pdfData.footnotePageIdx );
		}

		pdfData.coords.y -= pdfData.extraInFootnote;

		drawHorizontalLine( pdfData, m_opts );

		if( pdfData.continueParagraph )
			pdfData.coords.y -= pdfData.lineHeight;
	}

	if( pdfData.colorsStack.size() > 1 )
		pdfData.repeatColor();
}

void
PdfRenderer::drawHorizontalLine( PdfAuxData & pdfData, const RenderOpts & renderOpts )
{
	pdfData.setColor( renderOpts.m_borderColor );
	pdfData.drawLine( pdfData.coords.margins.left, pdfData.coords.y,
		pdfData.coords.pageWidth - pdfData.coords.margins.right,
		pdfData.coords.y );
	pdfData.restoreColor();
}

Utf8String
PdfRenderer::createUtf8String( const QString & text )
{
	return { text.toUtf8() };
}

QString
PdfRenderer::createQString( const char * str )
{
	return QString::fromUtf8( str, -1 );
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawHeading( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Heading< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	double offset, double nextItemMinHeight, CalcHeightOpt heightCalcOpt, double scale,
	bool withNewLine )
{
	if( item && item->text().get() )
	{
		pdfData.startLine = item->startLine();
		pdfData.startPos = item->startColumn();
		pdfData.endLine = item->endLine();
		pdfData.endPos = item->endColumn();

		const auto where = drawParagraph( pdfData, renderOpts, item->text().get(), doc,
			offset, withNewLine, heightCalcOpt,
			scale * ( 1.0 + ( 7 - item->level() ) * 0.25 ) );

		if( heightCalcOpt == CalcHeightOpt::Unknown && !item->label().isEmpty() &&
				!where.first.isEmpty() )
		{
			m_dests.insert( item->label(),
				std::make_shared< Destination> ( pdfData.doc->GetPages().GetPageAt(
						static_cast< unsigned int >( where.first.front().pageIdx ) ),
					pdfData.coords.margins.left + offset,
					where.first.front().y + where.first.front().height, 0.0 ) );
		}

		return where;
	}
	else
		return {};
}

QVector< QPair< QRectF, unsigned int > >
PdfRenderer::drawText( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Text< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	bool & newLine, Font * footnoteFont, double footnoteFontSize, double footnoteFontScale,
	MD::Item< MD::QStringTrait > * nextItem, int footnoteNum,
	double offset, bool firstInParagraph, CustomWidth * cw, double scale )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	auto * spaceFont = createFont( renderOpts.m_textFont, false, false,
		renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

	auto * font = createFont( renderOpts.m_textFont, item->opts() & MD::TextOption::BoldText,
		item->opts() & MD::TextOption::ItalicText,
		renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

	return drawString( pdfData, renderOpts, item->text(),
		spaceFont, renderOpts.m_textFontSize, scale,
		spaceFont, renderOpts.m_textFontSize, scale,
		font, renderOpts.m_textFontSize, scale,
		pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale ),
		doc, newLine,
		footnoteFont, footnoteFontSize, footnoteFontScale, nextItem, footnoteNum, offset,
		firstInParagraph, cw, QColor(),
		item->opts() & MD::TextOption::StrikethroughText,
		item->startLine(), item->startColumn(), item->endLine(), item->endColumn() );
}

namespace /* anonymous */ {

//! Combine smaller rectangles standing next each other to bigger one.
QVector< QPair< QRectF, unsigned int > >
normalizeRects( const QVector< QPair< QRectF, unsigned int > > & rects )
{
	QVector< QPair< QRectF, unsigned int > > ret;

	if( !rects.isEmpty() )
	{
		QPair< QRectF, int > to( rects.first() );

		auto it = rects.cbegin();
		++it;

		for( auto last = rects.cend(); it != last; ++it )
		{
			if( qAbs( it->first.y() - to.first.y() ) < 0.001 )
				to.first.setWidth( to.first.width() + it->first.width() );
			else
			{
				ret.append( to );

				to = *it;
			}
		}

		ret.append( to );
	}

	return ret;
}

} /* namespace anonymous */

QVector< QPair< QRectF, unsigned int > >
PdfRenderer::drawLink( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Link< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	bool & newLine, Font * footnoteFont, double footnoteFontSize, double footnoteFontScale,
	MD::Item< MD::QStringTrait > * nextItem, int footnoteNum,
	double offset, bool firstInParagraph, CustomWidth * cw, double scale )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	QVector< QPair< QRectF, unsigned int > > rects;

	QString url = item->url();

	const auto lit = doc->labeledLinks().find( url );

	if( lit != doc->labeledLinks().cend() )
		url = lit->second->url();

	bool draw = true;

	if( cw && !cw->isDrawing() )
		draw = false;

	auto * font = createFont( renderOpts.m_textFont, item->opts() & MD::TextOption::BoldText,
		item->opts() & MD::TextOption::ItalicText, renderOpts.m_textFontSize,
		pdfData.doc, scale, pdfData );

	if( !item->p()->isEmpty() )
	{
		pdfData.setColor( renderOpts.m_linkColor );

		for( auto it = item->p()->items().begin(), last = item->p()->items().end();
			it != last; ++it )
		{
			switch( (*it)->type() )
			{
				case MD::ItemType::Text :
				{
					auto * text = std::static_pointer_cast< MD::Text< MD::QStringTrait > >( *it ).get();

					auto * spaceFont = createFont( renderOpts.m_textFont, false, false,
						renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

					auto * font = createFont( renderOpts.m_textFont,
						text->opts() & MD::BoldText || item->opts() & MD::BoldText,
						text->opts() & MD::ItalicText || item->opts() & MD::ItalicText,
						renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

					rects.append( drawString( pdfData, renderOpts, text->text(),
						spaceFont, renderOpts.m_textFontSize, scale,
						spaceFont, renderOpts.m_textFontSize, scale,
						font, renderOpts.m_textFontSize, scale,
						pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale ),
						doc, newLine, footnoteFont, footnoteFontSize, footnoteFontScale,
						( it == std::prev( last ) ? nextItem : nullptr ), footnoteNum, offset,
						( it == item->p()->items().begin() && firstInParagraph ),
						cw, QColor(),
						text->opts() & MD::StrikethroughText ||
							item->opts() & MD::StrikethroughText,
						text->startLine(), text->startColumn(),
						text->endLine(), text->endColumn() ) );
				}
					break;

				case MD::ItemType::Code :
					rects.append( drawInlinedCode( pdfData, renderOpts,
						static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
						doc, newLine, offset,
						( it == item->p()->items().begin() && firstInParagraph ), cw, scale ) );
					break;

				case MD::ItemType::Image :
					rects.append( drawImage( pdfData, renderOpts,
						static_cast< MD::Image< MD::QStringTrait >* > ( it->get() ),
						doc, newLine, offset,
						( it == item->p()->items().begin() && firstInParagraph ), cw, scale ) );

				default :
					break;
			}
		}

		pdfData.restoreColor();
	}
	else if( item->img()->isEmpty() )
	{
		auto * spaceFont = createFont( renderOpts.m_textFont, false, false,
			renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

		rects = drawString( pdfData, renderOpts, url,
			spaceFont, renderOpts.m_textFontSize, scale,
			spaceFont, renderOpts.m_textFontSize, scale,
			font, renderOpts.m_textFontSize, scale,
			pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale ),
			doc, newLine,
			footnoteFont, footnoteFontSize, footnoteFontScale, nextItem, footnoteNum, offset,
			firstInParagraph, cw, QColor(),
			item->opts() & MD::TextOption::StrikethroughText,
			item->startLine(), item->startColumn(), item->endLine(), item->endColumn() );
	}
	// Otherwise image link.
	else
		rects.append( drawImage( pdfData, renderOpts, item->img().get(), doc, newLine, offset,
			firstInParagraph, cw, scale ) );

	rects = normalizeRects( rects );

	if( draw )
	{
		// If Web URL.
		if( !pdfData.anchors.contains( url ) &&
			pdfData.md->labeledHeadings().find( url ) == pdfData.md->labeledHeadings().cend() )
		{
			for( const auto & r : std::as_const( rects ) )
			{
				auto & annot = pdfData.doc->GetPages().GetPageAt(
					static_cast< unsigned int >( r.second ) )
						.GetAnnotations().CreateAnnot< PoDoFo::PdfAnnotationLink >(
					Rect( r.first.x(), r.first.y(), r.first.width(), r.first.height() ) );
				annot.SetBorderStyle( 0.0, 0.0, 0.0 );

				auto action = std::make_shared< PoDoFo::PdfAction >( *pdfData.doc,
					PoDoFo::PdfActionType::URI );
				action->SetURI( url.toLatin1().data() );

				annot.SetAction( action );
			}
		}
		// Otherwise internal link.
		else
			m_unresolvedLinks.insert( url, rects );
	}

	return rects;
}

QVector< QPair< QRectF, unsigned int > >
PdfRenderer::drawString( PdfAuxData & pdfData, const RenderOpts & renderOpts, const QString & str,
	Font * firstSpaceFont, double firstSpaceFontSize, double firstSpaceFontScale,
	Font * spaceFont, double spaceFontSize, double spaceFontScale,
	Font * font, double fontSize, double fontScale,
	double lineHeight,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc, bool & newLine,
	Font * footnoteFont, double footnoteFontSize, double footnoteFontScale,
	MD::Item< MD::QStringTrait > * nextItem,
	int footnoteNum, double offset,
	bool firstInParagraph, CustomWidth * cw, const QColor & background,
	bool strikeout, long long int startLine, long long int startPos,
	long long int endLine, long long int endPos )
{
	Q_UNUSED( doc )
	Q_UNUSED( renderOpts )

	pdfData.startLine = startLine;
	pdfData.startPos = startPos;
	pdfData.endLine = endLine;
	pdfData.endPos = endPos;

	bool draw = true;

	if( cw && !cw->isDrawing() )
		draw = false;

	bool footnoteAtEnd = false;
	double footnoteWidth = 0.0;

	if( nextItem && nextItem->type() == MD::ItemType::FootnoteRef &&
		doc->footnotesMap().find( static_cast< MD::FootnoteRef< MD::QStringTrait >* >(
			nextItem )->id() ) != doc->footnotesMap().cend() )
				footnoteAtEnd = true;

	if( footnoteAtEnd )
		footnoteWidth = pdfData.stringWidth( footnoteFont, footnoteFontSize, footnoteFontScale,
			createUtf8String( QString::number( footnoteNum ) ) );

	double h = lineHeight;
	double descent = 0.0;
	double ascent = pdfData.fontAscent( font, fontSize, fontScale );
	double d = 0.0;

	if( cw && cw->isDrawing() )
	{
		h = cw->height();
		descent = cw->descent();
		d = ( h - ascent ) / 2.0;
	}

	auto newLineFn = [&] ()
	{
		newLine = true;

		if( draw )
		{
			if( cw )
				cw->moveToNextLine();

			moveToNewLine( pdfData, offset, cw->height(), 1.0, cw->height() );

			h = cw->height();
			descent = cw->descent();
			d = ( h - ascent ) / 2.0;
		}
		else if( cw )
		{
			cw->append( { 0.0, lineHeight, 0.0, false, true, true, false, "" } );
			pdfData.coords.x = pdfData.coords.margins.left + offset;
		}
	};

	QVector< QPair< QRectF, unsigned int > > ret;

	{
		QMutexLocker lock( &m_mutex );

		if( m_terminate )
			return ret;
	}

	static const QString charsWithoutSpaceBefore = QLatin1String( ".,;" );

	const auto words = str.split( QRegularExpression( "[ \n]" ), Qt::SkipEmptyParts );

	const auto wv = pdfData.coords.pageWidth - pdfData.coords.margins.right;

	// We need to draw space char if first word is a word.
	if( !firstInParagraph && !newLine && !words.isEmpty() &&
		!charsWithoutSpaceBefore.contains( words.first() ) )
	{
		const auto w = pdfData.stringWidth( firstSpaceFont, firstSpaceFontSize,
			firstSpaceFontScale, " " );

		auto scale = 100.0;

		if( draw && cw )
			scale = cw->scale();

		const auto xv = pdfData.coords.x + w * scale / 100.0 + pdfData.stringWidth( font,
			fontSize, fontScale, createUtf8String( words.first() ) ) +
			( words.size() == 1 && footnoteAtEnd ? footnoteWidth : 0.0 );

		if( xv < wv || qAbs( xv - wv ) < 0.01 )
		{
			newLine = false;

			if( draw )
			{
				pdfData.drawText( pdfData.coords.x, pdfData.coords.y + d, " ",
					firstSpaceFont, firstSpaceFontSize * firstSpaceFontScale,
					scale / 100.0, false );
			}
			else if( cw )
				cw->append( { w, lineHeight, 0.0, true, false, true, false, " " } );

			ret.append( qMakePair( QRectF( pdfData.coords.x,
				pdfData.coords.y + d,
				w * scale / 100.0, lineHeight ), pdfData.currentPageIndex() ) );

			pdfData.coords.x += w * scale / 100.0;
		}
		else
			newLineFn();
	}

	const auto fullWidth = pdfData.coords.pageWidth - pdfData.coords.margins.left -
		pdfData.coords.margins.right;

	const auto spaceWidth = pdfData.stringWidth( font, fontSize, fontScale, " " );

	// Draw words.
	for( auto it = words.begin(), last = words.end(); it != last; ++it )
	{
		auto countCharsForAvailableSpace = []( const QString & s, double availableWidth,
			Font * font, const PdfAuxData & pdfData, double fontSize, double fontScale,
			QString & tmp ) -> qsizetype
		{
			qsizetype i = 0;

			for( ; i < s.length(); ++i )
			{
				tmp.push_back( s[ i ] );
				const auto l = pdfData.stringWidth( font, fontSize, fontScale,
					createUtf8String( tmp ) );

				if( l > availableWidth && !( qAbs( l - availableWidth ) < 0.01 ) )
				{
					tmp.removeLast();

					--i;

					break;
				}
			}

			return ( i < s.length() ? ++i : i );
		}; // countCharsForAvailableSpace

		auto drawSpaceIfNeeded = [&]()
		{
			if( it + 1 != last )
			{
				const auto nextLength = pdfData.stringWidth( font, fontSize, fontScale,
						createUtf8String( *( it + 1 ) ) ) +
					( it + 2 == last && footnoteAtEnd ? footnoteWidth : 0.0 );

				auto scale = 100.0;

				if( draw && cw )
					scale = cw->scale();

				const auto availableWidth = wv - ( pdfData.coords.x > 0.0 ? pdfData.coords.x : 0.0 ) -
					spaceWidth * scale / 100.0;

				const auto xv = pdfData.coords.x + spaceWidth * scale / 100.0 + nextLength;

				QString tmp;

				if( ( xv < wv || qAbs( xv - wv ) < 0.01 ) ||
					( nextLength > fullWidth * 2.0 / 3.0 &&
						countCharsForAvailableSpace( *( it + 1 ), availableWidth,
							font, pdfData, fontSize, fontScale, tmp ) > 4 ) )
				{
					newLine = false;

					if( draw )
					{
						ret.append( qMakePair( QRectF( pdfData.coords.x,
							pdfData.coords.y + d,
							spaceWidth * scale / 100.0, lineHeight ), pdfData.currentPageIndex() ) );

						if( background.isValid() )
						{
							pdfData.setColor( background );
							pdfData.drawRectangle( pdfData.coords.x, pdfData.coords.y +
								pdfData.fontDescent( font, fontSize, fontScale ) + d,
								spaceWidth * scale / 100.0,
								pdfData.lineSpacing( font, fontSize, fontScale ),
								PoDoFo::PdfPathDrawMode::Fill );
							pdfData.restoreColor();
						}

						pdfData.drawText( pdfData.coords.x, pdfData.coords.y + d, " ",
							spaceFont, spaceFontSize * spaceFontScale, scale / 100.0, strikeout );
					}
					else if( cw )
						cw->append( { spaceWidth, lineHeight, 0.0, true, false, true, false, " " } );

					pdfData.coords.x += spaceWidth * scale / 100.0;
				}
				else
					newLineFn();
			}
		}; // drawSpaceIfNeeded

		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				return ret;
		}

		const auto str = createUtf8String( *it );

		const auto length = pdfData.stringWidth( font, fontSize, fontScale, str );

		const auto xv = pdfData.coords.x + length +
			( it + 1 == last && footnoteAtEnd ? footnoteWidth : 0.0 );

		if( xv < wv || qAbs( xv - wv ) < 0.01 )
		{
			newLine = false;

			if( draw )
			{
				if( background.isValid() )
				{
					pdfData.setColor( background );
					pdfData.drawRectangle( pdfData.coords.x, pdfData.coords.y +
						pdfData.fontDescent( font, fontSize, fontScale ) + d,
						length, pdfData.lineSpacing( font, fontSize, fontScale ),
						PoDoFo::PdfPathDrawMode::Fill );
					pdfData.restoreColor();
				}

				pdfData.drawText( pdfData.coords.x, pdfData.coords.y + d, str,
					font, fontSize * fontScale, 1.0, strikeout );

				ret.append( qMakePair( QRectF( pdfData.coords.x,
					pdfData.coords.y + d,
					length, lineHeight ), pdfData.currentPageIndex() ) );
			}
			else if( cw )
				cw->append( { length + ( it + 1 == last && footnoteAtEnd ? footnoteWidth : 0.0 ),
					lineHeight, 0.0, false, false, true, false, *it } );

			pdfData.coords.x += length;

			drawSpaceIfNeeded();
		}
		// Need to move to new line.
		else
		{
			const auto worldLength = length +
				( it + 1 == last && footnoteAtEnd ? footnoteWidth : 0.0 );
			auto availableWidth = ( wv - ( pdfData.coords.x > 0.0 ? pdfData.coords.x : 0.0 ) );

			auto splitAndDraw = [&] ( QString s )
			{
				while( s.length() )
				{
					QString tmp;
					const auto i = countCharsForAvailableSpace( s, availableWidth,
						font, pdfData, fontSize, fontScale, tmp );

					s.remove( 0, i );

					const auto p = createUtf8String( tmp );
					const auto w = pdfData.stringWidth( font, fontSize, fontScale, p );

					if( draw )
					{
						pdfData.drawText( pdfData.coords.x, pdfData.coords.y + d, p,
							font, fontSize * fontScale, 1.0, strikeout );

						ret.append( qMakePair( QRectF( pdfData.coords.x,
								pdfData.coords.y + d, w, lineHeight ),
							pdfData.currentPageIndex() ) );
					}
					else if( cw )
						cw->append( { w, lineHeight, 0.0, false, false, true, false, tmp } );

					newLine = false;

					availableWidth = fullWidth;

					pdfData.coords.x += w;

					if( s.length() )
						newLineFn();
				}

				if( !draw && cw && it + 1 == last && footnoteAtEnd )
					cw->append( { footnoteWidth, lineHeight, 0.0, false, false, true, false,
						QString::number( footnoteNum ) } );

				drawSpaceIfNeeded();
			}; // splitAndDraw

			if( worldLength > fullWidth * 2.0 / 3.0 )
			{
				QString tmp;

				if( countCharsForAvailableSpace( *it, availableWidth, font, pdfData,
					fontSize, fontScale, tmp ) > 4 )
						splitAndDraw( *it );
				else
				{
					if( worldLength < fullWidth || qAbs( worldLength - fullWidth ) < 0.01 )
					{
						newLineFn();

						--it;
					}
					else
					{
						newLineFn();

						splitAndDraw( *it );
					}
				}
			}
			else
			{
				newLineFn();

				--it;
			}
		}
	}

	return ret;
}

QVector< QPair< QRectF, unsigned int > >
PdfRenderer::drawInlinedCode( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Code< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	bool & newLine, double offset,
	bool firstInParagraph, CustomWidth * cw,
	double scale )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	auto * textFont = createFont( renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
		pdfData.doc, scale, pdfData );

	auto * font = createFont( renderOpts.m_codeFont, item->opts() & MD::TextOption::BoldText,
		item->opts() & MD::TextOption::ItalicText, renderOpts.m_codeFontSize,
		pdfData.doc, scale, pdfData );

	return drawString( pdfData, renderOpts, item->text(),
		textFont, renderOpts.m_textFontSize, scale,
		font, renderOpts.m_codeFontSize, scale,
		font, renderOpts.m_codeFontSize, scale,
		pdfData.lineSpacing( textFont, renderOpts.m_textFontSize, scale ),
		doc, newLine,
		nullptr, 0.0, 0.0, nullptr, m_footnoteNum,
		offset, firstInParagraph, cw,
		renderOpts.m_syntax->theme().editorColor( KSyntaxHighlighting::Theme::CodeFolding ),
		item->opts() & MD::TextOption::StrikethroughText,
		item->startLine(), item->startColumn(),
		item->endLine(), item->endColumn() );
}

void
PdfRenderer::moveToNewLine( PdfAuxData & pdfData, double xOffset, double yOffset,
	double yOffsetMultiplier, double yOffsetOnNewPage )
{
	pdfData.coords.x = pdfData.coords.margins.left + xOffset;
	pdfData.coords.y -= yOffset * yOffsetMultiplier;

	if( pdfData.coords.y < pdfData.currentPageAllowedY() &&
		qAbs( pdfData.coords.y - pdfData.currentPageAllowedY() ) > 0.1 )
	{
		createPage( pdfData );

		pdfData.coords.x = pdfData.coords.margins.left + xOffset;
		pdfData.coords.y -= yOffsetOnNewPage;
	}
}

namespace /* anonymous */ {

QVector< WhereDrawn >
toWhereDrawn( const QVector< QPair< QRectF, unsigned int > > & rects, double pageHeight )
{
	struct AuxData{
		double minY = 0.0;
		double maxY = 0.0;
	}; // struct AuxData

	QMap< int, AuxData > map;

	for( const auto & r : rects )
	{
		if( !map.contains( r.second ) )
			map[ r.second ] = { pageHeight, 0.0 };

		if( r.first.y() < map[ r.second ].minY )
			map[ r.second ].minY = r.first.y();

		if( r.first.height() + r.first.y() > map[ r.second ].maxY )
			map[ r.second ].maxY = r.first.height() + r.first.y();
	}

	QVector< WhereDrawn > ret;

	for( auto it = map.cbegin(), last = map.cend(); it != last; ++it )
		ret.append( { it.key(), it.value().minY, it.value().maxY - it.value().minY } );

	return ret;
}

double totalHeight( const QVector< WhereDrawn > & where )
{
	return std::accumulate( where.cbegin(), where.cend(), 0.0,
		[] ( const double & val, const WhereDrawn & cur ) -> double
			{ return ( val + cur.height ); } );
}

} /* namespace anonymous */

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawParagraph( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Paragraph< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	double offset, bool withNewLine,
	CalcHeightOpt heightCalcOpt, double scale )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	QVector< QPair< QRectF, unsigned int > > rects;

	{
		QMutexLocker lock( &m_mutex );

		if( m_terminate )
			return {};
	}

	if( heightCalcOpt == CalcHeightOpt::Unknown )
		emit status( tr( "Drawing paragraph." ) );

	auto * font = createFont( renderOpts.m_textFont, false, false,
		renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

	auto * footnoteFont = font;

	const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale );

	pdfData.lineHeight = lineHeight;

	pdfData.coords.x = pdfData.coords.margins.left + offset;

	bool newLine = false;
	CustomWidth cw;

	bool lineBreak = false;
	bool firstInParagraph = true;

	// Calculate words/lines/spaces widthes.
	for( auto it = item->items().begin(), last = item->items().end(); it != last; ++it )
	{
		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				return {};
		}

		int nextFootnoteNum = m_footnoteNum;

		if( it + 1 != last && ( it + 1 )->get()->type() == MD::ItemType::FootnoteRef )
		{
			auto * ref = static_cast< MD::FootnoteRef< MD::QStringTrait >* > ( ( it + 1 )->get() );

			const auto fit = doc->footnotesMap().find( ref->id() );

			if( fit != doc->footnotesMap().cend() )
			{
				auto anchorIt = pdfData.footnotesAnchorsMap.constFind( fit->second.get() );

				if( anchorIt != pdfData.footnotesAnchorsMap.cend() )
					nextFootnoteNum = anchorIt->second;
			}
		}

		switch( (*it)->type() )
		{
			case MD::ItemType::Text :
				drawText( pdfData, renderOpts,
					static_cast< MD::Text< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, footnoteFont, renderOpts.m_textFontSize * scale, c_footnoteScale,
					( it + 1 != last ? ( it + 1 )->get() : nullptr ),
					nextFootnoteNum, offset, ( firstInParagraph || lineBreak ), &cw, scale );
				lineBreak = false;
				firstInParagraph = false;
				break;

			case MD::ItemType::Code :
				drawInlinedCode( pdfData, renderOpts,
					static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
				doc, newLine, offset, ( firstInParagraph || lineBreak ), &cw, scale );
				lineBreak = false;
				firstInParagraph = false;
				break;

			case MD::ItemType::Link :
				drawLink( pdfData, renderOpts,
					static_cast< MD::Link< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, footnoteFont, renderOpts.m_textFontSize * scale, c_footnoteScale,
					( it + 1 != last ? ( it + 1 )->get() : nullptr ),
					nextFootnoteNum, offset, ( firstInParagraph || lineBreak ), &cw, scale );
				lineBreak = false;
				firstInParagraph = false;
				break;

			case MD::ItemType::Image :
				drawImage( pdfData, renderOpts,
					static_cast< MD::Image< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, offset, ( firstInParagraph || lineBreak ), &cw, scale );
				lineBreak = false;
				firstInParagraph = false;
				break;

			case MD::ItemType::Math :
				drawMathExpr( pdfData, renderOpts,
					static_cast< MD::Math< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, offset, ( std::next( it ) != last),
					( firstInParagraph || lineBreak ),
					&cw, scale );
				lineBreak = false;
				firstInParagraph = false;
				break;

			case MD::ItemType::LineBreak :
			{
				lineBreak = true;
				cw.append( { 0.0, lineHeight, 0.0, false, true, false, false, "" } );
				pdfData.coords.x = pdfData.coords.margins.left + offset;
			}
				break;

			case MD::ItemType::FootnoteRef :
			{
				auto * ref = static_cast< MD::FootnoteRef< MD::QStringTrait >* > ( it->get() );

				const auto fit = doc->footnotesMap().find( ref->id() );

				if( fit != doc->footnotesMap().cend() )
				{
					auto anchorIt = pdfData.footnotesAnchorsMap.constFind( fit->second.get() );

					if( anchorIt == pdfData.footnotesAnchorsMap.cend() )
					{
						pdfData.footnotesAnchorsMap.insert( fit->second.get(),
							{ pdfData.currentFile, m_footnoteNum++ } );
					}
				}
				else
					drawText( pdfData, renderOpts,
						static_cast< MD::Text< MD::QStringTrait >* > ( it->get() ),
						doc, newLine, footnoteFont, renderOpts.m_textFontSize * scale, c_footnoteScale,
						( it + 1 != last ? ( it + 1 )->get() : nullptr ),
						nextFootnoteNum, offset, ( firstInParagraph || lineBreak ), &cw, scale );

				lineBreak = false;
				firstInParagraph = false;
			}
				break;

			default :
				break;
		}
	}

	if( !cw.isNewLineAtEnd() )
		cw.append( { 0.0, lineHeight, 0.0, false, true, false, false, "" } );

	cw.calcScale( pdfData.coords.pageWidth - pdfData.coords.margins.left -
		pdfData.coords.margins.right - offset );

	cw.setDrawing();

	switch( heightCalcOpt )
	{
		case CalcHeightOpt::Minimum :
		{
			QVector< WhereDrawn > r;
			r.append( { -1, 0.0,
				( ( withNewLine && !pdfData.firstOnPage ) ||
					( withNewLine && pdfData.drawFootnotes ) ?
						lineHeight + cw.firstItemHeight() :
						cw.firstItemHeight() ) } );

			return { r, {} };
		}

		case CalcHeightOpt::Full :
		{
			QVector< WhereDrawn > r;

			double h = 0.0;
			double max = 0.0;

			for( auto it = cw.cbegin(), last = cw.cend(); it != last; ++it )
			{
				if( it == cw.cbegin() && ( ( withNewLine && !pdfData.firstOnPage ) ||
					( withNewLine && pdfData.drawFootnotes ) ) )
						h += lineHeight;

				if( h + it->height > max )
					max = h + it->height;

				if( it->isNewLine )
				{
					r.append( { -1, 0.0, max } );
					max = 0.0;
					h = 0.0;
				}
			}

			return { r, {} };
		}

		default :
			break;
	}

	if( ( withNewLine && !pdfData.firstOnPage && heightCalcOpt == CalcHeightOpt::Unknown ) ||
		( withNewLine && pdfData.drawFootnotes && heightCalcOpt == CalcHeightOpt::Unknown ) )
			moveToNewLine( pdfData, offset, lineHeight + cw.height(), 1.0,
				( pdfData.drawFootnotes ? lineHeight + cw.height() : cw.height() ) );
	else
		moveToNewLine( pdfData, offset, cw.height(), 1.0, cw.height() );

	pdfData.coords.x = pdfData.coords.margins.left + offset;

	bool extraOnFirstLine = true;
	newLine = false;

	const auto firstLineY = pdfData.coords.y;
	const auto firstLinePageIdx = pdfData.currentPageIndex();
	const auto firstLineHeight = cw.height();

	pdfData.continueParagraph = true;

	lineBreak = false;
	firstInParagraph = true;

	// Actual drawing.
	for( auto it = item->items().begin(), last = item->items().end(); it != last; ++it )
	{
		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				return {};
		}

		int nextFootnoteNum = m_footnoteNum;

		if( it + 1 != last && ( it + 1 )->get()->type() == MD::ItemType::FootnoteRef )
		{
			auto * ref = static_cast< MD::FootnoteRef< MD::QStringTrait >* > ( ( it + 1 )->get() );

			const auto fit = doc->footnotesMap().find( ref->id() );

			if( fit != doc->footnotesMap().cend() )
			{
				auto anchorIt = pdfData.footnotesAnchorsMap.constFind( fit->second.get() );

				if( anchorIt != pdfData.footnotesAnchorsMap.cend() )
					nextFootnoteNum = anchorIt->second;
			}
		}

		switch( (*it)->type() )
		{
			case MD::ItemType::Text :
			{
				rects.append( drawText( pdfData, renderOpts,
					static_cast< MD::Text< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, nullptr, 0.0, 1.0, nullptr, nextFootnoteNum,
					offset, ( firstInParagraph || lineBreak ), &cw, scale ) );
				lineBreak = false;
				firstInParagraph = false;
			}
				break;

			case MD::ItemType::Code :
			{
				rects.append( drawInlinedCode( pdfData, renderOpts,
					static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, offset, ( firstInParagraph || lineBreak ), &cw, scale ) );
				lineBreak = false;
				firstInParagraph = false;
			}
				break;

			case MD::ItemType::Link :
			{
				auto link = static_cast< MD::Link< MD::QStringTrait >* > ( it->get() );

				if( cw.isImage() && extraOnFirstLine )
					pdfData.coords.y += cw.height();

				rects.append( drawLink( pdfData, renderOpts, link,
					doc, newLine, nullptr, 0.0, 1.0, nullptr, nextFootnoteNum,
					offset, ( firstInParagraph || lineBreak ), &cw, scale ) );
				lineBreak = false;
				firstInParagraph = false;
			}
				break;

			case MD::ItemType::Image :
			{
				if( extraOnFirstLine )
					pdfData.coords.y += cw.height();

				rects.append( drawImage( pdfData, renderOpts,
					static_cast< MD::Image< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, offset, ( firstInParagraph || lineBreak ), &cw, scale ) );
				lineBreak = false;
				firstInParagraph = false;
			}
				break;

			case MD::ItemType::Math :
			{
				rects.append( drawMathExpr( pdfData, renderOpts,
					static_cast< MD::Math< MD::QStringTrait >* > ( it->get() ),
					doc, newLine, offset, ( std::next( it ) != last ),
					( firstInParagraph || lineBreak ),
					&cw, scale ) );
				lineBreak = false;
				firstInParagraph = false;
			}
				break;

			case MD::ItemType::LineBreak :
			{
				lineBreak = true;
				moveToNewLine( pdfData, offset, lineHeight, 1.0, lineHeight );
			}
				break;

			case MD::ItemType::FootnoteRef :
			{
				lineBreak = false;

				auto * ref = static_cast< MD::FootnoteRef< MD::QStringTrait >* > ( it->get() );

				const auto fit = doc->footnotesMap().find( ref->id() );

				if( fit != doc->footnotesMap().cend() )
				{
					auto anchorIt = pdfData.footnotesAnchorsMap.constFind( fit->second.get() );
					int num = m_footnoteNum;

					if( anchorIt != pdfData.footnotesAnchorsMap.cend() )
						num = anchorIt->second;

					const auto str = createUtf8String( QString::number( num ) );

					const auto w = pdfData.stringWidth( footnoteFont,
						renderOpts.m_textFontSize * c_footnoteScale, scale, str );

					rects.append( qMakePair( QRectF( pdfData.coords.x, pdfData.coords.y,
							w, lineHeight ),
						pdfData.currentPageIndex() ) );

					m_unresolvedFootnotesLinks.insert( ref->id(),
						qMakePair( QRectF( pdfData.coords.x, pdfData.coords.y,
								w, lineHeight ),
							pdfData.currentPageIndex() ) );

					pdfData.setColor( renderOpts.m_linkColor );

					pdfData.drawText( pdfData.coords.x, pdfData.coords.y + lineHeight -
							pdfData.lineSpacing( footnoteFont,
								renderOpts.m_textFontSize * c_footnoteScale, scale ), str,
						footnoteFont, renderOpts.m_textFontSize * c_footnoteScale * scale,
						1.0, false );

					pdfData.restoreColor();

					pdfData.coords.x += w;

					addFootnote( ref->id(), fit->second, pdfData, renderOpts, doc );
				}
				else
					rects.append( drawText( pdfData, renderOpts,
						static_cast< MD::Text< MD::QStringTrait >* > ( it->get() ),
						doc, newLine, nullptr, 0.0, 1.0, nullptr, nextFootnoteNum,
						offset, ( firstInParagraph || lineBreak ), &cw, scale ) );

				firstInParagraph = false;
			}
				break;

			default :
				break;
		}

		extraOnFirstLine = firstInParagraph;
	}

	pdfData.continueParagraph = false;

	return { toWhereDrawn( normalizeRects( rects ), pdfData.coords.pageHeight ),
		{ firstLinePageIdx, firstLineY, firstLineHeight } };
}

QPair< QRectF, unsigned int >
PdfRenderer::drawMathExpr( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Math< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	bool & newLine, double offset, bool hasNext,
	bool firstInParagraph, CustomWidth * cw, double scale )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	JKQTMathText mt;
	mt.useAnyUnicode( renderOpts.m_mathFont, renderOpts.m_mathFont );
	mt.setFontPointSize( renderOpts.m_mathFontSize );
	mt.parse( item->expr() );

	QSizeF pxSize = {}, size = {};
	double descent = 0.0;

	{
		PoDoFoPaintDevice pd;
		QPainter p( &pd );
		pxSize = mt.getSize( p );
		size = { pxSize.width() * ( 1.0 / pd.physicalDpiX() * 72.0 ),
			pxSize.height() * ( 1.0 / pd.physicalDpiY() * 72.0 ) };
	}

	auto * font = createFont( renderOpts.m_textFont, false, false,
		renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

	const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale );

	newLine = false;

	bool draw = true;

	if( cw && !cw->isDrawing() )
		draw = false;

	if( cw && cw->isDrawing() && !item->isInline() )
	{
		cw->moveToNextLine();
		moveToNewLine( pdfData, offset, 0.0, 1.0, 0.0 );
	}

	double h = ( cw && cw->isDrawing() ? cw->height() : 0.0 );

	if( draw )
	{
		if( !item->isInline() )
		{
			newLine = true;

			double x = 0.0;
			double imgScale = 1.0;
			const double availableWidth = pdfData.coords.pageWidth - pdfData.coords.margins.left -
				pdfData.coords.margins.right - offset;
			double availableHeight = pdfData.coords.y - pdfData.currentPageAllowedY();

			if( size.width() - availableWidth > 0.01 )
				imgScale = ( availableWidth / size.width() ) * scale;

			const double pageHeight = pdfData.topY( pdfData.currentPageIndex() ) -
				pdfData.coords.margins.bottom;

			if( size.height() * imgScale - pageHeight > 0.01 )
			{
				imgScale = ( pageHeight / ( size.height() * imgScale ) ) * scale;

				if( !pdfData.firstOnPage )
				{
					createPage( pdfData );

					pdfData.coords.x += offset;
				}

				pdfData.freeSpaceOn( pdfData.currentPageIndex() );
			}
			else if( size.height() * imgScale - availableHeight > 0.01 )
			{
				createPage( pdfData );

				pdfData.freeSpaceOn( pdfData.currentPageIndex() );

				pdfData.coords.x += offset;
			}

			if( availableWidth - size.width() * imgScale > 0.01 )
				x = ( availableWidth - size.width() * imgScale ) / 2.0;

			PoDoFoPaintDevice pd;
			pd.setPdfPainter( *(*pdfData.painters)[ pdfData.currentPainterIdx ].get(), *pdfData.doc );
			QPainter p( &pd );

			mt.draw( p, 0, QRectF( QPointF( ( pdfData.coords.x + x ) / 72.0 * pd.physicalDpiX(),
				( pdfData.coords.pageHeight - pdfData.coords.y +
					( h - size.height() * imgScale ) / 2.0 + descent * imgScale ) /
						72.0 * pd.physicalDpiY() ),
				pxSize ) );

			const QRectF r = { pdfData.coords.x + x,
				pdfData.coords.y - ( h - size.height() * imgScale ) / 2.0 - descent * imgScale,
				size.width() * imgScale, size.height() * imgScale };
			const auto idx = pdfData.currentPageIndex();

			pdfData.coords.y -= h;

			if( cw )
				cw->moveToNextLine();

			if( hasNext )
				moveToNewLine( pdfData, offset, lineHeight, 1.0, lineHeight );

			return { r, idx };
		}
		else
		{
			auto sscale = 100.0;

			if( cw )
				sscale = cw->scale();

			const auto spaceWidth = pdfData.stringWidth( font,
				renderOpts.m_textFontSize, scale, " " );

			pdfData.coords.x += spaceWidth * sscale / 100.0;

			const double availableWidth = pdfData.coords.pageWidth - pdfData.coords.x -
				pdfData.coords.margins.right;

			const double availableTotalWidth = pdfData.coords.pageWidth -
				pdfData.coords.margins.left - pdfData.coords.margins.right - offset;

			if( size.width() - availableWidth > 0.01 )
			{
				if( cw )
				{
					cw->moveToNextLine();
					h = cw->height();
				}

				moveToNewLine( pdfData, offset, h, 1.0, h );
			}

			double imgScale = 1.0;

			if( size.width() - availableTotalWidth > 0.01 )
				imgScale = ( availableWidth / size.width() ) * scale;

			double availableHeight = pdfData.coords.y - pdfData.currentPageAllowedY();

			const double pageHeight = pdfData.topY( pdfData.currentPageIndex() ) -
				pdfData.coords.margins.bottom;

			if( size.height() * imgScale - pageHeight > 0.01 )
			{
				imgScale = ( pageHeight / ( size.height() * imgScale ) ) * scale;

				createPage( pdfData );

				pdfData.freeSpaceOn( pdfData.currentPageIndex() );

				pdfData.coords.x += offset;
			}
			else if( size.height() * imgScale - availableHeight > 0.01 )
			{
				createPage( pdfData );

				pdfData.freeSpaceOn( pdfData.currentPageIndex() );

				pdfData.coords.x += offset;
			}

			PoDoFoPaintDevice pd;
			pd.setPdfPainter( *(*pdfData.painters)[ pdfData.currentPainterIdx ].get(), *pdfData.doc );
			QPainter p( &pd );

			mt.draw( p, 0, QRectF( QPointF( ( pdfData.coords.x ) / 72.0 * pd.physicalDpiX(),
				( pdfData.coords.pageHeight - pdfData.coords.y - h +
					( h - size.height() * imgScale ) / 2.0 + descent * imgScale ) /
						72.0 * pd.physicalDpiY() ), pxSize ) );

			pdfData.coords.x += size.width() * imgScale;

			const QRectF r = { pdfData.coords.x,
				pdfData.coords.y + h - ( h - size.height() * imgScale ) / 2.0 - descent * imgScale,
				size.width() * imgScale, size.height() * imgScale };
			const auto idx = pdfData.currentPageIndex();

			return { r, idx };
		}
	}
	else
	{
		if( !item->isInline() )
		{
			double height = 0.0;

			if( !firstInParagraph )
				height += lineHeight;

			newLine = true;

			double imgScale = 1.0;
			const double availableWidth = pdfData.coords.pageWidth - pdfData.coords.margins.left -
				pdfData.coords.margins.right - offset;

			if( size.width() - availableWidth > 0.01 )
				imgScale = ( availableWidth / size.width() ) * scale;

			const double pageHeight = pdfData.topY( pdfData.currentPageIndex() ) -
				pdfData.coords.margins.bottom;

			if( size.height() * imgScale - pageHeight > 0.01 )
				imgScale = ( pageHeight / ( size.height() * imgScale ) ) * scale;

			height += size.height() * imgScale - pdfData.fontDescent( font,
				renderOpts.m_textFontSize, scale );

			pdfData.coords.x = pdfData.coords.margins.left + offset;

			cw->append( { 0.0, 0.0, descent, false, true, false, false, "" } );
			cw->append( { 0.0, height, descent, false, true, false, true, "" } );
		}
		else
		{
			const auto spaceWidth = pdfData.stringWidth( font,
				renderOpts.m_textFontSize, scale, " " );

			const double availableWidth = pdfData.coords.pageWidth - pdfData.coords.x -
				pdfData.coords.margins.right - spaceWidth;

			const double availableTotalWidth = pdfData.coords.pageWidth -
				pdfData.coords.margins.left - pdfData.coords.margins.right - offset;

			pdfData.coords.x += spaceWidth;

			if( size.width() - availableWidth > 0.01 )
			{
				cw->append( { 0.0, lineHeight, descent, false, true, true, false, "" } );
				pdfData.coords.x = pdfData.coords.margins.left + offset;
			}
			else
				cw->append( { spaceWidth, lineHeight, descent, true, false, true, false, " " } );

			double imgScale = 1.0;

			if( size.width() - availableTotalWidth > 0.01 )
				imgScale = ( availableWidth / size.width() ) * scale;

			const double pageHeight = pdfData.topY( pdfData.currentPageIndex() ) -
				pdfData.coords.margins.bottom;

			if( size.height() * imgScale - pageHeight > 0.01 )
				imgScale = ( pageHeight / ( size.height() * imgScale ) ) * scale;

			pdfData.coords.x += size.width() * imgScale;

			cw->append( { size.width() * imgScale,
				size.height() * imgScale - pdfData.fontDescent( font, renderOpts.m_textFontSize, scale ),
				descent, false, false, hasNext, false, "" } );
		}
	}

	return {};
}

void
PdfRenderer::reserveSpaceForFootnote( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	const QVector< WhereDrawn > & h, const double & currentY, int currentPage,
	double lineHeight, bool addExtraLine )
{
	const auto topY = pdfData.topFootnoteY( currentPage );
	const auto available = currentY - topY -
		( qAbs( topY ) < 0.01 ? pdfData.coords.margins.bottom : 0.0 );

	auto height = totalHeight( h ) + ( addExtraLine ? lineHeight : 0.0 );
	auto extra = 0.0;

	if( !pdfData.reserved.contains( currentPage ) )
		extra = pdfData.extraInFootnote;

	auto add = [&pdfData] ( const double & height, int currentPage )
	{
		if( pdfData.reserved.contains( currentPage ) )
			pdfData.reserved[ currentPage ] += height;
		else
			pdfData.reserved.insert( currentPage,
				height + pdfData.coords.margins.bottom );
	};

	if( height + extra < available )
		add( height + extra, currentPage );
	else
	{
		height = extra + ( addExtraLine ? lineHeight : 0.0 );

		for( int i = 0; i < h.size(); ++i )
		{
			const auto tmp = h[ i ].height;

			if( height + tmp < available )
				height += tmp;
			else
			{
				if( qAbs( height - extra ) > 0.01 )
					add( height, currentPage );

				reserveSpaceForFootnote( pdfData, renderOpts, h.mid( i ),
					pdfData.coords.pageHeight - pdfData.coords.margins.top,
					currentPage + 1, lineHeight, true );

				break;
			}
		}
	}
}

QVector< WhereDrawn >
PdfRenderer::drawFootnote( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	const QString & footnoteRefId, MD::Footnote< MD::QStringTrait > * note,
	CalcHeightOpt heightCalcOpt, double * lineHeight )
{
	QVector< WhereDrawn > ret;

	pdfData.startLine = note->startLine();
	pdfData.startPos = note->startColumn();
	pdfData.endLine = note->endLine();
	pdfData.endPos = note->endColumn();

	if( heightCalcOpt == CalcHeightOpt::Unknown && pdfData.footnotesAnchorsMap.contains( note ) )
		pdfData.currentFile = pdfData.footnotesAnchorsMap[ note ].first;

	static const double c_offset = 2.0;

	auto * font = createFont( renderOpts.m_textFont, false, false,
		renderOpts.m_textFontSize, pdfData.doc, c_footnoteScale, pdfData );

	auto footnoteOffset = c_offset * 2.0 / c_mmInPt +
		pdfData.stringWidth( font, renderOpts.m_textFontSize, c_footnoteScale,
			createUtf8String( QString::number( doc->footnotesMap().size() ) ) );

	if( lineHeight )
		*lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, c_footnoteScale );

	for( auto it = note->items().cbegin(), last = note->items().cend(); it != last; ++it )
	{
		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				break;
		}

		switch( (*it)->type() )
		{
			case MD::ItemType::Heading :
				ret.append( drawHeading( pdfData, renderOpts,
					static_cast< MD::Heading< MD::QStringTrait >* > ( it->get() ),
					doc, footnoteOffset,
					// If there is another item after heading we need to know its min
					// height to glue heading with it.
					( it + 1 != last ?
						minNecessaryHeight( pdfData, renderOpts, *( it + 1 ), doc, 0.0,
							c_footnoteScale ) :
						0.0 ), heightCalcOpt, c_footnoteScale ).first );
				pdfData.continueParagraph = true;
				break;

			case MD::ItemType::Paragraph :
				ret.append( drawParagraph( pdfData, renderOpts,
					static_cast< MD::Paragraph< MD::QStringTrait >* > ( it->get() ),
					doc, footnoteOffset,
					true, heightCalcOpt, c_footnoteScale ).first );
				pdfData.continueParagraph = true;
				break;

			case MD::ItemType::Code :
				ret.append( drawCode( pdfData, renderOpts,
					static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
					doc, footnoteOffset, heightCalcOpt, c_footnoteScale ).first );
				pdfData.continueParagraph = true;
				break;

			case MD::ItemType::Blockquote :
				ret.append( drawBlockquote( pdfData, renderOpts,
					static_cast< MD::Blockquote< MD::QStringTrait >* > ( it->get() ),
					doc, footnoteOffset, heightCalcOpt, c_footnoteScale ).first );
				pdfData.continueParagraph = true;
				break;

			case MD::ItemType::List :
			{
				auto * list = static_cast< MD::List< MD::QStringTrait >* > ( it->get() );
				const auto bulletWidth = maxListNumberWidth( list );

				ret.append( drawList( pdfData, renderOpts, list, doc, bulletWidth, footnoteOffset,
					heightCalcOpt, c_footnoteScale ).first );

				pdfData.continueParagraph = true;
			}
				break;

			case MD::ItemType::Table :
				ret.append( drawTable( pdfData, renderOpts,
					static_cast< MD::Table< MD::QStringTrait >* > ( it->get() ),
					doc, footnoteOffset, heightCalcOpt, c_footnoteScale ).first );
				pdfData.continueParagraph = true;
				break;

			default :
				break;
		}

		// Draw footnote number.
		if( it == note->items().cbegin() && heightCalcOpt == CalcHeightOpt::Unknown )
		{
			const auto str = createUtf8String( QString::number( pdfData.currentFootnote ) );
			const auto w = pdfData.stringWidth( font, renderOpts.m_textFontSize,
				c_footnoteScale, str );
			const auto y = ret.constFirst().y + ret.constFirst().height -
				pdfData.lineSpacing( font, renderOpts.m_textFontSize, c_footnoteScale );
			const auto x = pdfData.coords.margins.left + footnoteOffset -
				c_offset - w;
			const auto p = ret.constFirst().pageIdx;

			m_dests.insert( footnoteRefId,
				std::make_shared< Destination > ( pdfData.doc->GetPages().GetPageAt( p ),
					x, y + pdfData.lineSpacing( font, renderOpts.m_textFontSize, c_footnoteScale ),
					0.0 ) );

			pdfData.currentPainterIdx = p;

			pdfData.drawText( x, y, str, font, renderOpts.m_textFontSize * c_footnoteScale,
				1.0, false );

			pdfData.currentPainterIdx = pdfData.footnotePageIdx;

			++pdfData.currentFootnote;
		}
	}

	pdfData.continueParagraph = false;

	return ret;
}

QVector< WhereDrawn >
PdfRenderer::footnoteHeight( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc, MD::Footnote< MD::QStringTrait > * note,
	double * lineHeight )
{
	return drawFootnote( pdfData, renderOpts, doc, "", note, CalcHeightOpt::Full, lineHeight );
}

QPair< QRectF, unsigned int >
PdfRenderer::drawImage( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Image< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	bool & newLine, double offset, bool firstInParagraph, CustomWidth * cw, double scale )
{
	Q_UNUSED( doc )

	bool draw = true;
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	if( cw && !cw->isDrawing() )
		draw = false;

	if( draw )
	{
		emit status( tr( "Loading image." ) );

		const auto img = loadImage( item );

		if( !img.isNull() )
		{
			auto pdfImg = pdfData.doc->CreateImage();
			pdfImg->LoadFromBuffer( { img.data() , static_cast< size_t > ( img.size() ) } );

			const double iWidth = std::round( (double) pdfImg->GetWidth() /
				(double) pdfData.dpi * 72.0 );
			const double iHeight = std::round( (double) pdfImg->GetHeight() /
				(double) pdfData.dpi * 72.0 );

			newLine = true;

			auto * font = createFont( renderOpts.m_textFont, false, false,
				renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

			const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale );

			if( !firstInParagraph )
			{
				moveToNewLine( pdfData, offset, lineHeight, 1.0, lineHeight );

				if( cw && !cw->isImage() )
					cw->moveToNextLine();
			}
			else
				pdfData.coords.x += offset;

			double x = 0.0;
			double imgScale = 1.0;
			const double availableWidth = pdfData.coords.pageWidth - pdfData.coords.margins.left -
				pdfData.coords.margins.right - offset;
			double availableHeight = pdfData.coords.y - pdfData.currentPageAllowedY();

			if( iWidth > availableWidth )
				imgScale = ( availableWidth / iWidth ) * scale;

			const double pageHeight = pdfData.topY( pdfData.currentPageIndex() ) -
				pdfData.coords.margins.bottom;

			if( iHeight * imgScale > pageHeight )
			{
				imgScale = ( pageHeight / ( iHeight * imgScale ) ) * scale;

				if( !pdfData.firstOnPage )
				{
					createPage( pdfData );

					pdfData.coords.x += offset;
				}

				pdfData.freeSpaceOn( pdfData.currentPageIndex() );
			}
			else if( iHeight * imgScale > availableHeight )
			{
				createPage( pdfData );

				pdfData.freeSpaceOn( pdfData.currentPageIndex() );

				pdfData.coords.x += offset;
			}

			if( iWidth * imgScale < availableWidth )
				x = ( availableWidth - iWidth * imgScale ) / 2.0;

			const double dpiScale = (double) pdfImg->GetWidth() / iWidth;

			pdfData.drawImage( pdfData.coords.x + x,
				pdfData.coords.y - iHeight * imgScale,
				pdfImg.get(), imgScale / dpiScale, imgScale / dpiScale );

			pdfData.coords.y -= iHeight * imgScale;

			QRectF r( pdfData.coords.x + x, pdfData.coords.y,
				iWidth * imgScale, iHeight * imgScale );

			moveToNewLine( pdfData, offset, lineHeight, 1.0, lineHeight );

			if( cw )
				cw->moveToNextLine();

			return qMakePair( r, pdfData.currentPageIndex() );
		}
		else
			throw PdfRendererError( tr( "Unable to load image: %1.\n\n"
				"If this image is in Web, please be sure you are connected to the Internet. I'm "
				"sorry for the inconvenience." )
					.arg( item->url() ) );
	}
	else
	{
		emit status( tr( "Loading image." ) );

		const auto img = loadImage( item );

		auto height = 0.0;

		if( !img.isNull() )
		{
			auto pdfImg = pdfData.doc->CreateImage();
			pdfImg->LoadFromBuffer( { img.data(), static_cast< size_t > ( img.size() ) } );

			const double iWidth = std::round( (double) pdfImg->GetWidth() /
				(double) pdfData.dpi * 72.0 );
			const double iHeight = std::round( (double) pdfImg->GetHeight() /
				(double) pdfData.dpi * 72.0 );

			newLine = true;

			auto * font = createFont( renderOpts.m_textFont, false, false,
				renderOpts.m_textFontSize, pdfData.doc, scale, pdfData );

			const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale );

			if( !firstInParagraph )
				height += lineHeight;

			double imgScale = 1.0;
			const double availableWidth = pdfData.coords.pageWidth - pdfData.coords.margins.left -
				pdfData.coords.margins.right - offset;

			if( iWidth > availableWidth )
				imgScale = ( availableWidth / iWidth ) * scale;

			const double pageHeight = pdfData.topY( pdfData.currentPageIndex() ) -
				pdfData.coords.margins.bottom;

			if( iHeight * imgScale > pageHeight )
				imgScale = ( pageHeight / ( iHeight * imgScale ) ) * scale;

			height += iHeight * imgScale;
		}

		pdfData.coords.x = pdfData.coords.margins.left + offset;

		if( !cw->isNewLineAtEnd() && !firstInParagraph )
			cw->append( { 0.0, 0.0, 0.0, false, true, false, false, "" } );

		cw->append( { 0.0, height, 0.0, false, true, false, true, "" } );

		return qMakePair( QRectF(), pdfData.currentPageIndex() );
	}
}

//
// convert
//

QImage
convert( const Magick::Image & img )
{
	QImage qimg( static_cast< int > ( img.columns() ),
		static_cast< int > ( img.rows() ), QImage::Format_RGB888 );
	const Magick::PixelPacket * pixels;
	Magick::ColorRGB rgb;

	for( int y = 0; y < qimg.height(); ++y)
	{
		pixels = img.getConstPixels( 0, y, static_cast< std::size_t > ( qimg.width() ), 1 );

		for( int x = 0; x < qimg.width(); ++x )
		{
			rgb = ( *( pixels + x ) );

			qimg.setPixel( x, y, QColor( static_cast< int> ( 255 * rgb.red() ),
				static_cast< int > ( 255 * rgb.green() ),
				static_cast< int > ( 255 * rgb.blue() ) ).rgb() );
		}
	}

	return qimg;
}


//
// LoadImageFromNetwork
//

LoadImageFromNetwork::LoadImageFromNetwork( const QUrl & url, QThread * thread )
	:	m_thread( thread )
	,	m_reply( nullptr )
	,	m_url( url )
{
	connect( this, &LoadImageFromNetwork::start, this, &LoadImageFromNetwork::loadImpl,
		Qt::QueuedConnection );
}

const QImage &
LoadImageFromNetwork::image() const
{
	return m_img;
}

void
LoadImageFromNetwork::load()
{
	emit start();
}

void
LoadImageFromNetwork::loadImpl()
{
	QNetworkAccessManager * m = new QNetworkAccessManager( this );
	QNetworkRequest r( m_url );
	r.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );
	m_reply = m->get( r );

	connect( m_reply, &QNetworkReply::finished, this, &LoadImageFromNetwork::loadFinished );
	connect( m_reply,
		static_cast< void(QNetworkReply::*)(QNetworkReply::NetworkError) >(
			&QNetworkReply::errorOccurred ),
		this, &LoadImageFromNetwork::loadError );
}

void
LoadImageFromNetwork::loadFinished()
{
	const auto data = m_reply->readAll();
	const auto svg = QString( data.mid( 0, 4 ).toLower() );

	if( svg == QStringLiteral( "<svg" ) )
	{
		try {
			Magick::Image img;
			QTemporaryFile file( QStringLiteral( "XXXXXX.svg" ) );
			if( file.open() )
			{
				file.write( data );
				file.close();

				img.read( file.fileName().toStdString() );

				img.magick( "png" );

				m_img = convert( img );
			}
		}
		catch( const Magick::Exception & )
		{
		}
	}
	else
		m_img.loadFromData( data );

	m_reply->deleteLater();

	m_thread->quit();
}

void
LoadImageFromNetwork::loadError( QNetworkReply::NetworkError )
{
	m_reply->deleteLater();
	m_thread->quit();
}

QByteArray
PdfRenderer::loadImage( MD::Image< MD::QStringTrait > * item )
{
	if( m_imageCache.contains( item->url() ) )
		return m_imageCache[ item->url() ];

	QImage img;

	if( QFileInfo::exists( item->url() ) )
	{
		if( item->url().toLower().endsWith( QStringLiteral( "svg" ) ) )
		{
			try {
				Magick::Image mimg;
				mimg.read( item->url().toStdString() );
				mimg.magick( "png" );
				img = convert( mimg );
			}
			catch( const Magick::Exception & )
			{
			}
		}
		else
			img = QImage( item->url() );
	}
	else if( !QUrl( item->url() ).isRelative() )
	{
		QThread thread;

		LoadImageFromNetwork load( QUrl( item->url() ), &thread );

		load.moveToThread( &thread );
		thread.start();
		load.load();
		thread.wait();

		img = load.image();

#ifdef MD_PDF_TESTING
	if( img.isNull() )
	{
		terminate();

		QWARN( "Got empty image from network." );
	}
#endif
	}
	else
		throw PdfRendererError(
			tr( "Hmm, I don't know how to load this image: %1.\n\n"
				"This image is not a local existing file, and not in the Web. Check your Markdown." )
					.arg( item->url() ) );

	if( img.isNull() )
		throw PdfRendererError( tr( "Unable to load image: %1.\n\n"
			"If this image is in Web, please be sure you are connected to the Internet. I'm "
			"sorry for the inconvenience." ).arg( item->url() ) );

	QByteArray data;
	QBuffer buf( &data );

	QString fmt = QStringLiteral( "png" );

	if( item->url().endsWith( QStringLiteral( "jpg" ) ) ||
		item->url().endsWith( QStringLiteral( "jpeg" ) ))
			fmt = QStringLiteral( "jpg" );

	img.save( &buf, fmt.toLatin1().constData() );

	m_imageCache.insert( item->url(), data );

	return data;
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawCode( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Code< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	double offset, CalcHeightOpt heightCalcOpt,
	double scale )
{
	Q_UNUSED( doc )

	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	if( item->text().isEmpty() )
		return {};

	if( heightCalcOpt == CalcHeightOpt::Unknown )
		emit status( tr( "Drawing code." ) );

	auto * textFont = createFont( renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
		pdfData.doc, scale, pdfData );

	const auto textLHeight = pdfData.lineSpacing( textFont, renderOpts.m_textFontSize, scale );

	QStringList lines;

	if( heightCalcOpt == CalcHeightOpt::Unknown )
	{
		if( pdfData.coords.y - ( textLHeight * 2.0 ) < pdfData.currentPageAllowedY() &&
			qAbs( pdfData.coords.y - ( textLHeight * 2.0 ) - pdfData.currentPageAllowedY() ) > 0.1 )
				createPage( pdfData );
		else
			pdfData.coords.y -= textLHeight * 2.0;

		pdfData.coords.x = pdfData.coords.margins.left + offset;
	}

	lines = item->text().split( QLatin1Char( '\n' ), Qt::KeepEmptyParts );

	for( auto it = lines.begin(), last = lines.end(); it != last; ++it )
		it->replace( QStringLiteral( "\t" ), QStringLiteral( "    " ) );

	auto * font = createFont( renderOpts.m_codeFont, false, false, renderOpts.m_codeFontSize,
		pdfData.doc, scale, pdfData );

	const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_codeFontSize, scale );

	switch( heightCalcOpt )
	{
		case CalcHeightOpt::Minimum :
		{
			QVector< WhereDrawn > r;
			r.append( { -1, 0.0, textLHeight * 2.0 + lineHeight } );

			return { r, {} };
		}

		case CalcHeightOpt::Full :
		{
			QVector< WhereDrawn > r;
			r.append( { -1, 0.0, textLHeight * 2.0 + lineHeight } );

			auto i = 1;

			while( i < lines.size() )
			{
				r.append( { -1, 0.0, lineHeight } );
				++i;
			}

			return { r, {} };
		}

		default :
			break;
	}

	int i = 0;

	QVector< WhereDrawn > ret;

	{
		QMutexLocker lock( &m_mutex );

		if( m_terminate )
			return {};
	}

	pdfData.syntax->setDefinition( pdfData.syntax->definitionForName( item->syntax().toLower() ) );
	const auto colored = pdfData.syntax->prepare( lines );
	int currentWord = 0;
	const auto spaceWidth = pdfData.stringWidth( font, renderOpts.m_codeFontSize, scale, " " );

	const auto firstLinePageIdx = pdfData.currentPageIndex();
	const auto firstLineY = pdfData.coords.y;
	const auto firstLineHeight = lineHeight;

	while( i < lines.size() )
	{
		auto y = pdfData.coords.y;
		int j = i + 1;
		double h = 0.0;

		while( ( y - lineHeight > pdfData.currentPageAllowedY() ||
			qAbs( y - lineHeight - pdfData.currentPageAllowedY() ) < 0.1 ) && j < lines.size() )
		{
			h += lineHeight;
			y -= lineHeight;
			++j;
		}

		if( i < j )
		{
			pdfData.setColor( renderOpts.m_syntax->theme().editorColor(
				KSyntaxHighlighting::Theme::CodeFolding ) );
			pdfData.drawRectangle( pdfData.coords.x, y +
					pdfData.fontDescent( font, renderOpts.m_codeFontSize, scale ),
				pdfData.coords.pageWidth - pdfData.coords.x - pdfData.coords.margins.right,
				 h + lineHeight, PoDoFo::PdfPathDrawMode::Fill );
			pdfData.restoreColor();

			ret.append( { pdfData.currentPageIndex(), y, h + lineHeight } );
		}

		for( ; i < j; ++i )
		{
			pdfData.coords.x = pdfData.coords.margins.left + offset;

			while( true )
			{
				if( currentWord == colored.size() || colored[ currentWord ].line != i )
					break;

				pdfData.setColor( colored[ currentWord ].format.textColor( pdfData.syntax->theme() ) );

				const auto length = colored[ currentWord ].endPos -
					colored[ currentWord ].startPos + 1;

				Font * f = font;

				const auto italic = colored[ currentWord ].format.isItalic( pdfData.syntax->theme() );
				const auto bold = colored[ currentWord ].format.isBold( pdfData.syntax->theme() );

				if( italic || bold )
				{
					f = createFont( renderOpts.m_codeFont, bold, italic, renderOpts.m_codeFontSize,
							pdfData.doc, scale, pdfData );
				}

				pdfData.drawText( pdfData.coords.x, pdfData.coords.y,
					createUtf8String( lines.at( i ).mid( colored[ currentWord ].startPos, length ) ),
					f, renderOpts.m_codeFontSize * scale, 1.0, false );

				pdfData.coords.x += spaceWidth * length;

				pdfData.restoreColor();

				++currentWord;
			}

			pdfData.coords.y -= lineHeight;
		}

		if( i < lines.size() )
		{
			createPage( pdfData );
			pdfData.coords.x = pdfData.coords.margins.left + offset;
			pdfData.coords.y -= lineHeight;
		}
	}

	return { ret, { firstLinePageIdx, firstLineY, firstLineHeight } };
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawBlockquote( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Blockquote< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc, double offset,
	CalcHeightOpt heightCalcOpt, double scale )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	QVector< WhereDrawn > ret;

	if( heightCalcOpt == CalcHeightOpt::Unknown )
		emit status( tr( "Drawing blockquote." ) );

	bool first  = true;
	WhereDrawn firstLine = {};

	// Draw items.
	for( auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it )
	{
		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				return {};
		}

		switch( (*it)->type() )
		{
			case MD::ItemType::Heading :
			{
				if( heightCalcOpt != CalcHeightOpt::Minimum )
				{
					const auto where = drawHeading( pdfData, renderOpts,
						static_cast< MD::Heading< MD::QStringTrait >* > ( it->get() ),
						doc, offset + c_blockquoteBaseOffset,
						( it + 1 != last ?
							minNecessaryHeight( pdfData, renderOpts, *( it + 1 ), doc,
								offset + c_blockquoteBaseOffset, scale  ) : 0.0 ),
						heightCalcOpt, scale );

					ret.append( where.first );

					if( first )
					{
						firstLine = where.second;
						first = false;
					}
				}
				else
				{
					ret.append( { -1, 0.0, 0.0 } );

					return { ret, {} };
				}
			}
				break;

			case MD::ItemType::Paragraph :
			{
				const auto where = drawParagraph( pdfData, renderOpts,
					static_cast< MD::Paragraph< MD::QStringTrait >* > ( it->get() ),
					doc, offset + c_blockquoteBaseOffset, true, heightCalcOpt,
					scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			case MD::ItemType::Code :
			{
				const auto where = drawCode( pdfData, renderOpts,
					static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
					doc, offset + c_blockquoteBaseOffset, heightCalcOpt,
					scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			case MD::ItemType::Blockquote :
			{
				const auto where = drawBlockquote( pdfData, renderOpts,
					static_cast< MD::Blockquote< MD::QStringTrait >* > ( it->get() ),
					doc, offset + c_blockquoteBaseOffset, heightCalcOpt,
					scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			case MD::ItemType::List :
			{
				auto * list = static_cast< MD::List< MD::QStringTrait >* > ( it->get() );
				const auto bulletWidth = maxListNumberWidth( list );

				const auto where = drawList( pdfData, renderOpts,
					list,
					doc, bulletWidth, offset + c_blockquoteBaseOffset, heightCalcOpt,
					scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			case MD::ItemType::Table :
			{
				const auto where = drawTable( pdfData, renderOpts,
					static_cast< MD::Table< MD::QStringTrait >* > ( it->get() ),
					doc, offset + c_blockquoteBaseOffset, heightCalcOpt,
					scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			default :
				break;
		}

		if( heightCalcOpt == CalcHeightOpt::Minimum )
			return { ret, {} };
	}

	if( heightCalcOpt == CalcHeightOpt::Full )
		return { ret, {} };

	struct AuxData {
		double y = 0.0;
		double height = 0.0;
	}; // struct AuxData

	QMap< int, AuxData > map;

	for( const auto & where : std::as_const( ret ) )
	{
		if( !map.contains( where.pageIdx ) )
			map.insert( where.pageIdx, { where.y, where.height } );

		if( map[ where.pageIdx ].y > where.y )
		{
			map[ where.pageIdx ].height = map[ where.pageIdx ].y +
				map[ where.pageIdx ].height - where.y;
			map[ where.pageIdx ].y = where.y;
		}
	}

	// Draw blockquote left vertival bar.
	for( auto it = map.cbegin(), last = map.cend(); it != last; ++it )
	{
		pdfData.currentPainterIdx = it.key();
		pdfData.setColor( renderOpts.m_borderColor );
		pdfData.drawRectangle( pdfData.coords.margins.left + offset, it.value().y,
			c_blockquoteMarkWidth, it.value().height, PoDoFo::PdfPathDrawMode::Fill );
		pdfData.restoreColor();
	}

	pdfData.currentPainterIdx = pdfData.currentPageIndex();

	return { ret, firstLine };
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawList( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::List< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc, int bulletWidth, double offset,
	CalcHeightOpt heightCalcOpt, double scale, bool nested )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	QVector< WhereDrawn > ret;

	{
		QMutexLocker lock( &m_mutex );

		if( m_terminate )
			return {};
	}

	if( heightCalcOpt == CalcHeightOpt::Unknown )
		emit status( tr( "Drawing list." ) );

	int idx = 1;
	ListItemType prevListItemType = ListItemType::Unknown;
	bool first = true;
	WhereDrawn firstLine;

	for( auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it )
	{
		if( (*it)->type() == MD::ItemType::ListItem )
		{
			const auto where = drawListItem( pdfData, renderOpts,
				static_cast< MD::ListItem< MD::QStringTrait >* > ( it->get() ), doc, idx,
				prevListItemType, bulletWidth, offset, heightCalcOpt,
				scale, first && !nested );

			ret.append( where.first );

			if( first )
			{
				firstLine = where.second;
				first = false;
			}
		}

		if( heightCalcOpt == CalcHeightOpt::Minimum )
			break;
	}

	if( !ret.isEmpty() )
		ret.front().height += pdfData.lineSpacing( createFont( m_opts.m_textFont, false, false,
				m_opts.m_textFontSize, pdfData.doc, scale, pdfData ),
			renderOpts.m_textFontSize, scale );

	return { ret, firstLine };
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawListItem( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::ListItem< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc, int & idx,
	ListItemType & prevListItemType, int bulletWidth, double offset, CalcHeightOpt heightCalcOpt,
	double scale, bool firstInList )
{
	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	auto * font = createFont( renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
		pdfData.doc, scale, pdfData );

	const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale );
	const auto orderedListNumberWidth =
		pdfData.stringWidth( font, renderOpts.m_textFontSize, scale, "9" ) * bulletWidth +
		pdfData.stringWidth( font, renderOpts.m_textFontSize, scale, "." );
	const auto spaceWidth = pdfData.stringWidth( font, renderOpts.m_textFontSize, scale, " " );
	const auto unorderedMarkWidth = spaceWidth * 0.75;

	if( heightCalcOpt == CalcHeightOpt::Unknown )
		offset += orderedListNumberWidth + spaceWidth;

	QVector< WhereDrawn > ret;

	bool addExtraSpace = false;
	bool first = true;
	WhereDrawn firstLine;

	for( auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it )
	{
		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				return {};
		}

		switch( (*it)->type() )
		{
			case MD::ItemType::Heading :
			{
				if( heightCalcOpt != CalcHeightOpt::Minimum )
				{
					const auto where = drawHeading( pdfData, renderOpts,
						static_cast< MD::Heading< MD::QStringTrait >* > ( it->get() ),
						doc, offset,
						( it + 1 != last ?
							minNecessaryHeight( pdfData, renderOpts, *( it + 1 ),  doc, offset,
								scale ) :
							0.0 ),
						heightCalcOpt, scale, ( it == item->items().cbegin() && firstInList ) );

					ret.append( where.first );

					if( first )
					{
						firstLine = where.second;
						first = false;
					}
				}
				else
					ret.append( { -1, 0.0, 0.0 } );
			}
				break;

			case MD::ItemType::Paragraph :
			{
				const auto where = drawParagraph( pdfData, renderOpts,
					static_cast< MD::Paragraph< MD::QStringTrait >* > ( it->get() ),
					doc, offset,
					( it == item->items().cbegin() && firstInList ) || it != item->items().cbegin(),
					heightCalcOpt,
					scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}

				addExtraSpace = ( it != item->items().cbegin() );
			}
				break;

			case MD::ItemType::Code :
			{
				const auto where = drawCode( pdfData, renderOpts,
					static_cast< MD::Code< MD::QStringTrait >* > ( it->get() ),
					doc, offset, heightCalcOpt, scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}

				addExtraSpace = false;
			}
				break;

			case MD::ItemType::Blockquote :
			{
				const auto where = drawBlockquote( pdfData, renderOpts,
					static_cast< MD::Blockquote< MD::QStringTrait >* > ( it->get() ),
					doc, offset, heightCalcOpt, scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}

				addExtraSpace = ( it != item->items().cbegin() );
			}
				break;

			case MD::ItemType::List :
			{
				const auto where = drawList( pdfData, renderOpts,
					static_cast< MD::List< MD::QStringTrait >* > ( it->get() ),
					doc, bulletWidth, offset, heightCalcOpt,
					scale, true );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			case MD::ItemType::Table :
			{
				const auto where = drawTable( pdfData, renderOpts,
					static_cast< MD::Table< MD::QStringTrait >* > ( it->get() ),
					doc, offset, heightCalcOpt, scale );

				ret.append( where.first );

				if( first )
				{
					firstLine = where.second;
					first = false;
				}
			}
				break;

			default :
				break;
		}

		if( heightCalcOpt == CalcHeightOpt::Minimum )
			break;
	}

	if( addExtraSpace )
	{
		ret.append( { pdfData.currentPageIndex(), pdfData.coords.y, lineHeight } );

		if( heightCalcOpt != CalcHeightOpt::Full )
			moveToNewLine( pdfData, offset, lineHeight, 1.0, lineHeight );
	}

	if( heightCalcOpt == CalcHeightOpt::Unknown )
	{
		if( firstLine.pageIdx >= 0 )
		{
			pdfData.currentPainterIdx = firstLine.pageIdx;

			if( item->isTaskList() )
			{
				pdfData.setColor( Qt::black );
				pdfData.drawRectangle(
					pdfData.coords.margins.left + offset - ( orderedListNumberWidth + spaceWidth ),
					firstLine.y + qAbs( firstLine.height - orderedListNumberWidth ) / 2.0,
					orderedListNumberWidth, orderedListNumberWidth, PoDoFo::PdfPathDrawMode::Stroke );

				if( item->isChecked() )
				{
					const auto d = orderedListNumberWidth * 0.2;

					pdfData.drawRectangle(
						pdfData.coords.margins.left + offset + d - ( orderedListNumberWidth + spaceWidth ),
						firstLine.y + qAbs( firstLine.height - orderedListNumberWidth ) / 2.0 + d,
						orderedListNumberWidth - 2.0 * d, orderedListNumberWidth - 2.0 * d,
						PoDoFo::PdfPathDrawMode::Fill );
				}

				pdfData.restoreColor();
			}
			else if( item->listType() == MD::ListItem< MD::QStringTrait >::Ordered )
			{
				if( prevListItemType == ListItemType::Unordered )
					idx = 1;
				else if( prevListItemType == ListItemType::Ordered )
					++idx;

				prevListItemType = ListItemType::Ordered;

				const QString idxText = QString::number( idx ) + QLatin1Char( '.' );

				pdfData.drawText(
					pdfData.coords.margins.left + offset - ( orderedListNumberWidth + spaceWidth ),
					firstLine.y + qAbs( firstLine.height -
						pdfData.fontAscent( font, renderOpts.m_textFontSize, scale ) ) / 2.0,
					createUtf8String( idxText ),
					font, renderOpts.m_textFontSize * scale, 1.0, false );
			}
			else
			{
				prevListItemType = ListItemType::Unordered;

				pdfData.setColor( Qt::black );
				const auto r = unorderedMarkWidth / 2.0;
				(*pdfData.painters)[ pdfData.currentPainterIdx ]->DrawCircle(
					pdfData.coords.margins.left + offset + r - ( orderedListNumberWidth + spaceWidth ),
					firstLine.y + qAbs( firstLine.height - unorderedMarkWidth ) / 2.0, r,
					PoDoFo::PdfPathDrawMode::Fill );
				pdfData.restoreColor();
			}

			pdfData.currentPainterIdx = pdfData.currentPageIndex();
		}
	}

	return { ret, firstLine };
}

int
PdfRenderer::maxListNumberWidth( MD::List< MD::QStringTrait > * list ) const
{
	int counter = 0;

	for( auto it = list->items().cbegin(), last = list->items().cend(); it != last; ++it )
	{
		if( (*it)->type() == MD::ItemType::ListItem )
		{
			auto * item = static_cast< MD::ListItem< MD::QStringTrait >* > ( it->get() );

			if( item->listType() == MD::ListItem< MD::QStringTrait >::Ordered )
				++counter;
		}
	}

	for( auto it = list->items().cbegin(), last = list->items().cend(); it != last; ++it )
	{
		if( (*it)->type() == MD::ItemType::ListItem )
		{
			auto * item = static_cast< MD::ListItem< MD::QStringTrait >* > ( it->get() );

			for( auto lit = item->items().cbegin(), llast = item->items().cend(); lit != llast; ++lit )
			{
				if( (*lit)->type() == MD::ItemType::List )
				{
					auto i = maxListNumberWidth( static_cast< MD::List< MD::QStringTrait >* > ( lit->get() ) );

					if( i > counter )
						counter = i;
				}
			}
		}
	}

	return ( counter / 10 + 1 );
}

bool
operator != ( const PdfRenderer::FontAttribs & f1, const PdfRenderer::FontAttribs & f2 )
{
	return ( f1.family != f2.family || f1.bold != f2.bold ||
		f1.italic != f2.italic || f1.strikethrough != f2.strikethrough ||
		f1.size != f2.size );
}

bool
operator == ( const PdfRenderer::FontAttribs & f1, const PdfRenderer::FontAttribs & f2 )
{
	return ( f1.family == f2.family && f1.bold == f2.bold &&
		f1.italic == f2.italic && f1.strikethrough == f2.strikethrough &&
		f1.size == f2.size );
}

void
PdfRenderer::createAuxCell( const RenderOpts & renderOpts,
	PdfAuxData & pdfData,
	CellData & data,
	MD::Item< MD::QStringTrait > * item,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	const QString & url,
	const QColor & color )
{
	auto handleText = [&]()
	{
		auto * t = static_cast< MD::Text< MD::QStringTrait >* > ( item );

		const auto words = t->text().split( QRegularExpression( "[ \n]" ),
			Qt::SkipEmptyParts );

		for( const auto & w : words )
		{
			CellItem item;
			item.word = w;
			item.font = { renderOpts.m_textFont,
				(bool) ( t->opts() & MD::TextOption::BoldText ),
				(bool) ( t->opts() & MD::TextOption::ItalicText ),
				(bool) ( t->opts() & MD::TextOption::StrikethroughText ),
				renderOpts.m_textFontSize };

			if( !url.isEmpty() )
				item.url = url;

			if( color.isValid() )
				item.color = color;

			data.items.append( item );
		}
	};

	switch( item->type() )
	{
		case MD::ItemType::Text :
			handleText();
			break;

		case MD::ItemType::Code :
		{
			auto * c = static_cast< MD::Code< MD::QStringTrait >* > ( item );

			const auto words = c->text().split( QLatin1Char( ' ' ),
				Qt::SkipEmptyParts );

			for( const auto & w : words )
			{
				CellItem item;
				item.word = w;
				item.font = { renderOpts.m_codeFont, false, false, false,
					renderOpts.m_codeFontSize };
				item.background = renderOpts.m_syntax->theme().editorColor(
					KSyntaxHighlighting::Theme::CodeFolding );

				if( !url.isEmpty() )
					item.url = url;

				if( color.isValid() )
					item.color = color;

				data.items.append( item );
			}
		}
			break;

		case MD::ItemType::Link :
		{
			auto * l = static_cast< MD::Link< MD::QStringTrait >* > ( item );

			QString url = l->url();

			const auto lit = doc->labeledLinks().find( url );

			if( lit != doc->labeledLinks().cend() )
				url = lit->second->url();

			if( !l->p()->isEmpty() )
			{
				for( auto pit = l->p()->items().cbegin(), plast = l->p()->items().cend();
					pit != plast; ++pit )
				{
					createAuxCell( renderOpts, pdfData, data, pit->get(), doc,
						url, renderOpts.m_linkColor );
				}
			}
			else if( !l->img()->isEmpty() )
			{
				CellItem item;
				item.image = loadImage( l->img().get() );
				item.url = url;
				item.font = { renderOpts.m_textFont,
					(bool) ( l->opts() & MD::TextOption::BoldText ),
					(bool) ( l->opts() & MD::TextOption::ItalicText ),
					(bool) ( l->opts() & MD::TextOption::StrikethroughText ),
					renderOpts.m_textFontSize };

				data.items.append( item );
			}
			else if( !l->text().isEmpty() )
			{
				const auto words = l->text().split( QLatin1Char( ' ' ),
					Qt::SkipEmptyParts );

				for( const auto & w : words )
				{
					CellItem item;
					item.word = w;
					item.font = { renderOpts.m_textFont,
						(bool) ( l->opts() & MD::TextOption::BoldText ),
						(bool) ( l->opts() & MD::TextOption::ItalicText ),
						(bool) ( l->opts() & MD::TextOption::StrikethroughText ),
						renderOpts.m_textFontSize };
					item.url = url;
					item.color = renderOpts.m_linkColor;

					data.items.append( item );
				}
			}
			else
			{
				CellItem item;
				item.font = { renderOpts.m_textFont,
					(bool) ( l->opts() & MD::TextOption::BoldText ),
					(bool) ( l->opts() & MD::TextOption::ItalicText ),
					(bool) ( l->opts() & MD::TextOption::StrikethroughText ),
					renderOpts.m_textFontSize };
				item.url = url;
				item.color = renderOpts.m_linkColor;

				data.items.append( item );
			}
		}
			break;

		case MD::ItemType::Image :
		{
			auto * i = static_cast< MD::Image< MD::QStringTrait >* > ( item );

			CellItem item;

			emit status( tr( "Loading image." ) );

			item.image = loadImage( i );
			item.font = { renderOpts.m_textFont,
				false, false, false, renderOpts.m_textFontSize };

			if( !url.isEmpty() )
				item.url = url;

			data.items.append( item );
		}
			break;

		case MD::ItemType::FootnoteRef :
		{
			auto * ref = static_cast< MD::FootnoteRef< MD::QStringTrait >* > ( item );

			const auto fit = doc->footnotesMap().find( ref->id() );

			if( fit != doc->footnotesMap().cend() )
			{
				CellItem item;
				item.font = { renderOpts.m_textFont,
					false, false, false,
					renderOpts.m_textFontSize };

				auto anchorIt = pdfData.footnotesAnchorsMap.constFind( fit->second.get() );
				int num = m_footnoteNum;

				if( anchorIt == pdfData.footnotesAnchorsMap.cend() )
				{
					pdfData.footnotesAnchorsMap.insert( fit->second.get(),
						{ pdfData.currentFile, m_footnoteNum++ } );
				}
				else
					num = anchorIt->second;

				item.footnote = QString::number( num );
				item.footnoteRef = ref->id();
				item.footnoteObj = fit->second;

				if( !url.isEmpty() )
					item.url = url;

				if( color.isValid() )
					item.color = color;

				data.items.append( item );
			}
			else
				handleText();
		}
			break;

		default :
			break;
	}
}

QVector< QVector< PdfRenderer::CellData > >
PdfRenderer::createAuxTable( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Table< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc, double scale )
{
	Q_UNUSED( pdfData )
	Q_UNUSED( scale )

	const auto columnsCount = item->columnsCount();

	QVector< QVector< CellData > > auxTable;
	auxTable.resize( columnsCount );

	for( auto rit = item->rows().cbegin(), rlast = item->rows().cend(); rit != rlast; ++rit )
	{
		int i = 0;

		for( auto cit = (*rit)->cells().cbegin(), clast = (*rit)->cells().cend(); cit != clast; ++cit )
		{
			if( i == columnsCount )
				break;

			CellData data;
			data.alignment = item->columnAlignment( i );

			for( auto it = (*cit)->items().cbegin(), last = (*cit)->items().cend(); it != last; ++it )
				createAuxCell( renderOpts, pdfData, data, it->get(), doc );

			auxTable[ i ].append( data );

			++i;
		}

		for( ; i < columnsCount; ++i )
			auxTable[ i ].append( CellData() );
	}

	return auxTable;
}

void
PdfRenderer::calculateCellsSize( PdfAuxData & pdfData, QVector< QVector< CellData > > & auxTable,
	double spaceWidth, double offset, double lineHeight, double scale )
{
	QVector< double > columnWidthes;
	columnWidthes.resize( auxTable.size() );

	const auto availableWidth = pdfData.coords.pageWidth - pdfData.coords.margins.left -
		pdfData.coords.margins.right - offset;

	const auto width = availableWidth / auxTable.size();

	for( auto it = auxTable.begin(), last = auxTable.end(); it != last; ++it )
	{
		for( auto cit = it->begin(), clast = it->end(); cit != clast; ++cit )
			cit->setWidth( width - c_tableMargin * 2.0 );
	}

	for( auto it = auxTable.begin(), last = auxTable.end(); it != last; ++it )
		for( auto cit = it->begin(), clast = it->end(); cit != clast; ++cit )
			cit->heightToWidth( lineHeight, spaceWidth, scale, pdfData, this );
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawTable( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	MD::Table< MD::QStringTrait > * item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	double offset, CalcHeightOpt heightCalcOpt,
	double scale )
{
	QVector< WhereDrawn > ret;

	pdfData.startLine = item->startLine();
	pdfData.startPos = item->startColumn();
	pdfData.endLine = item->endLine();
	pdfData.endPos = item->endColumn();

	{
		QMutexLocker lock( &m_mutex );

		if( m_terminate )
			return {};
	}

	if( heightCalcOpt == CalcHeightOpt::Unknown )
		emit status( tr( "Drawing table." ) );

	auto * font = createFont( renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
		pdfData.doc, scale, pdfData );

	const auto lineHeight = pdfData.lineSpacing( font, renderOpts.m_textFontSize, scale );
	const auto spaceWidth = pdfData.stringWidth( font, renderOpts.m_textFontSize, scale, " " );

	auto auxTable = createAuxTable( pdfData, renderOpts, item, doc, scale );

	calculateCellsSize( pdfData, auxTable, spaceWidth, offset, lineHeight, scale );

	const auto r0h = rowHeight( auxTable, 0 );
	const bool justHeader = auxTable.at( 0 ).size() == 1;
	const auto r1h = ( !justHeader ? rowHeight( auxTable, 1 ) : 0 );

	switch( heightCalcOpt )
	{
		case CalcHeightOpt::Minimum :
		{
			ret.append( { -1, 0.0, r0h + r1h + ( c_tableMargin * ( justHeader ? 2.0 : 4.0 ) ) +
				lineHeight - ( pdfData.fontDescent( font, renderOpts.m_textFontSize, scale ) *
					( justHeader ? 1.0 : 2.0 ) ) } );

			return { ret, {} };
		}

		case CalcHeightOpt::Full :
		{
			ret.append( { -1, 0.0, r0h + r1h + ( c_tableMargin * ( justHeader ? 2.0 : 4.0 ) ) +
				lineHeight - ( pdfData.fontDescent( font, renderOpts.m_textFontSize, scale ) *
					( justHeader ? 1.0 : 2.0 ) ) } );

			for( int i = 2; i < auxTable.at( 0 ).size(); ++i )
				ret.append( { -1, 0.0, rowHeight( auxTable, i ) + c_tableMargin * 2.0 -
					pdfData.fontDescent( font, renderOpts.m_textFontSize, scale ) } );

			return { ret, {} };
		}

		default :
			break;
	}

	const auto nonSplittableHeight = ( r0h + r1h + ( c_tableMargin * ( justHeader ? 2.0 : 4.0 ) ) -
		( pdfData.fontDescent( font, renderOpts.m_textFontSize, scale ) *
			( justHeader ? 1.0 : 2.0 ) ) );

	if( pdfData.coords.y - nonSplittableHeight < pdfData.currentPageAllowedY() &&
		qAbs( pdfData.coords.y - nonSplittableHeight - pdfData.currentPageAllowedY() ) > 0.1 )
	{
		createPage( pdfData );

		pdfData.freeSpaceOn( pdfData.currentPageIndex() );
	}

	moveToNewLine( pdfData, offset, lineHeight, 1.0, lineHeight );

	QVector< QPair< QString, std::shared_ptr< MD::Footnote< MD::QStringTrait > > > > footnotes;
	bool first = true;
	WhereDrawn firstLine;

	for( int row = 0; row < auxTable[ 0 ].size(); ++row )
	{
		const auto where = drawTableRow( auxTable, row, pdfData, offset, lineHeight, renderOpts,
			doc, footnotes, scale );

		ret.append( where.first );

		if( first )
		{
			firstLine = where.second;
			first = false;
		}
	}

	for( const auto & f : std::as_const( footnotes ) )
		addFootnote( f.first, f.second, pdfData, renderOpts, doc );

	return { ret, firstLine };
}

void
PdfRenderer::addFootnote( const QString & refId,
	std::shared_ptr< MD::Footnote< MD::QStringTrait > > f, PdfAuxData & pdfData,
	const RenderOpts & renderOpts, std::shared_ptr< MD::Document< MD::QStringTrait > > doc )
{
	std::function< void( MD::Block< MD::QStringTrait > * b, PdfAuxData & pdfData ) > findFootnoteRefs;

	findFootnoteRefs = [&] ( MD::Block< MD::QStringTrait > * b, PdfAuxData & pdfData )
	{
		for( auto it = b->items().cbegin(), last = b->items().cend(); it != last; ++it )
		{
			auto cb = dynamic_cast< MD::Block< MD::QStringTrait >* > ( it->get() );

			if( cb )
				findFootnoteRefs( cb, pdfData );
			else
			{
				switch( (*it)->type() )
				{
					case MD::ItemType::Heading :
						findFootnoteRefs( static_cast< MD::Heading< MD::QStringTrait >* > (
							it->get() )->text().get(), pdfData );
						break;

					case MD::ItemType::Table :
					{
						auto t = static_cast< MD::Table< MD::QStringTrait >* > ( it->get() );

						for( const auto & r: t->rows() )
						{
							for( const auto & c : r->cells() )
								findFootnoteRefs( c.get(), pdfData );
						}
					}
						break;

					case MD::ItemType::FootnoteRef :
					{
						auto * ref = static_cast< MD::FootnoteRef< MD::QStringTrait >* > ( it->get() );

						const auto fit = doc->footnotesMap().find( ref->id() );

						if( fit != doc->footnotesMap().cend() )
						{
							this->addFootnote( ref->id(), fit->second, pdfData,
								renderOpts, doc );
						}
					}
						break;

					default :
						break;
				}
			}
		}
	};

	if( std::find_if( m_footnotes.cbegin(), m_footnotes.cend(),
		[&refId] ( const auto & p ) { return p.first == refId; } ) == m_footnotes.cend() )
	{
		m_footnotes.append( { refId, f } );

		PdfAuxData tmpData = pdfData;
		tmpData.coords = { { pdfData.coords.margins.left, pdfData.coords.margins.right,
				pdfData.coords.margins.top, pdfData.coords.margins.bottom },
			pdfData.page->GetRect().Width,
			pdfData.page->GetRect().Height,
			pdfData.coords.margins.left, pdfData.page->GetRect().Height -
				pdfData.coords.margins.top };

		double lineHeight = 0.0;
		auto h = footnoteHeight( tmpData, renderOpts,
			doc, f.get(), &lineHeight );

		reserveSpaceForFootnote( pdfData, renderOpts, h, pdfData.coords.y,
			pdfData.currentPageIdx, lineHeight );

		pdfData.footnotesAnchorsMap = tmpData.footnotesAnchorsMap;

		findFootnoteRefs( f.get(), pdfData );
	}
}

QPair< QVector< WhereDrawn >, WhereDrawn >
PdfRenderer::drawTableRow( QVector< QVector< CellData > > & table, int row, PdfAuxData & pdfData,
	double offset, double lineHeight, const RenderOpts & renderOpts,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	QVector< QPair< QString, std::shared_ptr< MD::Footnote< MD::QStringTrait > > > > & footnotes,
	double scale )
{
	QVector< WhereDrawn > ret;

	{
		QMutexLocker lock( &m_mutex );

		if( m_terminate )
			return {};
	}

	emit status( tr( "Drawing table row." ) );

	auto * textFont = createFont( renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
		pdfData.doc, scale, pdfData );

	const auto startPage = pdfData.currentPageIndex();
	const auto startY = pdfData.coords.y;
	auto endPage = startPage;
	auto endY = startY;
	int currentPage = startPage;
	const auto firstLinePageIdx = startPage;
	const auto firstLineHeight = lineHeight;
	const auto firstLineY = startY - firstLineHeight - c_tableMargin;

	TextToDraw text;
	QMap< QString, QVector< QPair< QRectF, unsigned int > > > links;

	int column = 0;

	// Draw cells.
	for( auto it = table.cbegin(), last = table.cend(); it != last; ++it )
	{
		{
			QMutexLocker lock( &m_mutex );

			if( m_terminate )
				return {};
		}

		emit status( tr( "Drawing table cell." ) );

		text.alignment = it->at( 0 ).alignment;
		text.availableWidth = it->at( 0 ).width;
		text.lineHeight = lineHeight;

		pdfData.currentPainterIdx = startPage;

		currentPage = startPage;

		auto startX = pdfData.coords.margins.left + offset;

		for( int i = 0; i < column; ++i )
			startX += table[ i ][ 0 ].width + c_tableMargin * 2.0;

		startX += c_tableMargin;

		double x = startX;
		double y = startY - c_tableMargin;

		if( y < pdfData.currentPageAllowedY() && qAbs( y - pdfData.currentPageAllowedY() ) > 0.1 )
		{
			newPageInTable( pdfData, currentPage, endPage, endY );

			y = pdfData.topY( currentPage );

			if( pdfData.drawFootnotes )
				y -= pdfData.extraInFootnote;
		}

		bool textBefore = false;
		const CellItem * lastItemInCell = it->at( row ).items.isEmpty() ?
			nullptr : &( it->at( row ).items.back() );
		const bool wasTextInLastPos = lastItemInCell ? ( !lastItemInCell->word.isEmpty() ||
			( lastItemInCell->image.isEmpty() && !lastItemInCell->url.isEmpty() ) ||
			!lastItemInCell->footnote.isEmpty() ) : false;

		bool addMargin = false;

		for( auto c = it->at( row ).items.cbegin(), clast = it->at( row ).items.cend(); c != clast; ++c )
		{
			if( !c->image.isNull() && !text.text.isEmpty() )
			{
				drawTextLineInTable( renderOpts, x, y, text, lineHeight, pdfData,
					links, textFont, currentPage,
					endPage, endY, footnotes, scale );
				addMargin = false;
			}

			if( !c->image.isNull() )
			{
				if( textBefore )
					y -= lineHeight;

				if( addMargin )
					y -= c_tableMargin;

				auto img = pdfData.doc->CreateImage();
				img->LoadFromBuffer( { c->image.data(), static_cast< size_t > ( c->image.size() ) } );

				const double iWidth = std::round( (double) img->GetWidth() /
					(double) pdfData.dpi * 72.0 );
				const double iHeight = std::round( (double) img->GetHeight() /
					(double) pdfData.dpi * 72.0 );
				const double dpiScale = (double) img->GetWidth() / iWidth;

				auto ratio = ( iWidth > it->at( 0 ).width ? it->at( 0 ).width / iWidth * scale :
					1.0 * scale );

				auto h = iHeight * ratio;

				if(  y - h < pdfData.currentPageAllowedY() &&
					qAbs( y - h - pdfData.currentPageAllowedY() ) > 0.1 )
				{
					newPageInTable( pdfData, currentPage, endPage, endY );

					y = pdfData.topY( currentPage );

					if( pdfData.drawFootnotes )
						y -= pdfData.extraInFootnote;
				}

				const auto availableHeight = pdfData.topY( currentPage ) -
					pdfData.currentPageAllowedY();

				if( h > availableHeight )
					ratio = availableHeight / iHeight;

				const auto w = iWidth * ratio;
				auto o = 0.0;

				if( w < table[ column ][ 0 ].width )
					o = ( table[ column ][ 0 ].width - w ) / 2.0;

				y -= iHeight * ratio;

				pdfData.drawImage( x + o, y, img.get(), ratio / dpiScale, ratio / dpiScale );

				if( !c->url.isEmpty() )
					links[ c->url ].append( qMakePair( QRectF( x + o, y,
							c->width( pdfData, this, scale ),
							iHeight * ratio ),
						currentPage ) );

				textBefore = false;
				addMargin = true;
			}
			else
			{
				addMargin = false;

				auto * font = createFont( c->font.family, c->font.bold, c->font.italic,
					c->font.size, pdfData.doc, scale, pdfData );

				auto w = pdfData.stringWidth( font, c->font.size, scale,
					createUtf8String( c->word.isEmpty() ? c->url : c->word ) );
				double s = 0.0;

				if( !text.text.isEmpty() )
				{
					if( text.text.last().font == c->font )
						s = pdfData.stringWidth( font, c->font.size, scale, " " );
					else
						s = pdfData.stringWidth( textFont, renderOpts.m_textFontSize, scale, " " );
				}

				double fw = 0.0;

				if( c + 1 != clast && !( c + 1 )->footnote.isEmpty() )
				{
					auto * f1 = createFont( ( c + 1 )->font.family, ( c + 1 )->font.bold,
						( c + 1 )->font.italic, ( c + 1 )->font.size, pdfData.doc, scale, pdfData );

					fw = pdfData.stringWidth( f1, ( c + 1 )->font.size, scale,
						createUtf8String( ( c + 1 )->footnote ) );
					w += fw;
				}

				if( text.width + s + w < it->at( 0 ).width ||
					qAbs( text.width + s + w - it->at( 0 ).width ) < 0.01 )
				{
					text.text.append( *c );

					if( c + 1 != clast && !( c + 1 )->footnote.isEmpty() )
						text.text.append( *( c + 1 ) );

					text.width += s + w;
				}
				else
				{
					if( !text.text.isEmpty() )
					{
						drawTextLineInTable( renderOpts, x, y, text, lineHeight, pdfData, links,
							textFont, currentPage, endPage, endY, footnotes, scale );
						text.text.append( *c );

						if( c + 1 != clast && !( c + 1 )->footnote.isEmpty() )
							text.text.append( *( c + 1 ) );

						text.width += w;
					}
					else
					{
						text.text.append( *c );
						text.width += w;
						drawTextLineInTable( renderOpts, x, y, text, lineHeight, pdfData, links,
							textFont, currentPage, endPage, endY, footnotes, scale );

						if( c + 1 != clast && !( c + 1 )->footnote.isEmpty() )
						{
							text.text.append( *( c + 1 ) );
							text.width += fw;
						}
					}
				}

				if( c + 1 != clast && !( c + 1 )->footnote.isEmpty() )
					++c;

				textBefore = true;
			}
		}

		if( !text.text.isEmpty() )
			drawTextLineInTable( renderOpts, x, y, text, lineHeight, pdfData,
				links, textFont, currentPage,
				endPage, endY, footnotes, scale );

		y -= c_tableMargin - ( wasTextInLastPos ?
			pdfData.fontDescent( textFont, renderOpts.m_textFontSize, scale ) : 0.0 );

		if( y < endY  && currentPage == pdfData.currentPageIndex() )
			endY = y;

		++column;
	}

	drawRowBorder( pdfData, startPage, ret, renderOpts, offset, table, startY, endY );

	pdfData.coords.y = endY;
	pdfData.currentPainterIdx = pdfData.currentPageIndex();

	processLinksInTable( pdfData, links, doc );

	return { ret, { firstLinePageIdx, firstLineY, firstLineHeight } };
}

void
PdfRenderer::drawRowBorder( PdfAuxData & pdfData, int startPage, QVector< WhereDrawn > & ret,
	const RenderOpts & renderOpts, double offset, const QVector< QVector< CellData > > & table,
	double startY, double endY )
{
	for( int i = startPage; i <= pdfData.currentPageIndex(); ++i )
	{
		pdfData.currentPainterIdx = i;

		pdfData.setColor( renderOpts.m_borderColor );

		const auto startX = pdfData.coords.margins.left + offset;
		auto endX = startX;

		for( int c = 0; c < table.size(); ++ c )
			endX += table.at( c ).at( 0 ).width + c_tableMargin * 2.0;

		if( i == startPage )
		{
			pdfData.drawLine( startX, startY, endX, startY );

			auto x = startX;
			auto y = endY;

			if( i == pdfData.currentPageIndex() )
			{
				pdfData.drawLine( startX, endY, endX, endY );
				pdfData.drawLine( x, startY, x, endY );
			}
			else
			{
				pdfData.drawLine( x, startY, x, pdfData.allowedY( i ) );
				y = pdfData.allowedY( i );
			}

			for( int c = 0; c < table.size(); ++c )
			{
				x += table.at( c ).at( 0 ).width + c_tableMargin * 2.0;

				pdfData.drawLine( x, startY, x, y );
			}

			ret.append( { i, ( i < pdfData.currentPageIndex() ? pdfData.allowedY( i ) : endY ),
				( i < pdfData.currentPageIndex() ? startY - pdfData.allowedY( i ) : startY - endY  ) } );
		}
		else if( i < pdfData.currentPageIndex() )
		{
			auto x = startX;
			auto y = pdfData.allowedY( i );
			auto sy = pdfData.topY( i );

			if( pdfData.drawFootnotes )
				sy -= pdfData.extraInFootnote + c_tableMargin;

			pdfData.drawLine( x, sy, x, y );

			for( int c = 0; c < table.size(); ++c )
			{
				x += table.at( c ).at( 0 ).width + c_tableMargin * 2.0;

				pdfData.drawLine( x, sy, x, y );
			}

			ret.append( { i, pdfData.allowedY( i ),
				sy - pdfData.allowedY( i ) } );
		}
		else
		{
			auto x = startX;
			auto y = endY;
			auto sy = pdfData.topY( i );

			if( pdfData.drawFootnotes )
				sy -= pdfData.extraInFootnote + c_tableMargin;

			pdfData.drawLine( x, sy, x, y );

			for( int c = 0; c < table.size(); ++c )
			{
				x += table.at( c ).at( 0 ).width + c_tableMargin * 2.0;

				pdfData.drawLine( x, sy, x, y );
			}

			pdfData.drawLine( startX, y, endX, y );

			ret.append( { pdfData.currentPageIndex(), endY,
				sy - endY } );
		}

		pdfData.restoreColor();
	}
}

void
PdfRenderer::drawTextLineInTable( const RenderOpts & renderOpts,
	double x, double & y, TextToDraw & text, double lineHeight,
	PdfAuxData & pdfData, QMap< QString, QVector< QPair< QRectF, unsigned int > > > & links,
	Font * font, int & currentPage, int & endPage, double & endY,
	QVector< QPair< QString, std::shared_ptr< MD::Footnote< MD::QStringTrait > > > > & footnotes,
	double scale )
{
	y -= lineHeight;

	if( y < pdfData.allowedY( currentPage ) )
	{
		newPageInTable( pdfData, currentPage, endPage, endY );

		y = pdfData.topY( currentPage ) - lineHeight;

		if( pdfData.drawFootnotes )
			y -= pdfData.extraInFootnote;
	}

	if( text.width < text.availableWidth ||
		qAbs( text.width - text.availableWidth ) < 0.01 )
	{
		switch( text.alignment )
		{
			case MD::Table< MD::QStringTrait >::AlignRight :
				x = x + text.availableWidth - text.width;
				break;

			case MD::Table< MD::QStringTrait >::AlignCenter :
				x = x + ( text.availableWidth - text.width ) / 2.0;
				break;

			default :
				break;
		}
	}
	else
	{
		const auto str = ( text.text.first().word.isEmpty() ? text.text.first().url :
			text.text.first().word );

		QString res;

		double w = 0.0;

		auto * f = createFont( text.text.first().font.family, text.text.first().font.bold,
			text.text.first().font.italic, text.text.first().font.size, pdfData.doc, scale, pdfData );

		for( const auto & ch : str )
		{
			w += pdfData.stringWidth( f, text.text.first().font.size, scale,
				createUtf8String( QString( ch ) ) );

			if( w >= text.availableWidth )
				break;
			else
				res.append( ch );
		}

		text.text.first().word = res;
	}

	for( auto it = text.text.cbegin(), last = text.text.cend(); it != last; ++it )
	{
		auto * f = createFont( it->font.family, it->font.bold,
			it->font.italic, it->font.size, pdfData.doc, scale, pdfData );

		if( it->background.isValid() )
		{
			pdfData.setColor( it->background );

			pdfData.drawRectangle( x, y + pdfData.fontDescent( f, it->font.size, scale ),
				it->width( pdfData, this, scale ), pdfData.lineSpacing( f, it->font.size, scale ),
				PoDoFo::PdfPathDrawMode::Fill );

			pdfData.restoreColor();
		}

		if( it->color.isValid() )
			pdfData.setColor( it->color );

		pdfData.drawText( x, y, createUtf8String( it->word.isEmpty() ?
			it->url : it->word ), f, it->font.size * scale, 1.0, it->font.strikethrough );

		pdfData.restoreColor();

		if( !it->url.isEmpty() )
			links[ it->url ].append( qMakePair( QRectF( x, y, it->width( pdfData, this, scale ),
				lineHeight ), currentPage ) );

		x += it->width( pdfData, this, scale );

		if( it + 1 != last && !( it + 1 )->footnote.isEmpty() )
		{
			++it;

			const auto str = createUtf8String( it->footnote );

			const auto w = pdfData.stringWidth( f, it->font.size * c_footnoteScale, scale, str );

			m_unresolvedFootnotesLinks.insert( it->footnoteRef,
				qMakePair( QRectF( x, y, w, lineHeight ),
					pdfData.currentPageIndex() ) );

			pdfData.setColor( renderOpts.m_linkColor );

			pdfData.drawText( x, y + lineHeight -
					pdfData.lineSpacing( f, it->font.size * c_footnoteScale, scale ),
				str, f, it->font.size * c_footnoteScale * scale, 1.0, false );

			pdfData.restoreColor();

			x += w;

			footnotes.append( { it->footnoteRef, it->footnoteObj } );
		}

		if( it + 1 != last )
		{
			auto tmpX = x;

			if( it->background.isValid() && it->font == ( it + 1 )->font )
			{
				pdfData.setColor( it->background );

				const auto sw = pdfData.stringWidth( f, it->font.size, scale, " " );

				pdfData.drawRectangle( x, y + pdfData.fontDescent( f, it->font.size, scale ),
					sw, pdfData.lineSpacing( f, it->font.size, scale ),
					PoDoFo::PdfPathDrawMode::Fill );

				x += sw;

				pdfData.restoreColor();
			}
			else
				x += pdfData.stringWidth( font, it->font.size, scale, " " );

			if( !( it + 1 )->url.isEmpty() && it->url == ( it + 1 )->url )
				links[ it->url ].append( qMakePair( QRectF( tmpX, y, x - tmpX, lineHeight ),
					currentPage ) );
		}
	}

	text.clear();
}

void
PdfRenderer::newPageInTable( PdfAuxData & pdfData, int & currentPage, int & endPage,
	double & endY )
{
	if( currentPage + 1 > pdfData.currentPageIndex() )
	{
		createPage( pdfData );

		if( pdfData.currentPageIndex() > endPage )
		{
			endPage = pdfData.currentPageIndex();
			endY = pdfData.coords.y;
		}

		++currentPage;
	}
	else
	{
		++currentPage;

		pdfData.currentPainterIdx = currentPage;
	}
}

void
PdfRenderer::processLinksInTable( PdfAuxData & pdfData,
	const QMap< QString, QVector< QPair< QRectF, unsigned int > > > & links,
	std::shared_ptr< MD::Document< MD::QStringTrait > > doc )
{
	for( auto it = links.cbegin(), last = links.cend(); it != last; ++it )
	{
		QString url = it.key();

		const auto lit = doc->labeledLinks().find( url );

		if( lit != doc->labeledLinks().cend() )
			url = lit->second->url();

		auto tmp = it.value();

		if( !tmp.isEmpty() )
		{
			QVector< QPair< QRectF, unsigned int > > rects;
			QPair< QRectF, unsigned int > r = tmp.first();

			for( auto rit = tmp.cbegin() + 1, rlast = tmp.cend(); rit != rlast; ++rit )
			{
				if( r.second == rit->second &&
					qAbs( r.first.x() + r.first.width() - rit->first.x() ) < 0.001 &&
					qAbs( r.first.y() - rit->first.y() ) < 0.001 )
				{
					r.first.setWidth( r.first.width() + rit->first.width() );
				}
				else
				{
					rects.append( r );
					r = *rit;
				}
			}

			rects.append( r );

			if( !pdfData.anchors.contains( url ) &&
				pdfData.md->labeledHeadings().find( url ) == pdfData.md->labeledHeadings().cend() )
			{
				for( const auto & r : std::as_const( rects ) )
				{
					auto & annot = pdfData.doc->GetPages().GetPageAt(
						static_cast< unsigned int >( r.second ) )
							.GetAnnotations().CreateAnnot< PoDoFo::PdfAnnotationLink >(
						Rect( r.first.x(), r.first.y(), r.first.width(), r.first.height() ) );
					annot.SetBorderStyle( 0.0, 0.0, 0.0 );

					auto action = std::make_shared< PoDoFo::PdfAction >( *pdfData.doc,
						PoDoFo::PdfActionType::URI );
					action->SetURI( url.toLatin1().data() );

					annot.SetAction( action );
				}
			}
			else
				m_unresolvedLinks.insert( url, rects );
		}
	}
}

double
PdfRenderer::minNecessaryHeight( PdfAuxData & pdfData, const RenderOpts & renderOpts,
	std::shared_ptr< MD::Item< MD::QStringTrait > > item, std::shared_ptr< MD::Document< MD::QStringTrait > > doc,
	double offset, double scale )
{
	QVector< WhereDrawn > ret;

	PdfAuxData tmp = pdfData;
	tmp.coords.y = tmp.coords.pageHeight - tmp.coords.margins.top;
	tmp.coords.x = tmp.coords.margins.left + offset;

	switch( item->type() )
	{
		case MD::ItemType::Heading :
			return 0.0;

		case MD::ItemType::Paragraph :
		{
			ret = drawParagraph( tmp, renderOpts, static_cast< MD::Paragraph< MD::QStringTrait >* > ( item.get() ),
				doc, offset, true, CalcHeightOpt::Minimum, scale ).first;
		}
			break;

		case MD::ItemType::Code :
		{
			ret = drawCode( tmp, renderOpts, static_cast< MD::Code< MD::QStringTrait >* > ( item.get() ),
				doc, offset, CalcHeightOpt::Minimum, scale ).first;
		}
			break;

		case MD::ItemType::Blockquote :
		{
			ret = drawBlockquote( tmp, renderOpts,
				static_cast< MD::Blockquote< MD::QStringTrait >* > ( item.get() ),
				doc, offset, CalcHeightOpt::Minimum, scale ).first;
		}
			break;

		case MD::ItemType::List :
		{
			auto * list = static_cast< MD::List< MD::QStringTrait >* > ( item.get() );
			const auto bulletWidth = maxListNumberWidth( list );

			ret = drawList( tmp, m_opts, list, m_doc, bulletWidth, offset,
				CalcHeightOpt::Minimum, scale ).first;
		}
			break;

		case MD::ItemType::Table :
		{
			ret = drawTable( tmp, renderOpts,
				static_cast< MD::Table< MD::QStringTrait >* > ( item.get() ),
				doc, offset, CalcHeightOpt::Minimum, scale ).first;
		}
			break;

		default :
			break;
	}

	if( !ret.isEmpty() )
		return ret.constFirst().height;
	else
		return 0.0;
}
