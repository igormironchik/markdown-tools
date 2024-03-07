
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
#include "podofo_paintdevice.hpp"

// Qt include.
#include <QPainterPath>
#include <QTextItem>
#include <QTransform>


//
// PoDoFoPaintDevicePrivate
//

struct PoDoFoPaintDevicePrivate {
	PoDoFoPaintDevicePrivate( PoDoFoPaintDevice * parent )
		:	q( parent )
	{
	}

	//! Parent.
	PoDoFoPaintDevice * q = nullptr;
	//! Paint engine.
	PoDoFoPaintEngine engine;
}; // struct PoDoFoPaintDevicePrivate


//
// PoDoFoPaintDevice
//

PoDoFoPaintDevice::PoDoFoPaintDevice()
	:	d( new PoDoFoPaintDevicePrivate( this ) )
{
}

PoDoFoPaintDevice::~PoDoFoPaintDevice()
{
}

void
PoDoFoPaintDevice::setPdfPainter( PoDoFo::PdfPainter & p, PoDoFo::PdfDocument & doc )
{
	d->engine.setPdfPainter( p, doc );
}

QPaintEngine *
PoDoFoPaintDevice::paintEngine() const
{
	return &d->engine;
}

int
PoDoFoPaintDevice::metric( QPaintDevice::PaintDeviceMetric metric ) const
{
	switch( metric )
	{
		case PdmWidth :
			return qRound( d->engine.pdfPainter() ?
				d->engine.pdfPainter()->GetCanvas()->GetRectRaw().Width / 72.0 * 1200.0 :
				100 );

        case PdmHeight :
			return qRound( d->engine.pdfPainter() ?
				d->engine.pdfPainter()->GetCanvas()->GetRectRaw().Height / 72.0 * 1200.0 :
				100 );

        case PdmWidthMM :
			return qRound( d->engine.pdfPainter() ?
				d->engine.pdfPainter()->GetCanvas()->GetRectRaw().Width / 72.0 * 25.4 :
				100.0 / 1200.0 * 25.4 );

        case PdmHeightMM :
			return qRound( d->engine.pdfPainter() ?
				d->engine.pdfPainter()->GetCanvas()->GetRectRaw().Height / 72.0 * 25.4 :
				100.0 / 1200.0 * 25.4 );

        case PdmNumColors :
			return INT_MAX;

        case PdmDepth :
			return 32;

        case PdmDpiX :
        case PdmDpiY :
        case PdmPhysicalDpiX :
        case PdmPhysicalDpiY :
			return 1200;

        case PdmDevicePixelRatio :
			return 1;

        case PdmDevicePixelRatioScaled :
			return 1 * QPaintDevice::devicePixelRatioFScale();
	}

	return -1;
}


//
// PoDoFoPaintEnginePrivate
//

struct PoDoFoPaintEnginePrivate {
	PoDoFoPaintEnginePrivate( PoDoFoPaintEngine * parent )
		:	q( parent )
	{
	}

	//! Parent.
	PoDoFoPaintEngine * q = nullptr;
	//! Pdf painter.
	PoDoFo::PdfPainter * painter = nullptr;
	//! Pdf document.
	PoDoFo::PdfDocument * doc = nullptr;
	//! Transformation.
	QTransform transform;
}; // struct PoDoFoPaintEnginePrivate


//
// PoDoFoPaintEngine
//

PoDoFoPaintEngine::PoDoFoPaintEngine()
	:	d( new PoDoFoPaintEnginePrivate( this ) )
{
}

PoDoFoPaintEngine::~PoDoFoPaintEngine()
{
}

void
PoDoFoPaintEngine::setPdfPainter( PoDoFo::PdfPainter & p, PoDoFo::PdfDocument & doc )
{
	d->painter = &p;
	d->doc = &doc;
}

PoDoFo::PdfPainter *
PoDoFoPaintEngine::pdfPainter() const
{
	return d->painter;
}

bool
PoDoFoPaintEngine::begin( QPaintDevice * )
{
	return true;
}

void
PoDoFoPaintEngine::drawEllipse( const QRectF & )
{
}

void
PoDoFoPaintEngine::drawEllipse( const QRect & )
{
}

void
PoDoFoPaintEngine::drawImage( const QRectF &, const QImage &,
	const QRectF &, Qt::ImageConversionFlags )
{
}

double
PoDoFoPaintEngine::qXtoPoDoFo( double x )
{
	return x / paintDevice()->physicalDpiX() * 72.0;
}

double
PoDoFoPaintEngine::qYtoPoDoFo( double y )
{
	return ( paintDevice()->height() - y ) / paintDevice()->physicalDpiY() * 72.0;
}

void
PoDoFoPaintEngine::drawLines( const QLineF * lines, int lineCount )
{
	if( d->painter )
	{
		for( int i = 0; i < lineCount; ++i )
		{
			const auto l = d->transform.map( lines[ i ] );

			d->painter->DrawLine( qXtoPoDoFo( l.x1() ), qYtoPoDoFo( l.y1() ),
				qXtoPoDoFo( l.x2() ), qYtoPoDoFo( l.y2() ) );
		}
	}
}

void
PoDoFoPaintEngine::drawLines( const QLine * lines, int lineCount )
{
	if( d->painter )
	{
		for( int i = 0; i < lineCount; ++i )
		{
			const auto l = d->transform.map( lines[ i ] );

			d->painter->DrawLine( qXtoPoDoFo( l.x1() ), qYtoPoDoFo( l.y1() ),
				qXtoPoDoFo( l.x2() ), qYtoPoDoFo( l.y2() ) );
		}
	}
}

void
PoDoFoPaintEngine::drawPath( const QPainterPath & path )
{
	if( d->painter )
	{
		PoDoFo::PdfPainterPath p;
		QPointF start, end;
		bool initialized = false;

		for( int i = 0; i < path.elementCount(); ++i )
		{
			const auto & e = path.elementAt( i );

			switch( e.type )
			{
				case QPainterPath::MoveToElement :
				{
					const auto pp = d->transform.map( QPointF( e.x, e.y ) );

					if( !initialized )
					{
						start = { pp.x(), pp.y() };
						initialized = true;
					}

					p.MoveTo( qXtoPoDoFo( pp.x() ), qYtoPoDoFo( pp.y() ) );
				}
					break;

				case QPainterPath::LineToElement :
				{
					const auto pp = d->transform.map( QPointF( e.x, e.y ) );

					p.AddLineTo( qXtoPoDoFo( pp.x() ), qYtoPoDoFo( pp.y() ) );
					end = { pp.x(), pp.y() };
				}
					break;

				case QPainterPath::CurveToElement :
				{
					Q_ASSERT( path.elementAt( i + 1 ).type == QPainterPath::CurveToDataElement );
					Q_ASSERT( path.elementAt( i + 2 ).type == QPainterPath::CurveToDataElement );

					const auto pp = d->transform.map( QPointF( e.x, e.y ) );
					const auto pp1 = d->transform.map( QPointF( path.elementAt( i + 1 ).x,
						path.elementAt( i + 1 ).y ) );
					const auto pp2 = d->transform.map( QPointF( path.elementAt( i + 2 ).x,
						path.elementAt( i + 2 ).y ) );

					end = { pp.x(), pp.y() };

					p.AddCubicBezierTo( qXtoPoDoFo( pp.x() ), qYtoPoDoFo( pp.y() ),
						qXtoPoDoFo( pp1.x() ), qYtoPoDoFo( pp1.y() ),
						qXtoPoDoFo( pp2.x() ), qYtoPoDoFo( pp2.y() ) );

					i += 2;
				}
					break;

				default :
					break;
			}
		}

		d->painter->DrawPath( p, start == end ? PoDoFo::PdfPathDrawMode::StrokeFill :
			PoDoFo::PdfPathDrawMode::Stroke );
	}
}

void
PoDoFoPaintEngine::drawPixmap( const QRectF &, const QPixmap &, const QRectF & )
{
}

void
PoDoFoPaintEngine::drawPoints( const QPointF *, int )
{
}

void
PoDoFoPaintEngine::drawPoints( const QPoint *, int )
{
}

void
PoDoFoPaintEngine::drawPolygon( const QPointF *, int,
	QPaintEngine::PolygonDrawMode )
{
}

void
PoDoFoPaintEngine::drawPolygon( const QPoint *, int,
	QPaintEngine::PolygonDrawMode )
{
}

double
PoDoFoPaintEngine::qWtoPoDoFo( double w )
{
	return w / paintDevice()->physicalDpiX() * 72.0;
}

double
PoDoFoPaintEngine::qHtoPoDoFo( double h )
{
	return h / paintDevice()->physicalDpiY() * 72.0;
}

PoDoFo::Rect
PoDoFoPaintEngine::qRectFtoPoDoFo( const QRectF & r )
{
	return PoDoFo::Rect( qXtoPoDoFo( r.x() ), qYtoPoDoFo( r.y() + r.height() ),
		qWtoPoDoFo( r.width() ), qHtoPoDoFo( r.height() ) );
}

void
PoDoFoPaintEngine::drawRects( const QRectF * rects, int rectCount )
{
	if( d->painter )
	{
		for( int i = 0; i < rectCount; ++i )
			d->painter->DrawRectangle( qRectFtoPoDoFo(
				d->transform.mapRect( rects[ i ] ) ) );
	}
}

void
PoDoFoPaintEngine::drawRects( const QRect * rects, int rectCount )
{
	if( d->painter )
	{
		for( int i = 0; i < rectCount; ++i )
			d->painter->DrawRectangle( qRectFtoPoDoFo(
				d->transform.mapRect( rects[ i ].toRectF() ) ) );
	}
}

void
PoDoFoPaintEngine::drawTextItem( const QPointF & p, const QTextItem & textItem )
{
	if( d->painter )
	{
		const auto f = qFontToPoDoFo( textItem.font() );
		const auto pp = d->transform.map( p );

		d->painter->TextObject.Begin();
		d->painter->TextObject.MoveTo( qXtoPoDoFo( pp.x() ),
			qYtoPoDoFo( pp.y() ) );
		d->painter->TextState.SetFont( *f.first, f.second );
		d->painter->TextState.SetFontScale( 1.0 );
		const auto utf8 = textItem.text().toUtf8();
		d->painter->TextObject.AddText( utf8.data() );
		d->painter->TextObject.End();
	}
}

void
PoDoFoPaintEngine::drawTiledPixmap( const QRectF &, const QPixmap &,
	const QPointF & )
{
}

bool
PoDoFoPaintEngine::end()
{
	return true;
}

QPaintEngine::Type
PoDoFoPaintEngine::type() const
{
	return QPaintEngine::Pdf;
}

inline PoDoFo::PdfLineCapStyle
capStyle( Qt::PenCapStyle s )
{
	switch( s )
	{
		case Qt::SquareCap :
			return PoDoFo::PdfLineCapStyle::Square;

		case Qt::FlatCap :
			return PoDoFo::PdfLineCapStyle::Butt;

		case Qt::RoundCap :
		default :
			return PoDoFo::PdfLineCapStyle::Round;
	}
}

inline PoDoFo::PdfLineJoinStyle
joinStyle( Qt::PenJoinStyle s )
{
	switch( s )
	{
		case Qt::MiterJoin :
			return PoDoFo::PdfLineJoinStyle::Miter;

		case Qt::BevelJoin :
			return PoDoFo::PdfLineJoinStyle::Bevel;

		case Qt::RoundJoin :
		default :
			return PoDoFo::PdfLineJoinStyle::Round;
	}
}

inline PoDoFo::PdfColor
color( const QColor & c )
{
	return PoDoFo::PdfColor( c.redF(), c.greenF(), c.blueF() );
}

QPair< PoDoFo::PdfFont*, double >
PoDoFoPaintEngine::qFontToPoDoFo( const QFont & f )
{
	PoDoFo::PdfFontSearchParams params;
	params.Style = PoDoFo::PdfFontStyle::Regular;
	if( f.bold() ) params.Style.value() |= PoDoFo::PdfFontStyle::Bold;
	if( f.italic() ) params.Style.value() |= PoDoFo::PdfFontStyle::Italic;

	auto * font = d->doc->GetFonts().SearchFont( f.family().toLocal8Bit().data(), params );

	const double size = f.pointSizeF() > 0.0 ? f.pointSizeF() :
		f.pixelSize() / paintDevice()->physicalDpiY() * 72.0;

	return { font, size };
}

void
PoDoFoPaintEngine::updateState( const QPaintEngineState & state )
{
	if( !d->painter )
		return;

	const auto st = state.state();

	if( st & QPaintEngine::DirtyPen )
	{
		const auto p = state.pen();

		d->painter->GraphicsState.SetLineWidth( p.widthF() / paintDevice()->physicalDpiX() * 72.0 );
		d->painter->GraphicsState.SetLineCapStyle( capStyle( p.capStyle() ) );
		d->painter->GraphicsState.SetLineJoinStyle( joinStyle( p.joinStyle() ) );
		d->painter->GraphicsState.SetFillColor( color( p.brush().color() ) );
		d->painter->GraphicsState.SetStrokeColor( color( p.color() ) );
		d->painter->GraphicsState.SetMiterLevel( p.miterLimit() );
	}

	if( st & QPaintEngine::DirtyBrush )
		d->painter->GraphicsState.SetFillColor( color( state.brush().color() ) );

	if( st & QPaintEngine::DirtyFont )
	{
		const auto f = qFontToPoDoFo( state.font() );

		d->painter->TextState.SetFont( *f.first, f.second );
	}

	if( st && QPaintEngine::DirtyTransform )
		d->transform = state.transform();
}
