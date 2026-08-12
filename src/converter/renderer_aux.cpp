/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// converter include.
#include "renderer.h"

// Skia include.
#include <include/core/SkFontMetrics.h>
#include <include/core/SkPicture.h>
#include <include/core/SkStream.h>
#include <include/docs/SkPDFDocument.h>
#include <include/docs/SkPDFJpegHelpers.h>

// Qt include.
#include <QThread>

#ifdef MD_PDF_TESTING
#include <QtTest/QtTest>
#endif

// C++ include.
#include <fstream>

// zlib include.
#include <zlib.h>

QByteArray qt_inflateSvgzDataFrom(QIODevice *device,
                                  bool doCheckContent)
{
    Q_UNUSED(doCheckContent)

    if (!device) {
        return QByteArray();
    }

    if (!device->isOpen()) {
        device->open(QIODevice::ReadOnly);
    }

    Q_ASSERT(device->isOpen() && device->isReadable());

    static const int CHUNK_SIZE = 4096;
    int zlibResult = Z_OK;

    QByteArray source;
    QByteArray destination;

    // Initialize zlib stream struct
    z_stream zlibStream;
    zlibStream.next_in = Z_NULL;
    zlibStream.avail_in = 0;
    zlibStream.avail_out = 0;
    zlibStream.zalloc = Z_NULL;
    zlibStream.zfree = Z_NULL;
    zlibStream.opaque = Z_NULL;

    // Adding 16 to the window size gives us gzip decoding
    if (inflateInit2(&zlibStream, MAX_WBITS + 16) != Z_OK) {
        return QByteArray();
    }

    bool stillMoreWorkToDo = true;
    while (stillMoreWorkToDo) {
        if (!zlibStream.avail_in) {
            source = device->read(CHUNK_SIZE);

            if (source.isEmpty())
                break;

            zlibStream.avail_in = source.size();
            zlibStream.next_in = reinterpret_cast<Bytef *>(source.data());
        }

        do {
            // Prepare the destination buffer
            int oldSize = destination.size();
            if (oldSize > INT_MAX - CHUNK_SIZE) {
                inflateEnd(&zlibStream);
                return QByteArray();
            }

            destination.resize(oldSize + CHUNK_SIZE);
            zlibStream.next_out = reinterpret_cast<Bytef *>(destination.data() + oldSize - zlibStream.avail_out);
            zlibStream.avail_out += CHUNK_SIZE;

            zlibResult = inflate(&zlibStream, Z_NO_FLUSH);
            switch (zlibResult) {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_STREAM_ERROR:
            case Z_MEM_ERROR: {
                inflateEnd(&zlibStream);
                return QByteArray();
            }
            }

            // If the output buffer still has more room after calling inflate
            // it means we have to provide more data, so exit the loop here
        } while (!zlibStream.avail_out);

        if (zlibResult == Z_STREAM_END) {
            // Make sure there are no more members to process before exiting
            if (!(zlibStream.avail_in && inflateReset(&zlibStream) == Z_OK))
                stillMoreWorkToDo = false;
        }
    }

    // Chop off trailing space in the buffer
    destination.chop(zlibStream.avail_out);

    inflateEnd(&zlibStream);
    return destination;
}

namespace MdPdf
{

namespace Render
{

//
// Utf8String
//

Utf8String::Utf8String(const QByteArray &a)
    : data(a)
{
}

Utf8String::Utf8String(const char *s)
    : data(s)
{
}

Utf8String::operator const char *() const
{
    return data.data();
}

Utf8String::operator std::string_view() const
{
    return data.data();
}

//
// RectF
//

RectF::RectF(qreal leftX,
             qreal bottomY,
             qreal width,
             qreal height)
    : m_leftX(leftX)
    , m_bottomY(bottomY)
    , m_width(width)
    , m_height(height)
{
}

qreal RectF::x() const
{
    return m_leftX;
}

qreal RectF::bottomY() const
{
    return m_bottomY;
}

qreal RectF::width() const
{
    return m_width;
}

qreal RectF::height() const
{
    return m_height;
}

void RectF::setWidth(qreal w)
{
    m_width = w;
}

//
// PrevBaselineStateStack
//

PrevBaselineStateStack::PrevBaselineStateStack(double lineHeight,
                                               double descent)
{
    m_stack.push_back({0.0, 1.0, lineHeight, descent});
}

double PrevBaselineStateStack::nextLineHeight(double lineHeight) const
{
    return lineHeight * nextScale();
}

double PrevBaselineStateStack::nextBaselineDelta(bool up) const
{
    return (up ? (m_stack.back().m_lineHeight - currentDescent()) * s_baselineScale : currentDescent());
}

double PrevBaselineStateStack::currentBaselineDelta() const
{
    return m_stack.back().m_baselineDelta;
}

double PrevBaselineStateStack::nextScale() const
{
    return m_stack.back().m_scale / s_scale;
}

double PrevBaselineStateStack::currentScale() const
{
    return m_stack.back().m_scale;
}

double PrevBaselineStateStack::currentDescent() const
{
    return m_stack.back().m_descent;
}

double PrevBaselineStateStack::nextDescent(double descent) const
{
    return descent * nextScale();
}

bool PrevBaselineStateStack::isMarkColorEnabled() const
{
    return (m_mark > 0);
}

// pair.first - line height, pair.second - lower part, below descent.
std::pair<double,
          double>
PrevBaselineStateStack::fullLineHeight() const
{
    const auto firstHeight = m_stack.front().m_lineHeight;
    const auto firstDescent = m_stack.front().m_descent;
    double upper = 0.0;
    double lower = 0.0;

    for (auto it = std::next(m_stack.cbegin()), last = m_stack.cend(); it != last; ++it) {
        if (it->m_baselineDelta > 0.0) {
            if ((it->m_lineHeight - it->m_descent + it->m_baselineDelta) > (firstHeight - firstDescent)) {
                const double tmp = it->m_lineHeight - it->m_descent + it->m_baselineDelta - firstHeight + firstDescent;

                if (tmp > upper) {
                    upper = tmp;
                }
            }
        } else {
            const double tmp = qAbs(it->m_baselineDelta + it->m_descent + firstDescent);

            if (tmp > lower) {
                lower = tmp;
            }
        }
    }

    return {firstHeight + upper + lower, lower};
}

const double PrevBaselineStateStack::s_scale = 1.5;
const double PrevBaselineStateStack::s_baselineScale = 0.5;

//
// LayoutDirectionHandler
//

double LayoutDirectionHandler::x() const
{
    return m_coords.m_x;
}
double LayoutDirectionHandler::y() const
{
    return m_coords.m_y;
}
void LayoutDirectionHandler::setRightToLeft(bool on)
{
    m_isRightToLeft = on;
}
bool LayoutDirectionHandler::isRightToLeft() const
{
    return m_isRightToLeft;
}
void LayoutDirectionHandler::setX(double value)
{
    m_coords.m_x = value;
}
void LayoutDirectionHandler::addX(double value)
{
    m_coords.m_x += xIncrementDirection() * value;
}
void LayoutDirectionHandler::moveXToBegin()
{
    setX((isRightToLeft() ? rightBorderXWithOffset() : leftBorderXWithOffset()));
}
void LayoutDirectionHandler::setY(double value)
{
    m_coords.m_y = value;
}
void LayoutDirectionHandler::addY(double value,
                                  double direction)
{
    m_coords.m_y += direction * value;
}
double LayoutDirectionHandler::leftBorderXWithOffset() const
{
    return (m_coords.m_margins.m_left
            + (!m_offset.empty() && m_offset.back()->m_left ? m_offset.back()->m_value : 0.0));
}
double LayoutDirectionHandler::rightBorderXWithOffset() const
{
    return (m_coords.m_pageWidth
            - m_coords.m_margins.m_right
            - (!m_offset.empty() && !m_offset.back()->m_left ? m_offset.back()->m_value : 0.0));
}

bool LayoutDirectionHandler::isFit(double width) const
{
    return (isRightToLeft()
                ? (x() - width >= leftBorderXWithOffset() || qAbs(leftBorderXWithOffset() - x() + width) < 0.01)
                : (x() + width <= rightBorderXWithOffset() || qAbs(x() + width - rightBorderXWithOffset()) < 0.01));
}

double LayoutDirectionHandler::topY() const
{
    return m_coords.m_margins.m_top;
}
const PageMargins &LayoutDirectionHandler::margins() const
{
    return m_coords.m_margins;
}
PageMargins &LayoutDirectionHandler::margins()
{
    return m_coords.m_margins;
}
double LayoutDirectionHandler::pageWidth() const
{
    return m_coords.m_pageWidth;
}
double LayoutDirectionHandler::pageHeight() const
{
    return m_coords.m_pageHeight;
}
double LayoutDirectionHandler::borderStartX() const
{
    return (isRightToLeft() ? m_coords.m_pageWidth - m_coords.m_margins.m_right : m_coords.m_margins.m_left);
}
double LayoutDirectionHandler::xIncrementDirection() const
{
    return (isRightToLeft() ? -1.0 : 1.0);
}
RectF LayoutDirectionHandler::currentRect(double width,
                                          double height,
                                          double baseline) const
{
    return RectF(startX(width), y(), width, height);
}
double LayoutDirectionHandler::startX(double width) const
{
    return (isRightToLeft() ? x() - width : x());
}
double LayoutDirectionHandler::availableWidth() const
{
    return (isRightToLeft() ? x() - leftBorderXWithOffset() : rightBorderXWithOffset() - x());
}

LayoutDirectionHandler::Offset::Offset(std::vector<Offset *> &offsets,
                                       double value,
                                       bool left)
    : m_value(value)
    , m_left(left)
    , m_offsets(offsets)
{
    m_offsets.push_back(this);
}

LayoutDirectionHandler::Offset::~Offset()
{
    m_offsets.pop_back();
}

LayoutDirectionHandler::Offset LayoutDirectionHandler::addOffset(double value,
                                                                 bool left)
{
    return Offset(m_offset, value, left);
}

//
// PdfAuxData
//

double PdfAuxData::topY(int page) const
{
    if (!m_drawFootnotes) {
        return m_layout.topY();
    } else {
        return topFootnoteY(page);
    }
}

int PdfAuxData::currentPageIndex() const
{
    if (!m_drawFootnotes) {
        return m_currentPageIdx;
    } else {
        return m_footnotePageIdx;
    }
}

double PdfAuxData::topFootnoteY(int page) const
{
    if (m_reserved.contains(page)) {
        return m_layout.pageHeight() - m_reserved[page];
    } else {
        return m_layout.pageHeight() - m_layout.m_coords.m_margins.m_bottom;
    }
}

double PdfAuxData::currentPageAllowedY() const
{
    return allowedY(m_currentPageIdx);
}

double PdfAuxData::allowedY(int page) const
{
    if (!m_drawFootnotes) {
        if (m_reserved.contains(page)) {
            return m_layout.pageHeight() - m_reserved[page];
        } else {
            return m_layout.pageHeight() - m_layout.margins().m_bottom;
        }
    } else {
        return m_layout.pageHeight() - m_layout.margins().m_bottom;
    }
}

void PdfAuxData::freeSpaceOn(int page)
{
    if (!m_drawFootnotes) {
        if (m_reserved.contains(page)) {
            double r = m_reserved[page];
            m_reserved.remove(page);

            if (page == m_footnotePageIdx) {
                m_footnotePageIdx = page + 1;
            }

            while (m_reserved.contains(++page)) {
                const double tmp = m_reserved[page];
                m_reserved[page] = r;
                r = tmp;
            }

            m_reserved[page] = r;
        }
    }
}

void PdfAuxData::drawBlob(double x,
                          const sk_sp<SkTextBlob> &blob)
{
    m_firstOnPage = false;

    SkPaint paint = m_currentPaint;
    paint.setAntiAlias(true);

#ifndef MD_PDF_TESTING
    (*m_pages)[m_currentPainterIdx].m_canvas->drawTextBlob(blob, x, 0.0, paint);
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Blob 0 \"\" %1 %2 0.0 0.0 %3 %4 0.0 0.0\n")
                                   .arg(QString::number(x, 'f', 16),
                                        QString::number(blob->bounds().y(), 'f', 16),
                                        QString::number(blob->bounds().width(), 'f', 16),
                                        QString::number(blob->bounds().height(), 'f', 16));
    } else {
        (*m_pages)[m_currentPainterIdx].m_canvas->drawTextBlob(blob, x, 0.0, paint);

        if (QTest::currentTestFailed()) {
            m_self->terminate();
        }

        int pos = m_testPos++;
        QCOMPARE(DrawPrimitive::Type::Blob, m_testData.at(pos).m_type);
        QCOMPARE(x, m_testData.at(pos).m_x);
        QCOMPARE(blob->bounds().y(), m_testData.at(pos).m_y);
        QCOMPARE(blob->bounds().width(), m_testData.at(pos).m_width);
        QCOMPARE(blob->bounds().height(), m_testData.at(pos).m_height);
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::drawText(double x,
                          double y,
                          const Utf8String &text,
                          const Font &font,
                          double size,
                          double scale,
                          bool strikeout)
{
    m_firstOnPage = false;

    auto copyFont = font;
    copyFont.setSize(size);
    copyFont.setScaleX(scale);
    SkPaint paint = m_currentPaint;
    paint.setAntiAlias(true);

#ifndef MD_PDF_TESTING
    (*m_pages)[m_currentPainterIdx].m_canvas->drawString(text, x, y, copyFont, paint);

    if (strikeout) {
        SkPaint paint = m_currentPaint;
        SkFontMetrics fm;
        copyFont.getMetrics(&fm);
        paint.setStrokeWidth(fm.fStrikeoutThickness);

        (*m_pages)[m_currentPainterIdx].m_canvas->drawLine(
            x,
            y + fm.fStrikeoutPosition,
            x + copyFont.measureText(text, text.data.size(), SkTextEncoding::kUTF8),
            y + fm.fStrikeoutPosition,
            paint);
    }
#else
    if (m_printDrawings) {
        const auto s = PdfRenderer::createQString(text);

        (*m_drawingsStream) << QStringLiteral("Text %1 \"%2\" %3 %4 0.0 0.0 0.0 0.0 0.0 0.0\n")
                                   .arg(QString::number(s.length()),
                                        s,
                                        QString::number(x, 'f', 16),
                                        QString::number(y, 'f', 16));
    } else {
        (*m_pages)[m_currentPainterIdx].m_canvas->drawString(text, x, y, copyFont, paint);

        if (strikeout) {
            SkPaint paint = m_currentPaint;
            SkFontMetrics fm;
            copyFont.getMetrics(&fm);
            paint.setStrokeWidth(fm.fStrikeoutThickness);

            (*m_pages)[m_currentPainterIdx].m_canvas->drawLine(
                x,
                y + fm.fStrikeoutPosition,
                x + copyFont.measureText(text, text.data.size(), SkTextEncoding::kUTF8),
                y + fm.fStrikeoutPosition,
                paint);
        }

        if (QTest::currentTestFailed()) {
            m_self->terminate();
        }

        int pos = m_testPos++;
        QCOMPARE(DrawPrimitive::Type::Text, m_testData.at(pos).m_type);
        QCOMPARE(PdfRenderer::createQString(text), m_testData.at(pos).m_text);
        QCOMPARE(x, m_testData.at(pos).m_x);
        QCOMPARE(y, m_testData.at(pos).m_y);
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::drawImage(double x,
                           double y,
                           const Image *img,
                           double xScale,
                           double yScale)
{
    m_firstOnPage = false;

#ifndef MD_PDF_TESTING
    (*m_pages)[m_currentPainterIdx].m_canvas->save();
    (*m_pages)[m_currentPainterIdx].m_canvas->scale(xScale, yScale);
    (*m_pages)[m_currentPainterIdx].m_canvas->drawImage(
        img,
        x / xScale,
        y / yScale,
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
    (*m_pages)[m_currentPainterIdx].m_canvas->restore();
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Image 0 \"\" %1 %2 0.0 0.0 0.0 0.0 %3 %4\n")
                                   .arg(QString::number(x, 'f', 16),
                                        QString::number(y, 'f', 16),
                                        QString::number(xScale, 'f', 16),
                                        QString::number(yScale, 'f', 16));
    } else {
        (*m_pages)[m_currentPainterIdx].m_canvas->save();
        (*m_pages)[m_currentPainterIdx].m_canvas->scale(xScale, yScale);
        (*m_pages)[m_currentPainterIdx].m_canvas->drawImage(
            img,
            x / xScale,
            y / yScale,
            SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear));
        (*m_pages)[m_currentPainterIdx].m_canvas->restore();

        if (QTest::currentTestFailed()) {
            m_self->terminate();
        }

        int pos = m_testPos++;
        QCOMPARE(x, m_testData.at(pos).m_x);
        QCOMPARE(y, m_testData.at(pos).m_y);
        QCOMPARE(xScale, m_testData.at(pos).m_xScale);
        QCOMPARE(yScale, m_testData.at(pos).m_yScale);
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::drawImage(double x,
                           double y,
                           const SkSVGDOM *img,
                           double xScale,
                           double yScale)
{
    m_firstOnPage = false;

#ifndef MD_PDF_TESTING
    (*m_pages)[m_currentPainterIdx].m_canvas->save();
    (*m_pages)[m_currentPainterIdx].m_canvas->translate(x, y);
    (*m_pages)[m_currentPainterIdx].m_canvas->scale(xScale, yScale);
    img->render((*m_pages)[m_currentPainterIdx].m_canvas);
    (*m_pages)[m_currentPainterIdx].m_canvas->restore();
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Image 0 \"\" %1 %2 0.0 0.0 0.0 0.0 %3 %4\n")
                                   .arg(QString::number(x, 'f', 16),
                                        QString::number(y, 'f', 16),
                                        QString::number(xScale, 'f', 16),
                                        QString::number(yScale, 'f', 16));
    } else {
        (*m_pages)[m_currentPainterIdx].m_canvas->save();
        (*m_pages)[m_currentPainterIdx].m_canvas->translate(x, y);
        (*m_pages)[m_currentPainterIdx].m_canvas->scale(xScale, yScale);
        img->render((*m_pages)[m_currentPainterIdx].m_canvas);
        (*m_pages)[m_currentPainterIdx].m_canvas->restore();

        if (QTest::currentTestFailed()) {
            m_self->terminate();
        }

        int pos = m_testPos++;
        QCOMPARE(x, m_testData.at(pos).m_x);
        QCOMPARE(y, m_testData.at(pos).m_y);
        QCOMPARE(xScale, m_testData.at(pos).m_xScale);
        QCOMPARE(yScale, m_testData.at(pos).m_yScale);
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::drawLine(double x1,
                          double y1,
                          double x2,
                          double y2)
{
#ifndef MD_PDF_TESTING
    (*m_pages)[m_currentPainterIdx].m_canvas->drawLine(x1, y1, x2, y2, m_currentPaint);
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Line 0 \"\" %1 %2 %3 %4 0.0 0.0 0.0 0.0\n")
                                   .arg(QString::number(x1, 'f', 16),
                                        QString::number(y1, 'f', 16),
                                        QString::number(x2, 'f', 16),
                                        QString::number(y2, 'f', 16));
    } else {
        (*m_pages)[m_currentPainterIdx].m_canvas->drawLine(x1, y1, x2, y2, m_currentPaint);

        if (QTest::currentTestFailed()) {
            m_self->terminate();
        }

        int pos = m_testPos++;
        QCOMPARE(x1, m_testData.at(pos).m_x);
        QCOMPARE(y1, m_testData.at(pos).m_y);
        QCOMPARE(x2, m_testData.at(pos).m_x2);
        QCOMPARE(y2, m_testData.at(pos).m_y2);
    }
#endif // MD_PDF_TESTING
}

SkSize a4Size()
{
    static const float inchToPoint = 72.0f;
    static const float widthInInches = 8.27f;
    static const float heightInInches = 11.69f;

    return {widthInInches * inchToPoint, heightInInches * inchToPoint};
}

void PdfAuxData::save(const QString &fileName)
{
    const auto write = [this](const QString &fileName) {
        SkDynamicMemoryWStream buffer;
        SkPDF::Metadata metadata;
        metadata.jpegDecoder = SkPDF::JPEG::Decode;
        metadata.jpegEncoder = SkPDF::JPEG::Encode;
        metadata.fTitle = SkString(fileName.toUtf8().data());
        metadata.fCreator =
            "This PDF was generated with Markdown Tools\n"
            "https://github.com/igormironchik/markdown-tools";
        auto pdfDocument = SkPDF::MakeDocument(&buffer, metadata);

        for (const auto &p : *m_pages) {
            SkCanvas *pageCanvas = pdfDocument->beginPage(a4Size().width(), a4Size().height());
            pageCanvas->drawPicture(p.m_recorder->finishRecordingAsPicture());
        }

        pdfDocument->close();

        sk_sp<SkData> pdfData = buffer.detachAsData();

        std::fstream f(fileName.toLocal8Bit().data(), std::ios::binary | std::ios::out);

        f.write(static_cast<const char *>(pdfData->writable_data()), pdfData->size());

        f.close();
    };

#ifndef MD_PDF_TESTING
    write(fileName);
#else
    if (!m_printDrawings) {
        write(fileName);
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::drawRectangle(double x,
                               double y,
                               double width,
                               double height,
                               SkPaint::Style m)
{
    auto paint = m_currentPaint;
    paint.setStyle(m);

#ifndef MD_PDF_TESTING
    (*m_pages)[m_currentPainterIdx].m_canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Rectangle 0 \"\" %1 %2 0.0 0.0 %3 %4 0.0 0.0\n")
                                   .arg(QString::number(x, 'f', 16),
                                        QString::number(y, 'f', 16),
                                        QString::number(width, 'f', 16),
                                        QString::number(height, 'f', 16));
    } else {
        (*m_pages)[m_currentPainterIdx].m_canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);

        if (QTest::currentTestFailed()) {
            m_self->terminate();
        }

        int pos = m_testPos++;
        QCOMPARE(x, m_testData.at(pos).m_x);
        QCOMPARE(y, m_testData.at(pos).m_y);
        QCOMPARE(width, m_testData.at(pos).m_width);
        QCOMPARE(height, m_testData.at(pos).m_height);
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::setColor(const QColor &c)
{
    m_colorsStack.push(c);
    m_currentPaint.setColor(SkColorSetRGB(c.red(), c.green(), c.blue()));
}

void PdfAuxData::restoreColor()
{
    if (m_colorsStack.size() > 1) {
        m_colorsStack.pop();
    }

    repeatColor();
}

void PdfAuxData::repeatColor()
{
    const auto &c = m_colorsStack.top();
    m_currentPaint.setColor(SkColorSetRGB(c.red(), c.green(), c.blue()));
}

TextBlobBuilderRunHandler::TextBlobBuilderRunHandler(const char *utf8Text,
                                                     SkPoint offset)
    : fUtf8Text(utf8Text)
    , fOffset(offset)
{
}

sk_sp<SkTextBlob> TextBlobBuilderRunHandler::makeBlob()
{
    return fBuilder.make();
}

SkPoint TextBlobBuilderRunHandler::endPoint()
{
    return fOffset;
}

void TextBlobBuilderRunHandler::beginLine()
{
    fCurrentPosition = fOffset;
    fMaxRunAscent = 0;
    fMaxRunDescent = 0;
    fMaxRunLeading = 0;
}

void TextBlobBuilderRunHandler::runInfo(const RunInfo &info)
{
    SkFontMetrics metrics;
    info.fFont.getMetrics(&metrics);
    fMaxRunAscent = std::min(fMaxRunAscent, metrics.fAscent);
    fMaxRunDescent = std::max(fMaxRunDescent, metrics.fDescent);
    fMaxRunLeading = std::max(fMaxRunLeading, metrics.fLeading);
}

void TextBlobBuilderRunHandler::commitRunInfo()
{
    fCurrentPosition.fY -= fMaxRunAscent;
}

TextBlobBuilderRunHandler::Buffer TextBlobBuilderRunHandler::runBuffer(const RunInfo &info)
{
    int glyphCount = SkTFitsIn<int>(info.glyphCount) ? info.glyphCount : INT_MAX;
    int utf8RangeSize = SkTFitsIn<int>(info.utf8Range.size()) ? info.utf8Range.size() : INT_MAX;

    const auto &runBuffer = fBuilder.allocRunTextPos(info.fFont, glyphCount, utf8RangeSize);
    if (runBuffer.utf8text && fUtf8Text) {
        memcpy(runBuffer.utf8text, fUtf8Text + info.utf8Range.begin(), utf8RangeSize);
    }
    fClusters = runBuffer.clusters;
    fGlyphCount = glyphCount;
    fClusterOffset = info.utf8Range.begin();

    return {runBuffer.glyphs, runBuffer.points(), nullptr, runBuffer.clusters, fCurrentPosition};
}

void TextBlobBuilderRunHandler::commitRunBuffer(const RunInfo &info)
{
    SkASSERT(0 <= fClusterOffset);
    for (int i = 0; i < fGlyphCount; ++i) {
        SkASSERT(fClusters[i] >= (unsigned)fClusterOffset);
        fClusters[i] -= fClusterOffset;
    }
    fCurrentPosition += info.fAdvance;
}

void TextBlobBuilderRunHandler::commitLine()
{
    fOffset += {0, fMaxRunDescent + fMaxRunLeading - fMaxRunAscent};
}

SkScalar TextBlobBuilderRunHandler::horizontalAdvance() const
{
    return fCurrentPosition.fX;
}

bool PdfAuxData::shape(TextBlobBuilderRunHandler &handler,
                       const Font &font,
                       double size,
                       double scale,
                       const String &s,
                       bool leftToRight) const
{
    if (m_shaper) {
        auto copyFont = font;
        copyFont.setSize(size * scale);

        SkBidiIterator::Level defaultLevel = leftToRight ? SkBidiIterator::kLTR : SkBidiIterator::kRTL;
        std::unique_ptr<SkShaper::BiDiRunIterator> bidi(
            SkShapers::unicode::BidiRunIterator(m_unicode, s, s.data.size(), defaultLevel));

        if (!bidi) {
            return false;
        }

        std::unique_ptr<SkShaper::LanguageRunIterator> language(SkShaper::MakeStdLanguageRunIterator(s, s.data.size()));

        if (!language) {
            return false;
        }

        std::unique_ptr<SkShaper::ScriptRunIterator> script(SkShapers::HB::ScriptRunIterator(s, s.data.size()));

        if (!script) {
            return false;
        }

        std::unique_ptr<SkShaper::FontRunIterator> font(
            std::make_unique<SkShaper::TrivialFontRunIterator>(copyFont, s.data.size()));

        if (!font) {
            return false;
        }

        SkShaper::Feature features[] = {
            {SkSetFourByteTag('l', 'i', 'g', 'a'), 0, 0, static_cast<size_t>(s.data.size())},
            {SkSetFourByteTag('c', 'a', 'l', 't'), 0, 0, static_cast<size_t>(s.data.size())}};
        size_t featuresSize = sizeof(features) / sizeof(features[0]);

        m_shaper
            ->shape(s, s.data.size(), *font, *bidi, *script, *language, features, featuresSize, 999999.0f, &handler);

        return true;
    }

    return false;
}

double PdfAuxData::stringWidth(const Font &font,
                               double size,
                               double scale,
                               const String &s,
                               bool leftToRight) const
{
    auto copyFont = font;
    copyFont.setSize(size * scale);

    TextBlobBuilderRunHandler handler(s, SkPoint::Make(0.0, 0.0));

    if (!shape(handler, font, size, scale, s, leftToRight)) {
        return copyFont.measureText(s, s.data.size(), SkTextEncoding::kUTF8);
    }

    return handler.horizontalAdvance();
}

double PdfAuxData::lineSpacing(const Font &font,
                               double size,
                               double scale) const
{
    auto copyFont = font;
    copyFont.setSize(size * scale);

    return copyFont.getMetrics(nullptr);
}

double PdfAuxData::fontAscent(const Font &font,
                              double size,
                              double scale) const
{
    auto copyFont = font;
    copyFont.setSize(size * scale);
    SkFontMetrics fm;
    copyFont.getMetrics(&fm);

    return fm.fAscent;
}

double PdfAuxData::fontBackgroundBoxScale(const Font &font,
                                          double size,
                                          double scale) const
{
    auto copyFont = font;
    copyFont.setSize(size * scale);
    SkFontMetrics fm;
    const auto ls = copyFont.getMetrics(&fm);

    return (fm.fDescent + qAbs(fm.fCapHeight)) / ls;
}

double PdfAuxData::fontDescent(const Font &font,
                               double size,
                               double scale) const
{
    auto copyFont = font;
    copyFont.setSize(size * scale);
    SkFontMetrics fm;
    copyFont.getMetrics(&fm);

    return fm.fDescent;
}

//
// RTLFlag
//

RTLFlag::RTLFlag()
    : m_isOn(false)
    , m_check(true)
{
}

bool RTLFlag::isCheck() const
{
    return m_check;
}
bool RTLFlag::isRightToLeft() const
{
    return m_isOn;
}

//
// CustomWidth
//

void CustomWidth::append(const Width &w)
{
    m_width.append(w);
}
double CustomWidth::scale() const
{
    return m_scale.at(m_pos);
}
double CustomWidth::height() const
{
    return m_height.at(m_pos);
}

double CustomWidth::descent() const
{
    return m_descent.at(m_pos);
}

double CustomWidth::width() const
{
    return m_lineWidth.at(m_pos);
}

void CustomWidth::moveToNextLine()
{
    ++m_pos;
}

bool CustomWidth::isDrawing() const
{
    return m_drawing;
}

void CustomWidth::setDrawing(bool on)
{
    m_drawing = on;
}

bool CustomWidth::isNewLineAtEnd() const
{
    return (m_width.isEmpty() ? false : m_width.back().m_isNewLine);
}

QVector<double>::ConstIterator CustomWidth::cbegin() const
{
    return m_height.cbegin();
}

QVector<double>::ConstIterator CustomWidth::cend() const
{
    return m_height.cend();
}

double CustomWidth::firstLineHeight() const
{
    if (!m_height.isEmpty()) {
        return m_height.first();
    } else {
        return 0.0;
    }
}

void CustomWidth::calcScale(double lineWidth)
{
    double w = 0.0;
    double sw = 0.0;
    double ww = 0.0;
    double h = 0.0;
    double d = 0.0;
    double lastSpaceWidth = 0.0;

    for (int i = 0, last = m_width.size(); i < last; ++i) {
        if (m_width.at(i).m_descent > d) {
            d = m_width.at(i).m_descent;
        }

        if (m_width.at(i).m_height - m_width.at(i).m_descent > h) {
            h = m_width.at(i).m_height - m_width.at(i).m_descent;
        }

        w += m_width.at(i).m_width;

        if (m_width.at(i).m_isSpace) {
            sw += m_width.at(i).m_width;
            lastSpaceWidth = m_width.at(i).m_width;
        } else {
            ww += m_width.at(i).m_width;

            if (m_width.at(i).m_width > 0.0) {
                lastSpaceWidth = 0.0;
            }
        }

        if (m_width.at(i).m_isNewLine) {
            if (lastSpaceWidth > 0.0) {
                w -= lastSpaceWidth;
                sw -= lastSpaceWidth;
                lastSpaceWidth = 0.0;
            }

            if (m_width.at(i).m_shrink) {
                auto ss = (lineWidth - ww) / sw;

                while (ww + sw * ss > lineWidth) {
                    ss -= 0.001;
                }

                m_scale.append(100.0 * ss);
            } else {
                m_scale.append(100.0);
            }

            double widthWithoutLastSpaces = w;

            for (int j = i; j >= 0; --j) {
                if (m_width.at(j).m_isSpace) {
                    widthWithoutLastSpaces -= m_width.at(j).m_width;
                } else {
                    break;
                }
            }

            if (m_width.at(i).m_alignment != ParagraphAlignment::Unknown) {
                m_alignment.append(m_width.at(i).m_alignment);
            } else {
                m_alignment.append(ParagraphAlignment::Unknown);
            }

            m_height.append(h + d);
            m_lineWidth.append(widthWithoutLastSpaces);
            m_descent.append(d);

            w = 0.0;
            sw = 0.0;
            ww = 0.0;
            h = 0.0;
            d = 0.0;
        }
    }
}

ParagraphAlignment CustomWidth::alignment() const
{
    return m_alignment.at(m_pos);
}

void CustomWidth::setAlignment(ParagraphAlignment alignment)
{
    std::for_each(m_alignment.begin(), m_alignment.end(), [alignment](auto &a) {
        if (a == ParagraphAlignment::Unknown) {
            a = alignment;
        }
    });
}

//
// AutoSubSupScriptInit
//

AutoSubSupScriptInit::AutoSubSupScriptInit(PdfRenderer *render,
                                           MD::ItemWithOpts *item,
                                           PrevBaselineStateStack &stack,
                                           double lineHeight,
                                           double descent)
    : m_render(render)
    , m_item(item)
    , m_stack(stack)
    , m_count(m_stack.m_stack.size())
{
    m_render->initSubSupScript(m_item, m_stack, lineHeight, descent);
}

AutoSubSupScriptInit::~AutoSubSupScriptInit()
{
    m_render->deinitSubSupScript(m_item, m_stack);
}

bool AutoSubSupScriptInit::wasAdded() const
{
    return (m_count != m_stack.m_stack.size());
}

//
// LoadImageFromNetwork
//

//
// LoadImageFromNetwork
//

LoadImageFromNetwork::LoadImageFromNetwork(const QUrl &url,
                                           QThread *thread,
                                           double height,
                                           bool scale)
    : m_thread(thread)
    , m_reply(nullptr)
    , m_url(url)
    , m_height(height)
    , m_scale(scale)
{
    connect(this, &LoadImageFromNetwork::start, this, &LoadImageFromNetwork::loadImpl, Qt::QueuedConnection);
}

const QImage &LoadImageFromNetwork::image() const
{
    return m_img;
}

void LoadImageFromNetwork::load()
{
    Q_EMIT start();
}

bool LoadImageFromNetwork::isSvg() const
{
    return !m_svgData.isEmpty();
}

const QByteArray &LoadImageFromNetwork::svgData() const
{
    return m_svgData;
}

void LoadImageFromNetwork::loadImpl()
{
    QNetworkAccessManager *m = new QNetworkAccessManager(this);
    QNetworkRequest r(m_url);
    r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m->get(r);

    connect(m_reply, &QNetworkReply::finished, this, &LoadImageFromNetwork::loadFinished);
    connect(m_reply,
            static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::errorOccurred),
            this,
            &LoadImageFromNetwork::loadError);
}

void LoadImageFromNetwork::loadFinished()
{
    auto data = m_reply->readAll();
    const auto svg = QString(data.mid(0, 4).toLower());

    if (svg == QStringLiteral("<svg")
        || svg == QStringLiteral("<?xm")
        || m_reply->url().fileName().toLower().endsWith(QStringLiteral("svgz"))
        || m_reply->url().fileName().toLower().endsWith(QStringLiteral("svg.gz"))
        || m_reply->url().fileName().toLower().endsWith(QStringLiteral("svg"))) {
        if (m_reply->url().fileName().toLower().endsWith(QStringLiteral("svgz"))
            || m_reply->url().fileName().toLower().endsWith(QStringLiteral("svg.gz"))) {
            m_reply->seek(0);
            m_svgData = qt_inflateSvgzDataFrom(m_reply, true);
        } else {
            m_svgData = data;
        }
    } else {
        m_img.loadFromData(data);
    }

    m_reply->deleteLater();

    m_thread->quit();
}

void LoadImageFromNetwork::loadError(QNetworkReply::NetworkError)
{
    m_reply->deleteLater();
    m_thread->quit();
}

} /* namespace Render */

} /* namespace MdPdf */
