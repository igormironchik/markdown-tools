/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "podofo_paintdevice.h"
#include "renderer.h"

// Qt include.
#include <QFile>
#include <QMap>
#include <QPainterPath>
#include <QTextItem>
#include <QTransform>

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
    PoDoFoPaintDevicePrivate(PoDoFoPaintDevice *parent,
                             Render::PdfAuxData &pdfData)
        : m_q(parent)
        , m_engine(pdfData)
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

PoDoFoPaintDevice::PoDoFoPaintDevice(Render::PdfAuxData &pdfData)
    : d(new PoDoFoPaintDevicePrivate(this,
                                     pdfData))
{
}

PoDoFoPaintDevice::~PoDoFoPaintDevice()
{
}

void PoDoFoPaintDevice::enableDrawing(bool on)
{
    d->m_engine.enableDrawing(on);
}

QPaintEngine *PoDoFoPaintDevice::paintEngine() const
{
    return &d->m_engine;
}

int PoDoFoPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch (metric) {
    case PdmWidth:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().GetWidth() / 72.0 * 1200.0
                          : 100);

    case PdmHeight:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().GetHeight() / 72.0 * 1200.0
                          : 100);

    case PdmWidthMM:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().GetWidth() / 72.0 * 25.4
                          : 100.0 / 1200.0 * 25.4);

    case PdmHeightMM:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->GetCanvas()->GetRectRaw().GetHeight() / 72.0 * 25.4
                          : 100.0 / 1200.0 * 25.4);

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

    default:
        return -1;
    }
}

//
// PoDoFoPaintEnginePrivate
//

struct PoDoFoPaintEnginePrivate {
    PoDoFoPaintEnginePrivate(PoDoFoPaintEngine *parent,
                             Render::PdfAuxData &pdfData)
        : m_q(parent)
        , m_pdfData(pdfData)
    {
    }

    //! Parent.
    PoDoFoPaintEngine *m_q = nullptr;
    //! Is drawing enabled?
    bool m_isDrawing = false;
    //! Transformation.
    QTransform m_transform;
    //! PDF auxiliary data.
    Render::PdfAuxData &m_pdfData;
}; // struct PoDoFoPaintEnginePrivate

//
// PoDoFoPaintEngine
//

PoDoFoPaintEngine::PoDoFoPaintEngine(Render::PdfAuxData &pdfData)
    : QPaintEngine(QPaintEngine::AllFeatures)
    , d(new PoDoFoPaintEnginePrivate(this,
                                     pdfData))
{
    pdfPainter()->Save();
}

PoDoFoPaintEngine::~PoDoFoPaintEngine()
{
    pdfPainter()->Restore();
}

void PoDoFoPaintEngine::enableDrawing(bool on)
{
    d->m_isDrawing = on;
}

PoDoFo::PdfPainter *PoDoFoPaintEngine::pdfPainter() const
{
    return (*d->m_pdfData.m_painters)[d->m_pdfData.m_currentPainterIdx].get();
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

void PoDoFoPaintEngine::drawImage(const QRectF &,
                                  const QImage &,
                                  const QRectF &,
                                  Qt::ImageConversionFlags)
{
}

double PoDoFoPaintEngine::qXtoPoDoFo(double x)
{
    return x / paintDevice()->physicalDpiX() * 72.0;
}

double PoDoFoPaintEngine::qYtoPoDoFo(double y)
{
    return (paintDevice()->height() - y) / paintDevice()->physicalDpiY() * 72.0;
}

void PoDoFoPaintEngine::drawLines(const QLineF *lines,
                                  int lineCount)
{
    if (d->m_isDrawing) {
        for (int i = 0; i < lineCount; ++i) {
            const auto l = d->m_transform.map(lines[i]);

            d->m_pdfData.drawLine(qXtoPoDoFo(l.x1()), qYtoPoDoFo(l.y1()), qXtoPoDoFo(l.x2()), qYtoPoDoFo(l.y2()));
        }
    }
}

void PoDoFoPaintEngine::drawLines(const QLine *lines,
                                  int lineCount)
{
    if (d->m_isDrawing) {
        for (int i = 0; i < lineCount; ++i) {
            const auto l = d->m_transform.map(lines[i]);

            d->m_pdfData.drawLine(qXtoPoDoFo(l.x1()), qYtoPoDoFo(l.y1()), qXtoPoDoFo(l.x2()), qYtoPoDoFo(l.y2()));
        }
    }
}

void PoDoFoPaintEngine::drawPath(const QPainterPath &path)
{
    if (d->m_isDrawing) {
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

                p.AddCubicBezierTo(qXtoPoDoFo(pp.x()),
                                   qYtoPoDoFo(pp.y()),
                                   qXtoPoDoFo(pp1.x()),
                                   qYtoPoDoFo(pp1.y()),
                                   qXtoPoDoFo(pp2.x()),
                                   qYtoPoDoFo(pp2.y()));

                i += 2;
            } break;

            default:
                break;
            }
        }

        pdfPainter()->DrawPath(p, start == end ? PoDoFo::PdfPathDrawMode::StrokeFill : PoDoFo::PdfPathDrawMode::Stroke);
    }
}

void PoDoFoPaintEngine::drawPixmap(const QRectF &,
                                   const QPixmap &,
                                   const QRectF &)
{
}

void PoDoFoPaintEngine::drawPoints(const QPointF *,
                                   int)
{
}

void PoDoFoPaintEngine::drawPoints(const QPoint *,
                                   int)
{
}

void PoDoFoPaintEngine::drawPolygon(const QPointF *,
                                    int,
                                    QPaintEngine::PolygonDrawMode)
{
}

void PoDoFoPaintEngine::drawPolygon(const QPoint *,
                                    int,
                                    QPaintEngine::PolygonDrawMode)
{
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
    return PoDoFo::Rect(qXtoPoDoFo(r.x()),
                        qYtoPoDoFo(r.y() + r.height()),
                        qWtoPoDoFo(r.width()),
                        qHtoPoDoFo(r.height()));
}

void PoDoFoPaintEngine::drawRects(const QRectF *rects,
                                  int rectCount)
{
    if (d->m_isDrawing) {
        for (int i = 0; i < rectCount; ++i) {
            pdfPainter()->DrawRectangle(qRectFtoPoDoFo(d->m_transform.mapRect(rects[i])));
        }
    }
}

void PoDoFoPaintEngine::drawRects(const QRect *rects,
                                  int rectCount)
{
    if (d->m_isDrawing) {
        for (int i = 0; i < rectCount; ++i) {
            pdfPainter()->DrawRectangle(qRectFtoPoDoFo(d->m_transform.mapRect(rects[i].toRectF())));
        }
    }
}

void PoDoFoPaintEngine::drawTextItem(const QPointF &p,
                                     const QTextItem &textItem)
{
    if (d->m_isDrawing) {
        const auto f = qFontToPoDoFo(textItem.font());
        const auto pp = d->m_transform.map(p);
        const auto utf8 = textItem.text().toUtf8();

        d->m_pdfData.drawText(qXtoPoDoFo(pp.x()), qYtoPoDoFo(pp.y()), utf8.constData(), f.first, f.second, 1.0, false);
    }
}

void PoDoFoPaintEngine::drawTiledPixmap(const QRectF &,
                                        const QPixmap &,
                                        const QPointF &)
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

inline double scaleOfTransform(const QTransform &t)
{
    const auto bv = t.m21();
    const auto dv = t.m22();

    return std::sqrt(bv * bv + dv * dv);
}

QPair<PoDoFo::PdfFont *,
      double>
PoDoFoPaintEngine::qFontToPoDoFo(const QFont &f)
{
    // const double size = (f.pointSizeF() > 0.0 ? f.pointSizeF() : f.pixelSize() / paintDevice()->physicalDpiY()
    // * 72.0);

    // This code is for MicroTeX, the author do things not right with font sizes.
    const auto scale = scaleOfTransform(d->m_transform);

    const double size = f.pointSize() * scale / paintDevice()->physicalDpiY() * 72.0;

    auto id = tex::FontInfo::__id(f.family().toStdString());

    if (id != -1) {
        const auto path = QStringLiteral(":/") + QString(tex::FontInfo::__get(id)->getPath().c_str());

        if (!d->m_pdfData.m_fontsCache.contains(path)) {
            QFile fontFile(path);

            if (fontFile.open(QIODevice::ReadOnly)) {
                const auto content = fontFile.readAll();
                fontFile.close();

                auto tmp = QSharedPointer<QTemporaryFile>(new QTemporaryFile);

                if (tmp->open()) {
                    tmp->write(content);
                    tmp->close();

                    d->m_pdfData.m_fontsCache.insert(path, tmp);

                    d->m_pdfData.m_doc->GetFonts().GetOrCreateFont(tmp->fileName().toLocal8Bit().constData());
                } else {
                    throw std::runtime_error("Unable to load font from file: \":/"
                                             + tex::FontInfo::__get(id)->getPath()
                                             + "\". Failed to create temporary file.");
                }
            } else {
                throw std::runtime_error(
                    "Unable to load font from file: \":/" + tex::FontInfo::__get(id)->getPath() + "\".");
            }
        }

        const auto fileName = d->m_pdfData.m_fontsCache[path]->fileName();

        auto &font = d->m_pdfData.m_doc->GetFonts().GetOrCreateFont(fileName.toLocal8Bit().constData());

        return {&font, size};
    } else {
        PoDoFo::PdfFontSearchParams params;
        PoDoFo::PdfFontStyle style;
        params.Style = PoDoFo::PdfFontStyle::Regular;
        if (f.bold()) {
            style |= PoDoFo::PdfFontStyle::Bold;
        }
        if (f.italic()) {
            style |= PoDoFo::PdfFontStyle::Italic;
        }

        params.Style = style;

        auto *font = d->m_pdfData.m_doc->GetFonts().SearchFont(f.family().toLocal8Bit().data(), params);

        return {font, size};
    }
}

void PoDoFoPaintEngine::updateState(const QPaintEngineState &state)
{
    if (!d->m_isDrawing) {
        return;
    }

    const auto st = state.state();

    auto painter = pdfPainter();

    if (st & QPaintEngine::DirtyPen) {
        const auto p = state.pen();

        // This code is for MicroTeX, the author do things not right with pen width.
        const auto scale = scaleOfTransform(d->m_transform);

        painter->GraphicsState.SetLineWidth(p.widthF() * scale / paintDevice()->physicalDpiX() * 72.0);
        painter->GraphicsState.SetLineCapStyle(capStyle(p.capStyle()));
        painter->GraphicsState.SetLineJoinStyle(joinStyle(p.joinStyle()));
        painter->GraphicsState.SetNonStrokingColor(color(p.brush().color()));
        painter->GraphicsState.SetStrokingColor(color(p.color()));
        painter->GraphicsState.SetMiterLevel(p.miterLimit());
    }

    if (st & QPaintEngine::DirtyBrush) {
        painter->GraphicsState.SetNonStrokingColor(color(state.brush().color()));
    }

    // if (st & QPaintEngine::DirtyFont) {
    //     const auto f = qFontToPoDoFo(state.font());

    //     d->m_painter->TextState.SetFont(*f.first, f.second);
    // }

    if (st && QPaintEngine::DirtyTransform) {
        d->m_transform = state.transform();
    }
}

} /* namespace MdPdf */
