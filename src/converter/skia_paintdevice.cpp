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
#include <include/core/SkFont.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkPathBuilder.h>
#include <include/core/SkTypeface.h>
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>

// C++ include.
#include <stdexcept>

namespace MdPdf
{

//
// SkiaPaintDevicePrivate
//

struct SkiaPaintDevicePrivate {
    SkiaPaintDevicePrivate(SkiaPaintDevice *parent,
                           Render::PdfAuxData &pdfData)
        : m_q(parent)
        , m_engine(pdfData)
    {
    }

    //! Parent.
    SkiaPaintDevice *m_q = nullptr;
    //! Paint m_engine.
    SkiaPaintEngine m_engine;
}; // struct SkiaPaintDevicePrivate

//
// SkiaPaintDevice
//

SkiaPaintDevice::SkiaPaintDevice(Render::PdfAuxData &pdfData)
    : d(new SkiaPaintDevicePrivate(this,
                                   pdfData))
{
}

SkiaPaintDevice::~SkiaPaintDevice()
{
}

void SkiaPaintDevice::enableDrawing(bool on)
{
    d->m_engine.enableDrawing(on);
}

QPaintEngine *SkiaPaintDevice::paintEngine() const
{
    return &d->m_engine;
}

int SkiaPaintDevice::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    switch (metric) {
    case PdmWidth:
        return qRound(d->m_engine.pdfPainter() ? d->m_engine.pdfPainter()->getBaseLayerSize().width() : 100.0);

    case PdmHeight:
        return qRound(d->m_engine.pdfPainter() ? d->m_engine.pdfPainter()->getBaseLayerSize().height() : 100.0);

    case PdmWidthMM:
        return qRound(d->m_engine.pdfPainter() ? d->m_engine.pdfPainter()->getBaseLayerSize().width() / 72.0 * 25.4
                                               : 100.0 / 72.0 * 25.4);

    case PdmHeightMM:
        return qRound(d->m_engine.pdfPainter() ? d->m_engine.pdfPainter()->getBaseLayerSize().height() / 72.0 * 25.4
                                               : 100.0 / 72.0 * 25.4);

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
// SkiaPaintEnginePrivate
//

struct SkiaPaintEnginePrivate {
    SkiaPaintEnginePrivate(SkiaPaintEngine *parent,
                           Render::PdfAuxData &pdfData)
        : m_q(parent)
        , m_pdfData(pdfData)
    {
    }

    //! Parent.
    SkiaPaintEngine *m_q = nullptr;
    //! Is drawing enabled?
    bool m_isDrawing = false;
    //! Transformation.
    SkMatrix m_transform;
    //! PDF auxiliary data.
    Render::PdfAuxData &m_pdfData;
    //! Current SkPaint.
    SkPaint m_currentPaint;
    //! Current fill color.
    SkColor m_fillColor;
}; // struct SkiaPaintEnginePrivate

//
// SkiaPaintEngine
//

SkiaPaintEngine::SkiaPaintEngine(Render::PdfAuxData &pdfData)
    : QPaintEngine(QPaintEngine::AllFeatures)
    , d(new SkiaPaintEnginePrivate(this,
                                   pdfData))
{
}

SkiaPaintEngine::~SkiaPaintEngine()
{
}

void SkiaPaintEngine::enableDrawing(bool on)
{
    d->m_isDrawing = on;
}

SkCanvas *SkiaPaintEngine::pdfPainter() const
{
    return (*d->m_pdfData.m_pages)[d->m_pdfData.m_currentPainterIdx].m_canvas;
}

bool SkiaPaintEngine::begin(QPaintDevice *)
{
    return true;
}

void SkiaPaintEngine::drawEllipse(const QRectF &)
{
}

void SkiaPaintEngine::drawEllipse(const QRect &)
{
}

void SkiaPaintEngine::drawImage(const QRectF &,
                                const QImage &,
                                const QRectF &,
                                Qt::ImageConversionFlags)
{
}

void SkiaPaintEngine::drawLines(const QLineF *lines,
                                int lineCount)
{
    if (d->m_isDrawing) {
        pdfPainter()->save();
        pdfPainter()->setMatrix(d->m_transform);

        for (int i = 0; i < lineCount; ++i) {
            d->m_pdfData.drawLine(lines[i].x1(), lines[i].y1(), lines[i].x2(), lines[i].y2());
        }

        pdfPainter()->restore();
    }
}

void SkiaPaintEngine::drawLines(const QLine *lines,
                                int lineCount)
{
    if (d->m_isDrawing) {
        pdfPainter()->save();
        pdfPainter()->setMatrix(d->m_transform);

        for (int i = 0; i < lineCount; ++i) {
            d->m_pdfData.drawLine(lines[i].x1(), lines[i].y1(), lines[i].x2(), lines[i].y2());
        }

        pdfPainter()->restore();
    }
}

void SkiaPaintEngine::drawPath(const QPainterPath &path)
{
    if (d->m_isDrawing) {
        SkPathBuilder p;
        QPointF start, end;
        bool initialized = false;

        for (int i = 0; i < path.elementCount(); ++i) {
            const auto &e = path.elementAt(i);

            switch (e.type) {
            case QPainterPath::MoveToElement: {
                if (!initialized) {
                    start = {e.x, e.y};
                    initialized = true;
                }

                p.moveTo(e.x, e.y);
            } break;

            case QPainterPath::LineToElement: {
                p.lineTo(e.x, e.y);
                end = {e.x, e.y};
            } break;

            case QPainterPath::CurveToElement: {
                Q_ASSERT(path.elementAt(i + 1).type == QPainterPath::CurveToDataElement);
                Q_ASSERT(path.elementAt(i + 2).type == QPainterPath::CurveToDataElement);

                const auto pp1 = QPointF(path.elementAt(i + 1).x, path.elementAt(i + 1).y);
                const auto pp2 = QPointF(path.elementAt(i + 2).x, path.elementAt(i + 2).y);

                end = {e.x, e.y};

                p.cubicTo(e.x,
                          e.y,
                          pp1.x(),
                          pp1.y(),
                          pp2.x(),
                          pp2.y());

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

        pdfPainter()->save();
        pdfPainter()->setMatrix(d->m_transform);
        pdfPainter()->drawPath(p.detach(), paint);
        pdfPainter()->restore();
    }
}

void SkiaPaintEngine::drawPixmap(const QRectF &,
                                 const QPixmap &,
                                 const QRectF &)
{
}

void SkiaPaintEngine::drawPoints(const QPointF *,
                                 int)
{
}

void SkiaPaintEngine::drawPoints(const QPoint *,
                                 int)
{
}

void SkiaPaintEngine::drawPolygon(const QPointF *,
                                  int,
                                  QPaintEngine::PolygonDrawMode)
{
}

void SkiaPaintEngine::drawPolygon(const QPoint *,
                                  int,
                                  QPaintEngine::PolygonDrawMode)
{
}

inline SkRect qRectFtoSkia(const QRectF &r)
{
    return SkRect::MakeLTRB(static_cast<float>(r.left()),
                            static_cast<float>(r.top()),
                            static_cast<float>(r.right()),
                            static_cast<float>(r.bottom()));
}


void SkiaPaintEngine::drawRects(const QRectF *rects,
                                int rectCount)
{
    if (d->m_isDrawing) {
        SkPaint paint = d->m_currentPaint;
        paint.setStyle(SkPaint::kStroke_Style);

        pdfPainter()->save();
        pdfPainter()->setMatrix(d->m_transform);

        for (int i = 0; i < rectCount; ++i) {
            pdfPainter()->drawRect(qRectFtoSkia(rects[i]), paint);
        }

        pdfPainter()->restore();
    }
}

void SkiaPaintEngine::drawRects(const QRect *rects,
                                int rectCount)
{
    if (d->m_isDrawing) {
        SkPaint paint = d->m_currentPaint;
        paint.setStyle(SkPaint::kStroke_Style);

        pdfPainter()->save();
        pdfPainter()->setMatrix(d->m_transform);

        for (int i = 0; i < rectCount; ++i) {
            pdfPainter()->drawRect(qRectFtoSkia(rects[i]), paint);
        }

        pdfPainter()->restore();
    }
}

void SkiaPaintEngine::drawTextItem(const QPointF &p,
                                   const QTextItem &textItem)
{
    if (d->m_isDrawing) {
        const auto f = qFontToSkia(textItem.font());
        const auto utf8 = textItem.text().toUtf8();

        pdfPainter()->save();
        pdfPainter()->setMatrix(d->m_transform);
        d->m_pdfData.drawText(p.x(), p.y(), utf8.constData(), f.first, f.second, 1.0, false);
        pdfPainter()->restore();
    }
}

void SkiaPaintEngine::drawTiledPixmap(const QRectF &,
                                      const QPixmap &,
                                      const QPointF &)
{
}

bool SkiaPaintEngine::end()
{
    return true;
}

QPaintEngine::Type SkiaPaintEngine::type() const
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
    return SkColorSetRGB(c.red(), c.green(), c.blue());
}

QPair<SkFont,
      double>
SkiaPaintEngine::qFontToSkia(const QFont &f)
{
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

        return {SkFont(d->m_pdfData.m_fontMgr->makeFromFile(fileName.toLocal8Bit().constData()), f.pointSize()), f.pointSize()};
    } else {
        SkFontStyle::Slant slant = f.italic() ? SkFontStyle::kItalic_Slant : SkFontStyle::kUpright_Slant;
        int weight = f.bold() ? SkFontStyle::kBold_Weight : SkFontStyle::kNormal_Weight;

        SkFontStyle style(weight, SkFontStyle::kNormal_Width, slant);

        return {SkFont(d->m_pdfData.m_fontMgr->matchFamilyStyle(f.family().toLocal8Bit().data(), style), f.pointSize()), f.pointSize()};
    }
}

SkMatrix convertQTransformToSkMatrix(const QTransform& qTransform) {
    SkMatrix skMatrix;
    skMatrix.setAll(
            qTransform.m11(),  qTransform.m21(),  qTransform.m31(),
            qTransform.m12(),  qTransform.m22(),  qTransform.m32(),
            qTransform.m13(),  qTransform.m23(),  qTransform.m33()
        );

    return skMatrix;
}

void SkiaPaintEngine::updateState(const QPaintEngineState &state)
{
    if (!d->m_isDrawing) {
        return;
    }

    const auto st = state.state();

    if (st & QPaintEngine::DirtyPen) {
        const auto p = state.pen();

        d->m_currentPaint.setStrokeWidth(p.widthF());
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
        d->m_transform = convertQTransformToSkMatrix(state.transform());
    }
}

} /* namespace MdPdf */
