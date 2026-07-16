/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "skia_paintdevice.h"
#include "renderer.h"

// Qt include.
#include <QFile>
#include <QMap>
#include <QPainterPath>
#include <QTextItem>
#include <QTransform>

// MicroTeX include.
#include <fonts/font_info.h>

// Skia include.
#include <include/core/SkPathBuilder.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkTypeface.h>

#ifdef Q_OS_LINUX
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>
#endif

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
                          ? d->m_engine.pdfPainter()->getBaseLayerSize().width() / 72.0 * 1200.0
                          : 100);

    case PdmHeight:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->getBaseLayerSize().height() / 72.0 * 1200.0
                          : 100);

    case PdmWidthMM:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->getBaseLayerSize().width() / 72.0 * 25.4
                          : 100.0 / 1200.0 * 25.4);

    case PdmHeightMM:
        return qRound(d->m_engine.pdfPainter()
                          ? d->m_engine.pdfPainter()->getBaseLayerSize().height() / 72.0 * 25.4
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
    //! Current SkPaint.
    SkPaint m_currentPaint;
    //! Current fill color.
    SkColor m_fillColor;
}; // struct PoDoFoPaintEnginePrivate

//
// PoDoFoPaintEngine
//

PoDoFoPaintEngine::PoDoFoPaintEngine(Render::PdfAuxData &pdfData)
    : QPaintEngine(QPaintEngine::AllFeatures)
    , d(new PoDoFoPaintEnginePrivate(this,
                                     pdfData))
{
    pdfPainter()->save();
}

PoDoFoPaintEngine::~PoDoFoPaintEngine()
{
    pdfPainter()->restore();
}

void PoDoFoPaintEngine::enableDrawing(bool on)
{
    d->m_isDrawing = on;
}

SkCanvas *PoDoFoPaintEngine::pdfPainter() const
{
    return (*d->m_pdfData.m_pages)[d->m_pdfData.m_currentPainterIdx].m_canvas;
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

double PoDoFoPaintEngine::qXtoSkia(double x)
{
    return x / paintDevice()->physicalDpiX() * 72.0;
}

double PoDoFoPaintEngine::qYtoSkia(double y)
{
    return (paintDevice()->height() - y) / paintDevice()->physicalDpiY() * 72.0;
}

void PoDoFoPaintEngine::drawLines(const QLineF *lines,
                                  int lineCount)
{
    if (d->m_isDrawing) {
        for (int i = 0; i < lineCount; ++i) {
            const auto l = d->m_transform.map(lines[i]);

            d->m_pdfData.drawLine(qXtoSkia(l.x1()), qYtoSkia(l.y1()), qXtoSkia(l.x2()), qYtoSkia(l.y2()));
        }
    }
}

void PoDoFoPaintEngine::drawLines(const QLine *lines,
                                  int lineCount)
{
    if (d->m_isDrawing) {
        for (int i = 0; i < lineCount; ++i) {
            const auto l = d->m_transform.map(lines[i]);

            d->m_pdfData.drawLine(qXtoSkia(l.x1()), qYtoSkia(l.y1()), qXtoSkia(l.x2()), qYtoSkia(l.y2()));
        }
    }
}

void PoDoFoPaintEngine::drawPath(const QPainterPath &path)
{
    if (d->m_isDrawing) {
        SkPathBuilder p;
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

                p.moveTo(qXtoSkia(pp.x()), qYtoSkia(pp.y()));
            } break;

            case QPainterPath::LineToElement: {
                const auto pp = d->m_transform.map(QPointF(e.x, e.y));

                p.lineTo(qXtoSkia(pp.x()), qYtoSkia(pp.y()));
                end = {pp.x(), pp.y()};
            } break;

            case QPainterPath::CurveToElement: {
                Q_ASSERT(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement);
                Q_ASSERT(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement);

                const auto pp = d->m_transform.map(QPointF(e.x, e.y));
                const auto pp1 = d->m_transform.map(QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y));
                const auto pp2 = d->m_transform.map(QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y));

                end = {pp.x(), pp.y()};

                p.cubicTo(qXtoSkia(pp.x()),
                                   qYtoSkia(pp.y()),
                                   qXtoSkia(pp1.x()),
                                   qYtoSkia(pp1.y()),
                                   qXtoSkia(pp2.x()),
                                   qYtoSkia(pp2.y()));

                i += 2;
            } break;

            default:
                break;
            }
        }

        SkPaint paint = d->m_currentPaint;
        paint.setStyle(start == end ? SkPaint::kFill_Style : SkPaint::kStroke_Style);

        if (paint.getStyle() == SkPaint::kFill_Style) {
            paint.setColor(d->m_fillColor);
        }

        pdfPainter()->drawPath(p.detach(), paint);
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

double PoDoFoPaintEngine::qWtoSkia(double w)
{
    return w / paintDevice()->physicalDpiX() * 72.0;
}

double PoDoFoPaintEngine::qHtoSkia(double h)
{
    return h / paintDevice()->physicalDpiY() * 72.0;
}

SkRect PoDoFoPaintEngine::qRectFtoSkia(const QRectF &r)
{
    return SkRect::MakeLTRB(
            static_cast<float>(r.left()),
            static_cast<float>(r.top()),
            static_cast<float>(r.right()),
            static_cast<float>(r.bottom())
        );
}

void PoDoFoPaintEngine::drawRects(const QRectF *rects,
                                  int rectCount)
{
    if (d->m_isDrawing) {
        SkPaint paint = d->m_currentPaint;
        paint.setStyle(SkPaint::kStroke_Style);

        for (int i = 0; i < rectCount; ++i) {
            pdfPainter()->drawRect(qRectFtoSkia(d->m_transform.mapRect(rects[i])), paint);
        }
    }
}

void PoDoFoPaintEngine::drawRects(const QRect *rects,
                                  int rectCount)
{
    if (d->m_isDrawing) {
        SkPaint paint = d->m_currentPaint;
        paint.setStyle(SkPaint::kStroke_Style);

        for (int i = 0; i < rectCount; ++i) {
            pdfPainter()->drawRect(qRectFtoSkia(d->m_transform.mapRect(rects[i])), paint);
        }
    }
}

void PoDoFoPaintEngine::drawTextItem(const QPointF &p,
                                     const QTextItem &textItem)
{
    if (d->m_isDrawing) {
        const auto f = qFontToSkia(textItem.font());
        const auto pp = d->m_transform.map(p);
        const auto utf8 = textItem.text().toUtf8();

        d->m_pdfData.drawText(qXtoSkia(pp.x()), qYtoSkia(pp.y()), utf8.constData(), f.first, f.second, 1.0, false);
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

inline SkPaint::Cap capStyle(Qt::PenCapStyle s)
{
    switch (s) {
    case Qt::SquareCap:
        return SkPaint::kSquare_Cap;

    case Qt::FlatCap:
        return SkPaint::kButt_Cap;

    case Qt::RoundCap:
    default:
        return SkPaint::kRound_Cap;
    }
}

inline SkPaint::Join joinStyle(Qt::PenJoinStyle s)
{
    switch (s) {
    case Qt::MiterJoin:
        return SkPaint::kMiter_Join;

    case Qt::BevelJoin:
        return SkPaint::kBevel_Join;

    case Qt::RoundJoin:
    default:
        return SkPaint::kRound_Join;
    }
}

inline SkColor color(const QColor &c)
{
    return SkColorSetRGB(c.redF(), c.greenF(), c.blueF());
}

inline double scaleOfTransform(const QTransform &t)
{
    const auto bv = t.m21();
    const auto dv = t.m22();

    return std::sqrt(bv * bv + dv * dv);
}

QPair<SkFont,
      double>
PoDoFoPaintEngine::qFontToSkia(const QFont &f)
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

#ifdef Q_OS_LINUX
        auto scanner = SkFontScanner_Make_FreeType();
        auto fontMgr = SkFontMgr_New_FontConfig(nullptr, std::move(scanner));
#endif

        return {SkFont(fontMgr->makeFromFile(fileName.toLocal8Bit().constData()), size), size};
    } else {        
#ifdef Q_OS_LINUX
        auto scanner = SkFontScanner_Make_FreeType();
        auto fontMgr = SkFontMgr_New_FontConfig(nullptr, std::move(scanner));
#endif

        SkFontStyle::Slant slant = f.italic() ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant;
        int weight = f.bold() ? SkFontStyle::kBold_Weight : SkFontStyle::kNormal_Weight;

        SkFontStyle style(weight, SkFontStyle::kNormal_Width, slant);

        return {SkFont(fontMgr->matchFamilyStyle(f.family().toLocal8Bit().data(), style), size), size};
    }
}

void PoDoFoPaintEngine::updateState(const QPaintEngineState &state)
{
    if (!d->m_isDrawing) {
        return;
    }

    const auto st = state.state();

    if (st & QPaintEngine::DirtyPen) {
        const auto p = state.pen();

        // This code is for MicroTeX, the author do things not right with pen width.
        const auto scale = scaleOfTransform(d->m_transform);

        d->m_currentPaint.setStrokeWidth(p.widthF() * scale / paintDevice()->physicalDpiX() * 72.0);
        d->m_currentPaint.setStrokeCap(capStyle(p.capStyle()));
        d->m_currentPaint.setStrokeJoin(joinStyle(p.joinStyle()));
        d->m_currentPaint.setColor(color(p.color()));
        d->m_currentPaint.setStrokeMiter(p.miterLimit());
        d->m_fillColor = color(p.brush().color());
    }

    if (st & QPaintEngine::DirtyBrush) {
        d->m_fillColor = color(state.brush().color());
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
