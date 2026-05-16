/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QPaintDevice>
#include <QPaintEngine>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTemporaryFile>

// podofo include.
#include <podofo/podofo.h>

namespace MdPdf
{

namespace Render
{

struct PdfAuxData;

}

//
// PoDoFoPaintDevice
//

struct PoDoFoPaintDevicePrivate;

//! Paint device to draw on PoDoFo page.
class PoDoFoPaintDevice final : public QPaintDevice
{
public:
    PoDoFoPaintDevice(Render::PdfAuxData &pdfData);
    ~PoDoFoPaintDevice() override;

    void enableDrawing(bool on = true);

    QPaintEngine *paintEngine() const override;

protected:
    int metric(QPaintDevice::PaintDeviceMetric metric) const override;

private:
    Q_DISABLE_COPY(PoDoFoPaintDevice)

    QScopedPointer<PoDoFoPaintDevicePrivate> d;
}; // class PoDoFoPaintDevice

//
// PoDoFoPaintEngine
//

struct PoDoFoPaintEnginePrivate;

//! Paint engine to draw on PoDoFo page.
class PoDoFoPaintEngine final : public QPaintEngine
{
public:
    PoDoFoPaintEngine(Render::PdfAuxData &pdfData);
    ~PoDoFoPaintEngine() override;

    void enableDrawing(bool on = true);

    PoDoFo::PdfPainter *pdfPainter() const;

    bool begin(QPaintDevice *pdev) override;
    void drawEllipse(const QRectF &rect) override;
    void drawEllipse(const QRect &rect) override;
    void drawImage(const QRectF &rectangle,
                   const QImage &image,
                   const QRectF &sr,
                   Qt::ImageConversionFlags flags = Qt::AutoColor) override;
    void drawLines(const QLineF *lines,
                   int lineCount) override;
    void drawLines(const QLine *lines,
                   int lineCount) override;
    void drawPath(const QPainterPath &path) override;
    void drawPixmap(const QRectF &r,
                    const QPixmap &pm,
                    const QRectF &sr) override;
    void drawPoints(const QPointF *points,
                    int pointCount) override;
    void drawPoints(const QPoint *points,
                    int pointCount) override;
    void drawPolygon(const QPointF *points,
                     int pointCount,
                     QPaintEngine::PolygonDrawMode mode) override;
    void drawPolygon(const QPoint *points,
                     int pointCount,
                     QPaintEngine::PolygonDrawMode mode) override;
    void drawRects(const QRectF *rects,
                   int rectCount) override;
    void drawRects(const QRect *rects,
                   int rectCount) override;
    void drawTextItem(const QPointF &p,
                      const QTextItem &textItem) override;
    void drawTiledPixmap(const QRectF &rect,
                         const QPixmap &pixmap,
                         const QPointF &p) override;
    bool end() override;
    QPaintEngine::Type type() const override;
    void updateState(const QPaintEngineState &state) override;

private:
    double qXtoPoDoFo(double x);
    double qYtoPoDoFo(double y);
    double qWtoPoDoFo(double w);
    double qHtoPoDoFo(double h);
    PoDoFo::Rect qRectFtoPoDoFo(const QRectF &r);
    QPair<PoDoFo::PdfFont *,
          double>
    qFontToPoDoFo(const QFont &f);

private:
    Q_DISABLE_COPY(PoDoFoPaintEngine)

    QScopedPointer<PoDoFoPaintEnginePrivate> d;
}; // class PoDoFoPaintEngine

} /* namespace MdPdf */
