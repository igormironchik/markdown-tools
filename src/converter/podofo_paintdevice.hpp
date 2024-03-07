
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

#ifndef MD_PDF_PODOFO_PAINTDEVICE_HPP_INCLUDED
#define MD_PDF_PODOFO_PAINTDEVICE_HPP_INCLUDED

// Qt include.
#include <QPaintDevice>
#include <QPaintEngine>
#include <QScopedPointer>

// podofo include.
#include <podofo/podofo.h>


//
// PoDoFoPaintDevice
//

struct PoDoFoPaintDevicePrivate;

//! Paint device to draw on PoDoFo page.
class PoDoFoPaintDevice final
	:	public QPaintDevice
{
public:
	PoDoFoPaintDevice();
	~PoDoFoPaintDevice() override;

	void setPdfPainter( PoDoFo::PdfPainter & p, PoDoFo::PdfDocument & doc );

	QPaintEngine * paintEngine() const override;

protected:
	int metric( QPaintDevice::PaintDeviceMetric metric ) const override;

private:
	Q_DISABLE_COPY( PoDoFoPaintDevice )

	QScopedPointer< PoDoFoPaintDevicePrivate > d;
}; // class PoDoFoPaintDevice


//
// PoDoFoPaintEngine
//

struct PoDoFoPaintEnginePrivate;

//! Paint engine to draw on PoDoFo page.
class PoDoFoPaintEngine final
	:	public QPaintEngine
{
public:
	PoDoFoPaintEngine();
	~PoDoFoPaintEngine() override;

	void setPdfPainter( PoDoFo::PdfPainter & p, PoDoFo::PdfDocument & doc );
	PoDoFo::PdfPainter * pdfPainter() const;

	bool begin( QPaintDevice * pdev ) override;
	void drawEllipse( const QRectF & rect ) override;
	void drawEllipse( const QRect & rect ) override;
	void drawImage( const QRectF & rectangle, const QImage & image,
		const QRectF & sr, Qt::ImageConversionFlags flags = Qt::AutoColor ) override;
	void drawLines( const QLineF * lines, int lineCount ) override;
	void drawLines( const QLine * lines, int lineCount ) override;
	void drawPath( const QPainterPath & path ) override;
	void drawPixmap( const QRectF & r, const QPixmap & pm, const QRectF & sr) override;
	void drawPoints( const QPointF * points, int pointCount ) override;
	void drawPoints( const QPoint * points, int pointCount ) override;
	void drawPolygon( const QPointF * points, int pointCount,
		QPaintEngine::PolygonDrawMode mode ) override;
	void drawPolygon( const QPoint * points, int pointCount,
		QPaintEngine::PolygonDrawMode mode ) override;
	void drawRects( const QRectF * rects, int rectCount ) override;
	void drawRects( const QRect * rects, int rectCount ) override;
	void drawTextItem( const QPointF & p, const QTextItem & textItem ) override;
	void drawTiledPixmap( const QRectF & rect, const QPixmap & pixmap,
		const QPointF & p ) override;
	bool end() override;
	QPaintEngine::Type type() const override;
	void updateState( const QPaintEngineState & state ) override;

private:
	double qXtoPoDoFo( double x );
	double qYtoPoDoFo( double y );
	double qWtoPoDoFo( double w );
	double qHtoPoDoFo( double h );
	PoDoFo::Rect qRectFtoPoDoFo( const QRectF & r );
	QPair< PoDoFo::PdfFont*, double > qFontToPoDoFo( const QFont & f );

private:
	Q_DISABLE_COPY( PoDoFoPaintEngine )

	QScopedPointer< PoDoFoPaintEnginePrivate > d;
}; // class PoDoFoPaintEngine

#endif // MD_PDF_PODOFO_PAINTDEVICE_HPP_INCLUDED
