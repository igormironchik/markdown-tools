/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "podofo_paintdevice.hpp"

// Qt include.
#include <QPainterPath>
#include <QTextItem>
#include <QTransform>
#include <QFile>

#include <QDebug>

// MicroTeX include.
#include <fonts/font_info.h>

// C++ include.
#include <stdexcept>

namespace MdPdf
{

//
// PoDoFoPaintDevicePrivate
//

struct PoDoFoPaintDevicePrivate {
    PoDoFoPaintDevicePrivate(PoDoFoPaintDevice *parent)
        : m_q(parent)
    {
    }

    //! Parent.
    PoDoFoPaintDevice *m_q = nullptr;
    //! Paint m_engine.
    PoDoFoPaintEngine m_engine;
}; // struct PoDoFoPaintDevicePrivate

//
// PoDoFoPaintDevice
//

PoDoFoPaintDevice::PoDoFoPaintDevice()
    : d(new PoDoFoPaintDevicePrivate(this))
{
}

PoDoFoPaintDevice::~PoDoFoPaintDevice()
{
}

void PoDoFoPaintDevice::setPdfPainter(PoDoFo::PdfPainter &p, PoDoFo::PdfDocument &doc)
{
    d->m_engine.setPdfPainter(p, doc);
}

QPaintEngine *PoDoFoPaintDevice::paintEngine() const
{
    return &d->m_engine;
}

int PoDoFoPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch (metric) {
    case PdmWidth:
        return qRound(d->m_engine.pdfPainter() ?
                          d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().Width / 72.0 * 1200.0 : 100);

    case PdmHeight:
        return qRound(d->m_engine.pdfPainter() ?
                          d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().Height / 72.0 * 1200.0 : 100);

    case PdmWidthMM:
        return qRound(d->m_engine.pdfPainter() ?
                          d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().Width / 72.0 * 25.4 : 100.0 / 1200.0 * 25.4);

    case PdmHeightMM:
        return qRound(d->m_engine.pdfPainter() ?
                          d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().Height / 72.0 * 25.4 : 100.0 / 1200.0 * 25.4);

    case PdmNumColors:
        return INT_MAX;

    case PdmDepth:
        return 32;

    case PdmDpiX:
    case PdmDpiY:
    case PdmPhysicalDpiX:
    case PdmPhysicalDpiY:
        return 1200;

    case PdmDevicePixelRatio:
        return 1;

    case PdmDevicePixelRatioScaled:
        return 1 * QPaintDevice::devicePixelRatioFScale();
    }

    return -1;
}

//
// PoDoFoPaintEnginePrivate
//

struct PoDoFoPaintEnginePrivate {
    PoDoFoPaintEnginePrivate(PoDoFoPaintEngine *parent)
        : m_q(parent)
    {
    }

    //! Parent.
    PoDoFoPaintEngine *m_q = nullptr;
    //! Pdf m_painter.
    PoDoFo::PdfPainter *m_painter = nullptr;
    //! Pdf document.
    PoDoFo::PdfDocument *m_doc = nullptr;
    //! Transformation.
    QTransform m_transform;
}; // struct PoDoFoPaintEnginePrivate

//
// PoDoFoPaintEngine
//

PoDoFoPaintEngine::PoDoFoPaintEngine()
    : QPaintEngine(QPaintEngine::AllFeatures)
    , d(new PoDoFoPaintEnginePrivate(this))
{
}

PoDoFoPaintEngine::~PoDoFoPaintEngine()
{
}

void PoDoFoPaintEngine::setPdfPainter(PoDoFo::PdfPainter &p, PoDoFo::PdfDocument &doc)
{
    d->m_painter = &p;
    d->m_doc = &doc;
}

PoDoFo::PdfPainter *PoDoFoPaintEngine::pdfPainter() const
{
    return d->m_painter;
}

bool PoDoFoPaintEngine::begin(QPaintDevice *)
{
    return true;
}

void PoDoFoPaintEngine::drawEllipse(const QRectF &)
{
}

void PoDoFoPaintEngine::drawEllipse(const QRect &)
{
}

void PoDoFoPaintEngine::drawImage(const QRectF &, const QImage &, const QRectF &, Qt::ImageConversionFlags)
{
    qDebug() << "draw image";
}

double PoDoFoPaintEngine::qXtoPoDoFo(double x)
{
    return x / paintDevice()->physicalDpiX() * 72.0;
}

double PoDoFoPaintEngine::qYtoPoDoFo(double y)
{
    return (paintDevice()->height() - y) / paintDevice()->physicalDpiY() * 72.0;
}

void PoDoFoPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    if (d->m_painter) {
        for (int i = 0; i < lineCount; ++i) {
            const auto l = d->m_transform.map(lines[i]);

            d->m_painter->DrawLine(qXtoPoDoFo(l.x1()), qYtoPoDoFo(l.y1()), qXtoPoDoFo(l.x2()), qYtoPoDoFo(l.y2()));
        }
    }
}

void PoDoFoPaintEngine::drawLines(const QLine *lines, int lineCount)
{
    if (d->m_painter) {
        for (int i = 0; i < lineCount; ++i) {
            const auto l = d->m_transform.map(lines[i]);

            d->m_painter->DrawLine(qXtoPoDoFo(l.x1()), qYtoPoDoFo(l.y1()), qXtoPoDoFo(l.x2()), qYtoPoDoFo(l.y2()));
        }
    }
}

void PoDoFoPaintEngine::drawPath(const QPainterPath &path)
{
    if (d->m_painter) {
        PoDoFo::PdfPainterPath p;
        QPointF start, end;
        bool initialized = false;

        for (int i = 0; i < path.elementCount(); ++i) {
            const auto &e = path.elementAt(i);

            switch (e.type) {
            case QPainterPath::MoveToElement: {
                const auto pp = d->m_transform.map(QPointF(e.x, e.y));

                if (!initialized) {
                    start = {pp.x(), pp.y()};
                    initialized = true;
                }

                p.MoveTo(qXtoPoDoFo(pp.x()), qYtoPoDoFo(pp.y()));
            } break;

            case QPainterPath::LineToElement: {
                const auto pp = d->m_transform.map(QPointF(e.x, e.y));

                p.AddLineTo(qXtoPoDoFo(pp.x()), qYtoPoDoFo(pp.y()));
                end = {pp.x(), pp.y()};
            } break;

            case QPainterPath::CurveToElement: {
                Q_ASSERT(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement);
                Q_ASSERT(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement);

                const auto pp = d->m_transform.map(QPointF(e.x, e.y));
                const auto pp1 = d->m_transform.map(QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y));
                const auto pp2 = d->m_transform.map(QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y));

                end = {pp.x(), pp.y()};

                p.AddCubicBezierTo(qXtoPoDoFo(pp.x()), qYtoPoDoFo(pp.y()), qXtoPoDoFo(pp1.x()),
                                   qYtoPoDoFo(pp1.y()), qXtoPoDoFo(pp2.x()), qYtoPoDoFo(pp2.y()));

                i += 2;
            } break;

            default:
                break;
            }
        }

        d->m_painter->DrawPath(p, start == end ? PoDoFo::PdfPathDrawMode::StrokeFill : PoDoFo::PdfPathDrawMode::Stroke);
    }
}

void PoDoFoPaintEngine::drawPixmap(const QRectF &, const QPixmap &, const QRectF &)
{
}

void PoDoFoPaintEngine::drawPoints(const QPointF *, int)
{
}

void PoDoFoPaintEngine::drawPoints(const QPoint *, int)
{
}

void PoDoFoPaintEngine::drawPolygon(const QPointF *, int, QPaintEngine::PolygonDrawMode)
{
    qDebug() << "draw polygon";
}

void PoDoFoPaintEngine::drawPolygon(const QPoint *, int, QPaintEngine::PolygonDrawMode)
{
    qDebug() << "draw polygon";
}

double PoDoFoPaintEngine::qWtoPoDoFo(double w)
{
    return w / paintDevice()->physicalDpiX() * 72.0;
}

double PoDoFoPaintEngine::qHtoPoDoFo(double h)
{
    return h / paintDevice()->physicalDpiY() * 72.0;
}

PoDoFo::Rect PoDoFoPaintEngine::qRectFtoPoDoFo(const QRectF &r)
{
    return PoDoFo::Rect(qXtoPoDoFo(r.x()), qYtoPoDoFo(r.y() + r.height()), qWtoPoDoFo(r.width()), qHtoPoDoFo(r.height()));
}

void PoDoFoPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    if (d->m_painter) {
        for (int i = 0; i < rectCount; ++i) {
            d->m_painter->DrawRectangle(qRectFtoPoDoFo(d->m_transform.mapRect(rects[i])));
        }
    }
}

void PoDoFoPaintEngine::drawRects(const QRect *rects, int rectCount)
{
    if (d->m_painter) {
        for (int i = 0; i < rectCount; ++i) {
            d->m_painter->DrawRectangle(qRectFtoPoDoFo(d->m_transform.mapRect(rects[i].toRectF())));
        }
    }
}

void PoDoFoPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    if (d->m_painter) {
        const auto f = qFontToPoDoFo(textItem.font());
        const auto pp = d->m_transform.map(p);

        qDebug() << textItem.font().family() << textItem.text();

        d->m_painter->TextObject.Begin();
        d->m_painter->TextObject.MoveTo(qXtoPoDoFo(pp.x()), qYtoPoDoFo(pp.y()));
        d->m_painter->TextState.SetFont(*f.first, f.second);
        d->m_painter->TextState.SetFontScale(1.0);
        const auto utf8 = textItem.text().toUtf8();
        d->m_painter->TextObject.AddText(utf8.data());
        d->m_painter->TextObject.End();
    }
}

void PoDoFoPaintEngine::drawTiledPixmap(const QRectF &, const QPixmap &, const QPointF &)
{
}

bool PoDoFoPaintEngine::end()
{
    return true;
}

QPaintEngine::Type PoDoFoPaintEngine::type() const
{
    return QPaintEngine::Pdf;
}

inline PoDoFo::PdfLineCapStyle capStyle(Qt::PenCapStyle s)
{
    switch (s) {
    case Qt::SquareCap:
        return PoDoFo::PdfLineCapStyle::Square;

    case Qt::FlatCap:
        return PoDoFo::PdfLineCapStyle::Butt;

    case Qt::RoundCap:
    default:
        return PoDoFo::PdfLineCapStyle::Round;
    }
}

inline PoDoFo::PdfLineJoinStyle joinStyle(Qt::PenJoinStyle s)
{
    switch (s) {
    case Qt::MiterJoin:
        return PoDoFo::PdfLineJoinStyle::Miter;

    case Qt::BevelJoin:
        return PoDoFo::PdfLineJoinStyle::Bevel;

    case Qt::RoundJoin:
    default:
        return PoDoFo::PdfLineJoinStyle::Round;
    }
}

inline PoDoFo::PdfColor color(const QColor &c)
{
    return PoDoFo::PdfColor(c.redF(), c.greenF(), c.blueF());
}

QPair<PoDoFo::PdfFont *, double> PoDoFoPaintEngine::qFontToPoDoFo(const QFont &f)
{
    //const double size = (f.pointSizeF() > 0.0 ? f.pointSizeF() : f.pixelSize() / paintDevice()->physicalDpiY() * 72.0);

    // This code is for MicroTeX, the author do things not right with font sizes.
    const auto bv = d->m_transform.m21();
    const auto dv = d->m_transform.m22();
    const auto scale = std::sqrt(bv * bv + dv * dv);

    const double size = f.pointSize() * scale / paintDevice()->physicalDpiY() * 72.0;

    auto id = tex::FontInfo::__id(f.family().toStdString());

    if (id != -1) {
        const auto path = QStringLiteral(":/") + QString(tex::FontInfo::__get(id)->getPath().c_str());
        QFile fontFile(path);

        if (fontFile.open(QIODevice::ReadOnly)) {
            auto content = fontFile.readAll();
            fontFile.close();

            PoDoFo::PdfFontCreateParams params;
            params.Flags = PoDoFo::PdfFontCreateFlags::DontSubset;

            auto &font = d->m_doc->GetFonts().GetOrCreateFontFromBuffer(
                        PoDoFo::bufferview(content.data(), content.size()), params);

            return {&font, size};
        } else {
            throw std::runtime_error("Unable to load font from file: \":/" +
                                     tex::FontInfo::__get(id)->getPath() + "\".");
        }
    } else {
        PoDoFo::PdfFontSearchParams params;
        params.Style = PoDoFo::PdfFontStyle::Regular;
        if (f.bold()) {
            params.Style.value() |= PoDoFo::PdfFontStyle::Bold;
        }
        if (f.italic()) {
            params.Style.value() |= PoDoFo::PdfFontStyle::Italic;
        }

        auto *font = d->m_doc->GetFonts().SearchFont(f.family().toLocal8Bit().data(), params);

        return {font, size};
    }
}

void PoDoFoPaintEngine::updateState(const QPaintEngineState &state)
{
    if (!d->m_painter) {
        return;
    }

    const auto st = state.state();

    if (st & QPaintEngine::DirtyPen) {
        const auto p = state.pen();

        d->m_painter->GraphicsState.SetLineWidth(p.widthF() / paintDevice()->physicalDpiX() * 72.0);
        d->m_painter->GraphicsState.SetLineCapStyle(capStyle(p.capStyle()));
        d->m_painter->GraphicsState.SetLineJoinStyle(joinStyle(p.joinStyle()));
        d->m_painter->GraphicsState.SetNonStrokingColor(color(p.brush().color()));
        d->m_painter->GraphicsState.SetStrokingColor(color(p.color()));
        d->m_painter->GraphicsState.SetMiterLevel(p.miterLimit());
    }

    if (st & QPaintEngine::DirtyBrush) {
        d->m_painter->GraphicsState.SetNonStrokingColor(color(state.brush().color()));
    }

    if (st & QPaintEngine::DirtyFont) {
        const auto f = qFontToPoDoFo(state.font());

        d->m_painter->TextState.SetFont(*f.first, f.second);
    }

    if (st && QPaintEngine::DirtyTransform) {
        d->m_transform = state.transform();
    }
}

} /* namespace MdPdf */
