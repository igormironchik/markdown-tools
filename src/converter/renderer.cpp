/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "renderer.hpp"
#include "const.hpp"
#include "podofo_paintdevice.hpp"

// shared include.
#include "utils.hpp"

#ifdef MD_PDF_TESTING
#include <QtTest/QtTest>
#include <test_const.hpp>
#endif // MD_PDF_TESTING

// Qt include.
#include <QApplication>
#include <QBuffer>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QPainter>
#include <QRegularExpression>
#include <QScreen>
#include <QTemporaryFile>
#include <QThread>

// MicroTeX include.
#include <platform/qt/graphic_qt.h>
#include <latex.h>

// C++ include.
#include <cmath>
#include <functional>
#include <utility>

// md4qt include.
#include <md4qt/algo.h>

namespace MdPdf
{

namespace Render
{

//
// PdfRendererError
//

//! Internal exception.
class PdfRendererError
{
public:
    explicit PdfRendererError(const QString &reason)
        : m_what(reason)
    {
    }

    const QString &what() const noexcept
    {
        return m_what;
    }

private:
    QString m_what;
}; // class PdfRendererError

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
        return m_reserved[page];
    } else {
        return 0.0;
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
            return m_reserved[page];
        } else {
            return m_layout.margins().m_bottom;
        }
    } else {
        return m_layout.margins().m_bottom;
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

void PdfAuxData::drawText(double x, double y, const char *text, Font *font, double size, double scale, bool strikeout)
{
    m_firstOnPage = false;

#ifndef MD_PDF_TESTING
    (*m_painters)[m_currentPainterIdx]->TextObject.Begin();
    (*m_painters)[m_currentPainterIdx]->TextObject.MoveTo(x, y);
    (*m_painters)[m_currentPainterIdx]->TextState.SetFont(*font, size);
    (*m_painters)[m_currentPainterIdx]->TextState.SetFontScale(scale);
    const auto st = (*m_painters)[m_currentPainterIdx]->TextState;
    (*m_painters)[m_currentPainterIdx]->TextObject.AddText(text);
    (*m_painters)[m_currentPainterIdx]->TextObject.End();

    if (strikeout) {
        (*m_painters)[m_currentPainterIdx]->Save();

        (*m_painters)[m_currentPainterIdx]->GraphicsState.SetLineWidth(font->GetStrikeThroughThickness(st));

        (*m_painters)[m_currentPainterIdx]->DrawLine(x,
                                                 y + font->GetStrikeThroughPosition(st),
                                                 x + font->GetStringLength(text, st),
                                                 y + font->GetStrikeThroughPosition(st));

        (*m_painters)[m_currentPainterIdx]->Restore();
    }
#else
    if (m_printDrawings) {
        const auto s = PdfRenderer::createQString(text);

        (*m_drawingsStream) << QStringLiteral("Text %1 \"%2\" %3 %4 0.0 0.0 0.0 0.0 0.0 0.0\n")
                                 .arg(QString::number(s.length()), s, QString::number(x, 'f', 16), QString::number(y, 'f', 16));
    } else {
        (*m_painters)[m_currentPainterIdx]->TextObject.Begin();
        (*m_painters)[m_currentPainterIdx]->TextObject.MoveTo(x, y);
        (*m_painters)[m_currentPainterIdx]->TextState.SetFont(*font, size);
        (*m_painters)[m_currentPainterIdx]->TextState.SetFontScale(scale);
        const auto st = (*m_painters)[m_currentPainterIdx]->TextState;
        (*m_painters)[m_currentPainterIdx]->TextObject.AddText(text);
        (*m_painters)[m_currentPainterIdx]->TextObject.End();

        if (strikeout) {
            (*m_painters)[m_currentPainterIdx]->Save();

            (*m_painters)[m_currentPainterIdx]->GraphicsState.SetLineWidth(font->GetStrikeThroughThickness(st));

            (*m_painters)[m_currentPainterIdx]->DrawLine(x,
                                                     y + font->GetStrikeThroughPosition(st),
                                                     x + font->GetStringLength(text, st),
                                                     y + font->GetStrikeThroughPosition(st));

            (*m_painters)[m_currentPainterIdx]->Restore();
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

void PdfAuxData::drawImage(double x, double y, Image *img, double xScale, double yScale)
{
    m_firstOnPage = false;

#ifndef MD_PDF_TESTING
    (*m_painters)[m_currentPainterIdx]->DrawImage(*img, x, y, xScale, yScale);
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Image 0 \"\" %2 %3 0.0 0.0 0.0 0.0 %4 %5\n")
                                 .arg(QString::number(x, 'f', 16),
                                      QString::number(y, 'f', 16),
                                      QString::number(xScale, 'f', 16),
                                      QString::number(yScale, 'f', 16));
    } else {
        (*m_painters)[m_currentPainterIdx]->DrawImage(*img, x, y, xScale, yScale);

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

void PdfAuxData::drawLine(double x1, double y1, double x2, double y2)
{
#ifndef MD_PDF_TESTING
    (*m_painters)[m_currentPainterIdx]->DrawLine(x1, y1, x2, y2);
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Line 0 \"\" %1 %2 %3 %4 0.0 0.0 0.0 0.0\n")
                                 .arg(QString::number(x1, 'f', 16), QString::number(y1, 'f', 16),
                                      QString::number(x2, 'f', 16), QString::number(y2, 'f', 16));
    } else {
        (*m_painters)[m_currentPainterIdx]->DrawLine(x1, y1, x2, y2);

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

void PdfAuxData::save(const QString &fileName)
{
#ifndef MD_PDF_TESTING
    m_doc->Save(fileName.toLocal8Bit().data());
#else
    if (!m_printDrawings) {
        m_doc->Save(fileName.toLocal8Bit().data());
    }
#endif // MD_PDF_TESTING
}

void PdfAuxData::drawRectangle(double x, double y, double width, double height, PoDoFo::PdfPathDrawMode m)
{
#ifndef MD_PDF_TESTING
    (*m_painters)[m_currentPainterIdx]->DrawRectangle(x, y, width, height, m);
#else
    if (m_printDrawings) {
        (*m_drawingsStream) << QStringLiteral("Rectangle 0 \"\" %1 %2 0.0 0.0 %3 %4 0.0 0.0\n")
                                 .arg(QString::number(x, 'f', 16),
                                      QString::number(y, 'f', 16),
                                      QString::number(width, 'f', 16),
                                      QString::number(height, 'f', 16));
    } else {
        (*m_painters)[m_currentPainterIdx]->DrawRectangle(x, y, width, height, m);

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

    (*m_painters)[m_currentPainterIdx]->GraphicsState.SetNonStrokingColor(Color(c.redF(), c.greenF(), c.blueF()));
    (*m_painters)[m_currentPainterIdx]->GraphicsState.SetStrokingColor(Color(c.redF(), c.greenF(), c.blueF()));
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

    (*m_painters)[m_currentPainterIdx]->GraphicsState.SetNonStrokingColor(Color(c.redF(), c.greenF(), c.blueF()));
    (*m_painters)[m_currentPainterIdx]->GraphicsState.SetStrokingColor(Color(c.redF(), c.greenF(), c.blueF()));
}

double PdfAuxData::imageWidth(const QByteArray &image)
{
    auto pdfImg = m_doc->CreateImage();
    pdfImg->LoadFromBuffer({image.data(), static_cast<size_t>(image.size())});

    return std::round((double)pdfImg->GetWidth() / (double)m_dpi * 72.0);
}

double PdfAuxData::stringWidth(Font *font, double size, double scale, const String &s) const
{
    PoDoFo::PdfTextState st;
    st.FontSize = size * scale;

    return font->GetStringLength(s, st);
}

double PdfAuxData::lineSpacing(Font *font, double size, double scale) const
{
    PoDoFo::PdfTextState st;
    st.FontSize = size * scale;

    return font->GetLineSpacing(st);
}

double PdfAuxData::fontAscent(Font *font, double size, double scale) const
{
    PoDoFo::PdfTextState st;
    st.FontSize = size * scale;

    return font->GetAscent(st);
}

double PdfAuxData::fontDescent(Font *font, double size, double scale) const
{
    PoDoFo::PdfTextState st;
    st.FontSize = size * scale;

    return font->GetDescent(st);
}

//
// PdfRenderer::CustomWidth
//

double PdfRenderer::CustomWidth::firstItemHeight() const
{
    if (!m_width.isEmpty()) {
        return m_width.constFirst().m_height;
    } else {
        return 0.0;
    }
}

void PdfRenderer::CustomWidth::calcScale(double lineWidth)
{
    double w = 0.0;
    double sw = 0.0;
    double ww = 0.0;
    double h = 0.0;
    double lastSpaceWidth = 0.0;

    for (int i = 0, last = m_width.size(); i < last; ++i) {
        if (m_width.at(i).m_height > h) {
            h = m_width.at(i).m_height;
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

            m_height.append(h);

            w = 0.0;
            sw = 0.0;
            ww = 0.0;
            h = 0.0;
        }
    }
}

//
// PdfRenderer::CellItem
//

double PdfRenderer::CellItem::width(PdfAuxData &pdfData, PdfRenderer *render, double scale) const
{
    auto *f = render->createFont(m_font.m_family, m_font.m_bold, m_font.m_italic, m_font.m_size,
                                 pdfData.m_doc, scale, pdfData);

    if (!m_word.isEmpty()) {
        return pdfData.stringWidth(f, m_font.m_size, scale, createUtf8String(m_word));
    } else if (!m_image.isNull()) {
        return pdfData.imageWidth(m_image);
    } else if (!m_url.isEmpty()) {
        return pdfData.stringWidth(f, m_font.m_size, scale, createUtf8String(m_url));
    } else if (!m_footnote.isEmpty()) {
        return pdfData.stringWidth(f, m_font.m_size * s_footnoteScale, scale, createUtf8String(m_footnote));
    } else {
        return 0.0;
    }
}

//
// PdfRenderer::CellData
//

void PdfRenderer::CellData::heightToWidth(double lineHeight, double spaceWidth,
                                          double scale, PdfAuxData &pdfData, PdfRenderer *render)
{
    m_height = 0.0;

    bool newLine = true;

    double w = 0.0;

    bool addMargin = false;

    for (auto it = m_items.cbegin(), last = m_items.cend(); it != last; ++it) {
        if (it->m_image.isNull()) {
            addMargin = false;

            if (newLine) {
                m_height += lineHeight;
                newLine = false;
                w = 0.0;
            }

            w += it->width(pdfData, render, scale);

            if (w >= m_width) {
                newLine = true;
                continue;
            }

            double sw = spaceWidth;

            if (it != m_items.cbegin() && it->m_font != (it - 1)->m_font) {
                sw = pdfData.stringWidth(render->createFont(it->m_font.m_family, it->m_font.m_bold, it->m_font.m_italic,
                                                            it->m_font.m_size, pdfData.m_doc, scale, pdfData),
                                         it->m_font.m_size,
                                         scale,
                                         String(" "));
            }

            if (it + 1 != last && !(it + 1)->m_footnote.isEmpty()) {
                sw = 0.0;
            }

            if (it + 1 != last) {
                if (w + sw + (it + 1)->width(pdfData, render, scale) > m_width) {
                    newLine = true;
                } else {
                    w += sw;
                    newLine = false;
                }
            }
        } else {
            auto pdfImg = pdfData.m_doc->CreateImage();
            pdfImg->LoadFromBuffer({it->m_image.data(), static_cast<size_t>(it->m_image.size())});

            const double iWidth = std::round((double)pdfImg->GetWidth() / (double)pdfData.m_dpi * 72.0);
            const double iHeight = std::round((double)pdfImg->GetHeight() / (double)pdfData.m_dpi * 72.0);

            if (iWidth > m_width) {
                m_height += iHeight / (iWidth / m_width) * scale;
            } else {
                m_height += iHeight * scale;
            }

            newLine = true;

            if (!addMargin) {
                addMargin = true;
            } else {
                continue;
            }
        }

        if (addMargin) {
            m_height += s_tableMargin;
        }
    }
}

//
// PdfRenderer
//

PdfRenderer::PdfRenderer()
    : m_terminate(false)
    , m_footnoteNum(1)
#ifdef MD_PDF_TESTING
    , m_isError(false)
#endif
{
    connect(this, &PdfRenderer::start, this, &PdfRenderer::renderImpl, Qt::QueuedConnection);
}

void PdfRenderer::render(const QString &fileName, std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                         const RenderOpts &opts, bool testing)
{
    m_fileName = fileName;
    m_doc = doc;
    m_opts = opts;

    if (!testing) {
        emit start();
    }
}

void PdfRenderer::terminate()
{
    QMutexLocker lock(&m_mutex);

    m_terminate = true;

#ifdef MD_PDF_TESTING
    QFAIL("Test terminated.");
#endif
}

#ifdef MD_PDF_TESTING
bool PdfRenderer::isError() const
{
    return m_isError;
}
#endif

void PdfRenderer::renderImpl()
{
    PdfAuxData pdfData;

    try {
        const int itemsCount = m_doc->items().size();

        emit progress(0);
        emit status(tr("Rendering PDF..."));

        Document document;
        std::vector<std::shared_ptr<Painter>> painters;

        pdfData.m_doc = &document;
        pdfData.m_painters = &painters;

        pdfData.m_layout.margins().m_left = m_opts.m_left;
        pdfData.m_layout.margins().m_right = m_opts.m_right;
        pdfData.m_layout.margins().m_top = m_opts.m_top;
        pdfData.m_layout.margins().m_bottom = m_opts.m_bottom;
        pdfData.m_dpi = m_opts.m_dpi;
        pdfData.m_syntax = m_opts.m_syntax;
        pdfData.m_resvgOpts.reset(new ResvgOptions);
        pdfData.m_resvgOpts->setDpi(m_opts.m_dpi);
        pdfData.m_resvgOpts->loadSystemFonts();

        pdfData.m_colorsStack.push(Qt::black);

        pdfData.m_md = m_doc;

#ifdef MD_PDF_TESTING
        pdfData.m_fonts[QStringLiteral("Droid Serif")] = s_font;
        pdfData.m_fonts[QStringLiteral("Droid Serif Bold")] = s_boldFont;
        pdfData.m_fonts[QStringLiteral("Droid Serif Italic")] = s_italicFont;
        pdfData.m_fonts[QStringLiteral("Droid Serif Bold Italic")] = s_boldItalicFont;
        pdfData.m_fonts[QStringLiteral("Space Mono")] = s_monoFont;
        pdfData.m_fonts[QStringLiteral("Space Mono Italic")] = s_monoItalicFont;
        pdfData.m_fonts[QStringLiteral("Space Mono Bold")] = s_monoBoldFont;
        pdfData.m_fonts[QStringLiteral("Space Mono Bold Italic")] = s_monoBoldItalicFont;

        pdfData.m_self = this;

        if (m_opts.m_printDrawings) {
            pdfData.m_printDrawings = true;

            pdfData.m_drawingsFile.reset(new QFile(m_opts.m_testDataFileName));
            if (!pdfData.m_drawingsFile->open(QIODevice::WriteOnly)) {
                QFAIL("Unable to open file for dump drawings.");
            }

            pdfData.m_drawingsStream.reset(new QTextStream(pdfData.m_drawingsFile.get()));
        } else {
            pdfData.m_testData = m_opts.m_testData;
        }
#endif // MD_PDF_TESTING

        MD::forEach<MD::QStringTrait>(
            {MD::ItemType::Blockquote},
            m_doc,
            [&pdfData](MD::Item<MD::QStringTrait> *i) {
                auto b = static_cast<MD::Blockquote<MD::QStringTrait> *>(i);

                if (!b->items().empty() && b->items().front()->type() == MD::ItemType::Paragraph) {
                    auto p = static_cast<MD::Paragraph<MD::QStringTrait> *>(b->items().front().get());

                    if (!p->items().empty() && p->items().front()->type() == MD::ItemType::Text) {
                        auto t = static_cast<MD::Text<MD::QStringTrait> *>(p->items().front().get());

                        auto isAloneMark = [](MD::Paragraph<MD::QStringTrait> *p) -> bool {
                            return (p->items().size() > 1 ? p->items().at(1)->startLine() != p->items().front()->startLine() : true);
                        };

                        if (!t->opts()) {
                            QColor c;
                            bool highlight = false;
                            QString url;
                            QString text;

                            if (t->text() == QStringLiteral("[!NOTE]")) {
                                if (isAloneMark(p)) {
                                    c = QColor::fromString(QStringLiteral("#1f6feb"));
                                    highlight = true;
                                    url = QStringLiteral("qrc:/svg/note.svg");
                                    text = QStringLiteral(" Note");
                                }
                            } else if (t->text() == QStringLiteral("[!TIP]")) {
                                if (isAloneMark(p)) {
                                    c = QColor::fromString(QStringLiteral("#238636"));
                                    highlight = true;
                                    url = QStringLiteral("qrc:/svg/tip.svg");
                                    text = QStringLiteral(" Tip");
                                }
                            } else if (t->text() == QStringLiteral("[!WARNING]")) {
                                if (isAloneMark(p)) {
                                    c = QColor::fromString(QStringLiteral("#9e6a03"));
                                    highlight = true;
                                    url = QStringLiteral("qrc:/svg/warning.svg");
                                    text = QStringLiteral(" Warning");
                                }
                            } else if (t->text() == QStringLiteral("[!CAUTION]")) {
                                if (isAloneMark(p)) {
                                    c = QColor::fromString(QStringLiteral("#da3633"));
                                    highlight = true;
                                    url = QStringLiteral("qrc:/svg/caution.svg");
                                    text = QStringLiteral(" Caution");
                                }
                            } else if (t->text() == QStringLiteral("[!IMPORTANT]")) {
                                if (isAloneMark(p)) {
                                    c = QColor::fromString(QStringLiteral("#8250df"));
                                    highlight = true;
                                    url = QStringLiteral("qrc:/svg/important.svg");
                                    text = QStringLiteral(" Important");
                                }
                            }

                            if (highlight) {
                                p->removeItemAt(0);

                                if (p->items().empty()) {
                                    b->removeItemAt(0);
                                }

                                auto np = std::make_shared<MD::Paragraph<MD::QStringTrait>>();
                                np->setStartColumn(t->startColumn());
                                np->setStartLine(t->startLine());
                                np->setEndColumn(t->endColumn());
                                np->setEndLine(t->endLine());
                                auto i = std::make_shared<MD::Image<MD::QStringTrait>>();
                                i->setUrl(url);
                                i->setStartColumn(t->startColumn());
                                i->setStartLine(t->startLine());
                                i->setEndColumn(i->startColumn());
                                i->setEndLine(t->endLine());
                                np->appendItem(i);
                                auto nt = std::make_shared<MD::Text<MD::QStringTrait>>();
                                nt->setText(text);
                                nt->setStartColumn(t->startColumn() + 1);
                                nt->setStartLine(t->startLine());
                                nt->setEndColumn(t->endColumn());
                                nt->setEndLine(t->endLine());
                                np->appendItem(nt);

                                b->insertItem(0, np);

                                pdfData.m_highlightedBlockquotes.insert(b, c);
                            }
                        }
                    }
                }
            },
            1);

        int itemIdx = 0;

        pdfData.m_extraInFootnote =
            pdfData.lineSpacing(createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize,
                                           pdfData.m_doc, 1.0, pdfData), m_opts.m_textFontSize, 1.0)
            / 3.0;

        createPage(pdfData);

        for (auto it = m_doc->items().cbegin(), last = m_doc->items().cend(); it != last; ++it) {
            switch ((*it)->type()) {
            case MD::ItemType::Anchor:
                pdfData.m_anchors.push_back(static_cast<MD::Anchor<MD::QStringTrait> *>(it->get())->label());

            default:
                break;
            }
        }

        RTLFlag rtl;

        for (auto it = m_doc->items().cbegin(), last = m_doc->items().cend(); it != last; ++it) {
            ++itemIdx;

            {
                QMutexLocker lock(&m_mutex);

                if (m_terminate)
                    break;
            }

            switch ((*it)->type()) {
            case MD::ItemType::Heading:
                drawHeading(pdfData,
                            m_opts,
                            static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                            m_doc,
                            0.0,
                            // If there is another item after heading we need to know its min
                            // height to glue heading with it.
                            (it + 1 != last ? minNecessaryHeight(pdfData, m_opts, *(it + 1), m_doc, 0.0, 1.0) : 0.0),
                            CalcHeightOpt::Unknown,
                            1.0,
                            true, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::Paragraph:
                drawParagraph(pdfData, m_opts, static_cast<MD::Paragraph<MD::QStringTrait> *>(it->get()),
                              m_doc, 0.0, true, CalcHeightOpt::Unknown, 1.0, Qt::black, false, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::Code:
                drawCode(pdfData, m_opts, static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                         m_doc, 0.0, CalcHeightOpt::Unknown, 1.0);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::Blockquote:
                drawBlockquote(pdfData, m_opts, static_cast<MD::Blockquote<MD::QStringTrait> *>(it->get()),
                               m_doc, 0.0, CalcHeightOpt::Unknown, 1.0, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::List: {
                auto *list = static_cast<MD::List<MD::QStringTrait> *>(it->get());
                const auto bulletWidth = maxListNumberWidth(list);

                drawList(pdfData, m_opts, list, m_doc, bulletWidth, 0.0, CalcHeightOpt::Unknown, 1.0, false, &rtl);
                resetRTLFlagToDefaults(&rtl);
            } break;

            case MD::ItemType::Table:
                drawTable(pdfData, m_opts, static_cast<MD::Table<MD::QStringTrait> *>(it->get()),
                          m_doc, 0.0, CalcHeightOpt::Unknown, 1.0, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::PageBreak: {
                if (itemIdx < itemsCount)
                    createPage(pdfData);
            } break;

            case MD::ItemType::Anchor: {
                auto *a = static_cast<MD::Anchor<MD::QStringTrait> *>(it->get());
                auto dest = pdfData.m_doc->CreateDestination();
                dest->SetDestination(*pdfData.m_page, pdfData.m_layout.margins().m_left,
                                     pdfData.m_layout.topY(), 0.0);
                m_dests.insert(a->label(), std::move(dest));
                pdfData.m_currentFile = a->label();
            } break;

            default:
                break;
            }

            emit progress(static_cast<int>(static_cast<double>(itemIdx) / static_cast<double>(itemsCount) * 100.0));
        }

        if (!m_footnotes.isEmpty()) {
            pdfData.m_drawFootnotes = true;
            pdfData.m_layout.moveXToBegin();
            pdfData.m_layout.setY(pdfData.topFootnoteY(pdfData.m_reserved.firstKey()) - pdfData.m_extraInFootnote);

            pdfData.m_currentPainterIdx = pdfData.m_reserved.firstKey();
            pdfData.m_footnotePageIdx = pdfData.m_reserved.firstKey();

            drawHorizontalLine(pdfData, m_opts);

            for (const auto &f : std::as_const(m_footnotes)) {
                drawFootnote(pdfData, m_opts, m_doc, f.first, f.second.get(), CalcHeightOpt::Unknown);
            }
        }

        resolveLinks(pdfData);

        finishPages(pdfData);

        emit status(tr("Saving PDF..."));

        pdfData.save(m_fileName);

        emit done(m_terminate);

#ifdef MD_PDF_TESTING
        if (m_opts.m_printDrawings) {
            pdfData.m_drawingsFile->close();
        }

        if (pdfData.m_testPos != pdfData.m_testData.size()) {
            m_isError = true;
        }
#endif // MD_PDF_TESTING
    } catch (const PoDoFo::PdfError &e) {
        const auto &cs = e.GetCallStack();
        QString msg;

        for (const auto &i : cs) {
            auto filepath = i.GetFilePath();

            if (!filepath.empty()) {
                msg.append(QStringLiteral(" Error Source : "));
            }
            msg.append(QString::fromUtf8(filepath));
            msg.append(QStringLiteral(": "));
            msg.append(QString::number(i.GetLine()));
            msg.append(QStringLiteral("\n"));

            if (!i.GetInformation().empty()) {
                msg.append(QStringLiteral("Information: "));
            }
            msg.append(QString::fromUtf8(i.GetInformation()));
            msg.append(QStringLiteral("\n"));
        }

        handleException(pdfData, QString::fromLatin1("Error during drawing PDF:\n%1").arg(msg));
    } catch (const PdfRendererError &e) {
        handleException(pdfData, e.what());
    } catch (const std::exception &e) {
        handleException(pdfData, QString::fromLatin1("Error during drawing PDF: %1").arg(e.what()));
    } catch (...) {
        handleException(pdfData, QStringLiteral("Error during drawing PDF."));
    }

    try {
        clean();
    } catch (const std::exception &e) {
#ifdef MD_PDF_TESTING
        m_isError = true;
#endif
        emit error(QString::fromLatin1("Error freeing memory after all operations: %1").arg(e.what()));
    }

    deleteLater();
}

void PdfRenderer::handleException(PdfAuxData &pdfData, const QString &msg)
{
#ifdef MD_PDF_TESTING
    m_isError = true;
#endif

    const auto fullMsg = QStringLiteral(
                             "%1\n\nError occured in the \"%2\" file, "
                             "between start position %3 on line %4 and end position %5 on line %6.")
                             .arg(msg)
                             .arg(pdfData.m_currentFile)
                             .arg(pdfData.m_startPos + 1)
                             .arg(pdfData.m_startLine + 1)
                             .arg(pdfData.m_endPos + 1)
                             .arg(pdfData.m_endLine + 1);

    emit error(fullMsg);
}

void PdfRenderer::finishPages(PdfAuxData &pdfData)
{
    for (const auto &p : *pdfData.m_painters) {
        p->FinishDrawing();
    }
}

void PdfRenderer::clean()
{
    m_dests.clear();
    m_unresolvedLinks.clear();
    m_unresolvedFootnotesLinks.clear();
}

double PdfRenderer::rowHeight(const QVector<QVector<CellData>> &table, int row)
{
    double h = 0.0;

    for (auto it = table.cbegin(), last = table.cend(); it != last; ++it) {
        if ((*it)[row].m_height > h) {
            h = (*it)[row].m_height;
        }
    }

    return h;
}

void PdfRenderer::resolveLinks(PdfAuxData &pdfData)
{
    for (auto it = m_unresolvedLinks.cbegin(), last = m_unresolvedLinks.cend(); it != last; ++it) {
        if (m_dests.contains(it.key())) {
            for (const auto &r : std::as_const(it.value())) {
                auto &page = pdfData.m_doc->GetPages().GetPageAt(r.second);
                auto &annot = page.GetAnnotations().CreateAnnot<PoDoFo::PdfAnnotationLink>(
                            Rect(r.first.x(), r.first.y(), r.first.width(), r.first.height()));
                annot.SetBorderStyle(0.0, 0.0, 0.0);
                annot.SetDestination(*m_dests.value(it.key()).get());
            }
        }
#ifdef MD_PDF_TESTING
        else {
            terminate();

            QFAIL("Unresolved link.");
        }
#endif // MD_PDF_TESTING
    }

    for (auto it = m_unresolvedFootnotesLinks.cbegin(), last = m_unresolvedFootnotesLinks.cend(); it != last; ++it) {
        if (m_dests.contains(it.key())) {
            auto &page = pdfData.m_doc->GetPages().GetPageAt(it.value().second);
            auto &annot = page.GetAnnotations().CreateAnnot<PoDoFo::PdfAnnotationLink>(
                Rect(it.value().first.x(), it.value().first.y(), it.value().first.width(), it.value().first.height()));
            annot.SetBorderStyle(0.0, 0.0, 0.0);
            annot.SetDestination(*m_dests.value(it.key()).get());
        }
#ifdef MD_PDF_TESTING
        else {
            terminate();

            QFAIL("Unresolved footnote link.");
        }
#endif // MD_PDF_TESTING
    }
}

Font *PdfRenderer::createFont(const QString &name, bool bold, bool italic, double size,
                              Document *doc, double scale, const PdfAuxData &pdfData)
{
    PoDoFo::PdfFontSearchParams params;
    params.Style = PoDoFo::PdfFontStyle::Regular;
    if (bold) {
        params.Style.value() |= PoDoFo::PdfFontStyle::Bold;
    }
    if (italic) {
        params.Style.value() |= PoDoFo::PdfFontStyle::Italic;
    }

#ifdef MD_PDF_TESTING
    const QString internalName = name + (bold ? QStringLiteral(" Bold") :
                                                QString()) + (italic ? QStringLiteral(" Italic") : QString());

    auto &font = doc->GetFonts().GetOrCreateFont(pdfData.m_fonts[internalName].toLocal8Bit().data());

    return &font;
#else
    Q_UNUSED(pdfData)

    auto *font = doc->GetFonts().SearchFont(name.toLocal8Bit().data(), params);

    if (!font) {
        throw PdfRendererError(tr("Unable to create font: %1. Please choose another one.\n\n"
                                  "This application uses PoDoFo C++ library to create PDF. And not all fonts supported by Qt "
                                  "are supported by PoDoFo. I'm sorry for the inconvenience.")
                                   .arg(name));
    }

    return font;
#endif // MD_PDF_TESTING
}

namespace /* anonymous */
{

inline bool isGoodWidth(double w)
{
    return (!std::isinf(w) && !std::isnan(w) && w > 0.0);
}

} /* namespace anonymous */

bool PdfRenderer::isFontCreatable(const QString &name, bool monospace)
{
    Document doc;

    PoDoFo::PdfFontSearchParams params;
    params.Style = PoDoFo::PdfFontStyle::Regular;

    auto font = doc.GetFonts().SearchFont(name.toLocal8Bit().data(), params);

    if (font) {
        PdfAuxData pdfData;

        const auto w1 = pdfData.stringWidth(font, 10.0, 1.0, createUtf8String(" "));
        const auto w2 = pdfData.stringWidth(font, 10.0, 1.0, createUtf8String("A"));
        const auto w3 = pdfData.stringWidth(font, 10.0, 1.0, createUtf8String("ABC"));

        if (isGoodWidth(w1) && isGoodWidth(w2) && isGoodWidth(w3)) {
            if (monospace) {
                if (std::abs(w1 - w2) > 0.001 || std::abs(w1 * 3.0 - w3) > 0.001) {
                    return false;
                } else {
                    return true;
                }
            } else {
                return true;
            }

        } else {
            return false;
        }
    } else {
        return false;
    }
}

void PdfRenderer::createPage(PdfAuxData &pdfData)
{
    std::function<void(PdfAuxData &)> create;

    create = [&create](PdfAuxData &pdfData) {
        pdfData.m_page = &pdfData.m_doc->GetPages().CreatePage(Page::CreateStandardPageSize(PoDoFo::PdfPageSize::A4));

        if (!pdfData.m_page) {
            throw PdfRendererError(
                QLatin1String("Oops, can't create empty page in PDF.\n\n"
                              "This is very strange, it should not appear ever, but it is. "
                              "I'm sorry for the inconvenience."));
        }

        pdfData.m_firstOnPage = true;

        auto painter = std::make_shared<Painter>();
        painter->SetCanvas(*pdfData.m_page);

        (*pdfData.m_painters).push_back(painter);
        pdfData.m_currentPainterIdx = pdfData.m_painters->size() - 1;

        pdfData.m_layout.m_coords.m_pageWidth = pdfData.m_page->GetRect().Width;
        pdfData.m_layout.m_coords.m_pageHeight = pdfData.m_page->GetRect().Height;
        pdfData.m_layout.moveXToBegin();
        pdfData.m_layout.setY(pdfData.m_layout.topY());

        ++pdfData.m_currentPageIdx;

        const auto topY = pdfData.topFootnoteY(pdfData.m_currentPageIdx);

        if (pdfData.m_layout.topY() - topY - pdfData.m_extraInFootnote < pdfData.m_lineHeight) {
            create(pdfData);
        }
    };

    if (!pdfData.m_drawFootnotes) {
        create(pdfData);
    } else {
        auto it = pdfData.m_reserved.find(++pdfData.m_footnotePageIdx);

        if (it == pdfData.m_reserved.cend()) {
            it = pdfData.m_reserved.upperBound(pdfData.m_footnotePageIdx);
        }

        if (it != pdfData.m_reserved.cend()) {
            pdfData.m_footnotePageIdx = it.key();
        } else {
            pdfData.m_footnotePageIdx = pdfData.m_currentPageIdx + 1;
        }

        if (pdfData.m_footnotePageIdx <= pdfData.m_currentPageIdx) {
            pdfData.m_currentPainterIdx = pdfData.m_footnotePageIdx;
            pdfData.m_layout.moveXToBegin();
            pdfData.m_layout.setY(pdfData.topFootnoteY(pdfData.m_footnotePageIdx));
        } else {
            create(pdfData);

            pdfData.m_layout.setY(pdfData.topFootnoteY(pdfData.m_footnotePageIdx));
        }

        pdfData.m_layout.addY(pdfData.m_extraInFootnote);

        drawHorizontalLine(pdfData, m_opts);

        if (pdfData.m_continueParagraph) {
            pdfData.m_layout.addY(pdfData.m_lineHeight);
        }
    }

    if (pdfData.m_colorsStack.size() > 1) {
        pdfData.repeatColor();
    }
}

void PdfRenderer::drawHorizontalLine(PdfAuxData &pdfData, const RenderOpts &renderOpts)
{
    pdfData.setColor(renderOpts.m_borderColor);
    pdfData.drawLine(pdfData.m_layout.margins().m_left, pdfData.m_layout.y(),
                     pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_right, pdfData.m_layout.y());
    pdfData.restoreColor();
}

Utf8String PdfRenderer::createUtf8String(const QString &text)
{
    return {text.toUtf8()};
}

QString PdfRenderer::createQString(const char *str)
{
    return QString::fromUtf8(str, -1);
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawHeading(PdfAuxData &pdfData,
                                                                const RenderOpts &renderOpts,
                                                                MD::Heading<MD::QStringTrait> *item,
                                                                std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                double offset,
                                                                double nextItemMinHeight,
                                                                CalcHeightOpt heightCalcOpt,
                                                                double scale,
                                                                bool withNewLine,
                                                                RTLFlag *rtl)
{
    Q_UNUSED(nextItemMinHeight)

    if (item && item->text().get()) {
        pdfData.m_startLine = item->startLine();
        pdfData.m_startPos = item->startColumn();
        pdfData.m_endLine = item->endLine();
        pdfData.m_endPos = item->endColumn();

        const auto where =
            drawParagraph(pdfData, renderOpts, item->text().get(), doc, offset, withNewLine,
                          heightCalcOpt, scale * (1.0 + (7 - item->level()) * 0.25), Qt::black, false, rtl);

        if (heightCalcOpt == CalcHeightOpt::Unknown && !item->label().isEmpty() && !where.first.isEmpty()) {
            auto dest = pdfData.m_doc->CreateDestination();
            dest->SetDestination(pdfData.m_doc->GetPages().GetPageAt(static_cast<unsigned int>(where.first.front().m_pageIdx)),
                                 pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() * offset,
                                 where.first.front().m_y + where.first.front().m_height,
                                 0.0);
            m_dests.insert(item->label(), std::move(dest));
        }

        return where;
    } else {
        return {};
    }
}

QVector<QPair<QRectF, unsigned int>> PdfRenderer::drawText(PdfAuxData &pdfData,
                                                           const RenderOpts &renderOpts,
                                                           MD::Text<MD::QStringTrait> *item,
                                                           std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                           bool &newLine,
                                                           Font *footnoteFont,
                                                           double footnoteFontSize,
                                                           double footnoteFontScale,
                                                           MD::Item<MD::QStringTrait> *nextItem,
                                                           int footnoteNum,
                                                           double offset,
                                                           bool firstInParagraph,
                                                           CustomWidth &cw,
                                                           double scale,
                                                           const QColor &color,
                                                           RTLFlag *rtl)
{
    auto *spaceFont = createFont(renderOpts.m_textFont, false, false,
                                 renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    auto *font = createFont(renderOpts.m_textFont,
                            item->opts() & MD::TextOption::BoldText,
                            item->opts() & MD::TextOption::ItalicText,
                            renderOpts.m_textFontSize,
                            pdfData.m_doc,
                            scale,
                            pdfData);

    return drawString(pdfData,
                      renderOpts,
                      item->text(),
                      spaceFont,
                      renderOpts.m_textFontSize,
                      scale,
                      font,
                      renderOpts.m_textFontSize,
                      scale,
                      pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale),
                      doc,
                      newLine,
                      footnoteFont,
                      footnoteFontSize,
                      footnoteFontScale,
                      nextItem,
                      footnoteNum,
                      offset,
                      firstInParagraph,
                      cw,
                      QColor(),
                      item->opts() & MD::TextOption::StrikethroughText,
                      item->startLine(),
                      item->startColumn(),
                      item->endLine(),
                      item->endColumn(),
                      color,
                      nullptr,
                      0.0,
                      0.0,
                      rtl);
}

namespace /* anonymous */
{

//! Combine smaller rectangles standing next each other to bigger one.
QVector<QPair<QRectF, unsigned int>> normalizeRects(const QVector<QPair<QRectF, unsigned int>> &rects)
{
    QVector<QPair<QRectF, unsigned int>> ret;

    if (!rects.isEmpty()) {
        QPair<QRectF, int> to(rects.first());

        auto it = rects.cbegin();
        ++it;

        for (auto last = rects.cend(); it != last; ++it) {
            if (qAbs(it->first.y() - to.first.y()) < 0.001) {
                to.first.setWidth(to.first.width() + it->first.width());
            } else {
                ret.append(to);

                to = *it;
            }
        }

        ret.append(to);
    }

    return ret;
}

} /* namespace anonymous */

QVector<QPair<QRectF, unsigned int>> PdfRenderer::drawLink(PdfAuxData &pdfData,
                                                           const RenderOpts &renderOpts,
                                                           MD::Link<MD::QStringTrait> *item,
                                                           std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                           bool &newLine,
                                                           Font *footnoteFont,
                                                           double footnoteFontSize,
                                                           double footnoteFontScale,
                                                           MD::Item<MD::QStringTrait> *nextItem,
                                                           int footnoteNum,
                                                           double offset,
                                                           bool firstInParagraph,
                                                           CustomWidth &cw,
                                                           double scale,
                                                           RTLFlag *rtl)
{
    QVector<QPair<QRectF, unsigned int>> rects;

    QString url = item->url();

    const auto lit = doc->labeledLinks().find(url);

    if (lit != doc->labeledLinks().cend()) {
        url = lit->second->url();
    }

    bool draw = true;

    if (!cw.isDrawing()) {
        draw = false;
    }

    auto *font = createFont(renderOpts.m_textFont,
                            item->opts() & MD::TextOption::BoldText,
                            item->opts() & MD::TextOption::ItalicText,
                            renderOpts.m_textFontSize,
                            pdfData.m_doc,
                            scale,
                            pdfData);

    if (!item->p()->isEmpty()) {
        for (auto it = item->p()->items().begin(), last = item->p()->items().end(); it != last; ++it) {
            switch ((*it)->type()) {
            case MD::ItemType::Text: {
                auto *text = std::static_pointer_cast<MD::Text<MD::QStringTrait>>(*it).get();

                auto *spaceFont = createFont(renderOpts.m_textFont, false, false,
                                             renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

                auto *font = createFont(renderOpts.m_textFont,
                                        text->opts() & MD::BoldText || item->opts() & MD::BoldText,
                                        text->opts() & MD::ItalicText || item->opts() & MD::ItalicText,
                                        renderOpts.m_textFontSize,
                                        pdfData.m_doc,
                                        scale,
                                        pdfData);

                rects.append(drawString(pdfData,
                                        renderOpts,
                                        text->text(),
                                        spaceFont,
                                        renderOpts.m_textFontSize,
                                        scale,
                                        font,
                                        renderOpts.m_textFontSize,
                                        scale,
                                        pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale),
                                        doc,
                                        newLine,
                                        footnoteFont,
                                        footnoteFontSize,
                                        footnoteFontScale,
                                        (it == std::prev(last) ? nextItem : nullptr),
                                        footnoteNum,
                                        offset,
                                        (it == item->p()->items().begin() && firstInParagraph),
                                        cw,
                                        QColor(),
                                        text->opts() & MD::StrikethroughText || item->opts() & MD::StrikethroughText,
                                        text->startLine(),
                                        text->startColumn(),
                                        text->endLine(),
                                        text->endColumn(),
                                        renderOpts.m_linkColor,
                                        nullptr,
                                        0.0,
                                        0.0,
                                        rtl));
            } break;

            case MD::ItemType::Code: {
                rects.append(drawInlinedCode(pdfData,
                                             renderOpts,
                                             static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                                             doc,
                                             newLine,
                                             offset,
                                             (it == item->p()->items().begin() && firstInParagraph),
                                             cw,
                                             scale,
                                             renderOpts.m_linkColor,
                                             rtl));

                setRTLFlagToFalseIfCheck(rtl);
            }
                break;

            case MD::ItemType::Image: {
                rects.append(drawImage(pdfData,
                                       renderOpts,
                                       static_cast<MD::Image<MD::QStringTrait> *>(it->get()),
                                       doc,
                                       newLine,
                                       offset,
                                       (it == item->p()->items().begin() && firstInParagraph),
                                       cw,
                                       1.0,
                                       renderOpts.m_imageAlignment));

                setRTLFlagToFalseIfCheck(rtl);
            } break;

            default:
                break;
            }
        }
    } else if (item->img()->isEmpty()) {
        auto *spaceFont = createFont(renderOpts.m_textFont, false, false,
                                     renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

        rects = drawString(pdfData,
                           renderOpts,
                           url,
                           spaceFont,
                           renderOpts.m_textFontSize,
                           scale,
                           font,
                           renderOpts.m_textFontSize,
                           scale,
                           pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale),
                           doc,
                           newLine,
                           footnoteFont,
                           footnoteFontSize,
                           footnoteFontScale,
                           nextItem,
                           footnoteNum,
                           offset,
                           firstInParagraph,
                           cw,
                           QColor(),
                           item->opts() & MD::TextOption::StrikethroughText,
                           item->startLine(),
                           item->startColumn(),
                           item->endLine(),
                           item->endColumn(),
                           renderOpts.m_linkColor,
                           nullptr,
                           0.0,
                           0.0,
                           rtl);
    }
    // Otherwise image link.
    else {
        rects.append(drawImage(pdfData, renderOpts, item->img().get(), doc, newLine, offset,
                               firstInParagraph, cw, 1.0, renderOpts.m_imageAlignment));

        setRTLFlagToFalseIfCheck(rtl);
    }

    rects = normalizeRects(rects);

    if (draw) {
        // If Web URL.
        if (!pdfData.m_anchors.contains(url) && pdfData.m_md->labeledHeadings().find(url) == pdfData.m_md->labeledHeadings().cend()) {
            for (const auto &r : std::as_const(rects)) {
                auto &annot = pdfData.m_doc->GetPages()
                                  .GetPageAt(static_cast<unsigned int>(r.second))
                                  .GetAnnotations()
                                  .CreateAnnot<PoDoFo::PdfAnnotationLink>(Rect(r.first.x(), r.first.y(), r.first.width(), r.first.height()));
                annot.SetBorderStyle(0.0, 0.0, 0.0);

                auto action = pdfData.m_doc->CreateAction<PoDoFo::PdfActionURI>();
                action->SetURI(PoDoFo::PdfString(std::string(url.toLatin1().data())));

                annot.SetAction(*action.get());
            }
        }
        // Otherwise internal link.
        else {
            m_unresolvedLinks.insert(url, rects);
        }
    }

    return rects;
}

namespace /* anonymous */
{

inline bool isRightToLeft(const QVector<QPair<QString, bool>> &words)
{
    for (const auto & w : std::as_const(words)) {
        if (w.first != QStringLiteral(" ")) {
            return w.second;
        }
    }

    return false;
}

} /* namespace anonymous */

QVector<QPair<QRectF, unsigned int>> PdfRenderer::drawString(PdfAuxData &pdfData,
                                                             const RenderOpts &renderOpts,
                                                             const QString &str,
                                                             Font *spaceFont,
                                                             double spaceFontSize,
                                                             double spaceFontScale,
                                                             Font *font,
                                                             double fontSize,
                                                             double fontScale,
                                                             double lineHeight,
                                                             std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                             bool &newLine,
                                                             Font *footnoteFont,
                                                             double footnoteFontSize,
                                                             double footnoteFontScale,
                                                             MD::Item<MD::QStringTrait> *nextItem,
                                                             int footnoteNum,
                                                             double offset,
                                                             bool firstInParagraph,
                                                             CustomWidth &cw,
                                                             const QColor &background,
                                                             bool strikeout,
                                                             long long int startLine,
                                                             long long int startPos,
                                                             long long int endLine,
                                                             long long int endPos,
                                                             const QColor &color,
                                                             Font *regularSpaceFont,
                                                             double regularSpaceFontSize,
                                                             double regularSpaceFontScale,
                                                             RTLFlag *rtl)
{
    Q_UNUSED(doc)
    Q_UNUSED(renderOpts)

    const bool onNewMdLine = pdfData.m_endLine != startLine;

    pdfData.m_startLine = startLine;
    pdfData.m_startPos = startPos;
    pdfData.m_endLine = endLine;
    pdfData.m_endPos = endPos;

    bool draw = true;

    if (!cw.isDrawing()) {
        draw = false;
    }

    bool footnoteAtEnd = false;
    double footnoteWidth = 0.0;

    if (nextItem && nextItem->type() == MD::ItemType::FootnoteRef
        && doc->footnotesMap().find(static_cast<MD::FootnoteRef<MD::QStringTrait> *>(nextItem)->id()) != doc->footnotesMap().cend()) {
        footnoteAtEnd = true;
    }

    if (footnoteAtEnd) {
        footnoteWidth = pdfData.stringWidth(footnoteFont, footnoteFontSize,
                                            footnoteFontScale, createUtf8String(QString::number(footnoteNum)));
    }

    double h = lineHeight;
    const double actualLineHeight = pdfData.lineSpacing(font, fontSize, fontScale);
    const double d = -pdfData.fontDescent(font, fontSize, fontScale) + (h - actualLineHeight) / 2.0;

    if (cw.isDrawing()) {
        h = cw.height();
    }

    auto newLineFn = [&]() {
        newLine = true;

        if (draw) {
            cw.moveToNextLine();

            moveToNewLine(pdfData, offset, cw.height(), 1.0, cw.height());

            h = cw.height();
        } else {
            cw.append({0.0, lineHeight, false, true, true, ""});
            pdfData.m_layout.moveXToBegin();
        }
    };

    QVector<QPair<QRectF, unsigned int>> ret;

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate)
            return ret;
    }

    if (rtl && rtl->isCheck()) {
        pdfData.m_layout.setRightToLeft(str.isRightToLeft());
        rtl->m_check = false;
        rtl->m_isOn = pdfData.m_layout.isRightToLeft();
    } else if (rtl) {
        pdfData.m_layout.setRightToLeft(rtl->isRightToLeft());
    }

    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

    auto words = splitString(str, false);

    if (pdfData.m_layout.isRightToLeft()) {
        orderWords(words);
    }

    const auto fullWidth = pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
            pdfData.m_layout.margins().m_right;

    const auto spaceWidth = pdfData.stringWidth(spaceFont, spaceFontSize, spaceFontScale, " ");

    bool firstSpaceDrawn = false;

    auto drawSpace = [&](bool useRegularSpace) {
        firstSpaceDrawn = true;
        auto scale = 100.0;

        if (draw) {
            scale = cw.scale();
        }

        const auto currentSpaceWidth = (useRegularSpace && regularSpaceFont ?
            pdfData.stringWidth(regularSpaceFont, regularSpaceFontSize, regularSpaceFontScale, " ") :
            spaceWidth);

        const auto width = currentSpaceWidth * scale / 100.0;

        if (pdfData.m_layout.isFit(width)) {
            newLine = false;

            if (draw) {
                ret.append(qMakePair(pdfData.m_layout.currentRect(width, lineHeight),
                                     pdfData.currentPageIndex()));

                if (background.isValid() &&!useRegularSpace) {
                    pdfData.setColor(background);
                    pdfData.drawRectangle(pdfData.m_layout.startX(width),
                                          pdfData.m_layout.y() + pdfData.fontDescent(font, fontSize, fontScale) + d,
                                          width,
                                          pdfData.lineSpacing(font, fontSize, fontScale),
                                          PoDoFo::PdfPathDrawMode::Fill);
                    pdfData.restoreColor();
                }

                Font *font = (useRegularSpace && regularSpaceFont ? regularSpaceFont : spaceFont);
                const auto size = (useRegularSpace && regularSpaceFont ? regularSpaceFontSize * regularSpaceFontScale :
                                                                         spaceFontSize * spaceFontScale);
                pdfData.drawText(pdfData.m_layout.startX(width), pdfData.m_layout.y() + d, " ",
                                 font, size, scale / 100.0, strikeout);
            } else {
                cw.append({currentSpaceWidth, lineHeight, true, false, true, " "});
            }

            pdfData.m_layout.addX(width);
        }
    }; // drawSpace

    auto countCharsForAvailableSpace =
        [](const QString &s, double availableWidth, Font *font, const PdfAuxData &pdfData,
            double fontSize, double fontScale, QString &tmp) -> qsizetype {
        qsizetype i = 0;

        for (; i < s.length(); ++i) {
            tmp.push_back(s[i]);
            const auto l = pdfData.stringWidth(font, fontSize, fontScale, createUtf8String(tmp));

            if (l > availableWidth && !(qAbs(l - availableWidth) < 0.01)) {
                tmp.removeLast();

                --i;

                break;
            }
        }

        return (i < s.length() ? ++i : i);
    }; // countCharsForAvailableSpace

    // Draw words.
    for (auto it = words.begin(), last = words.end(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return ret;
            }
        }

        const auto str = createUtf8String(it->first);

        if (it->first == QStringLiteral(" ")) {
            if (!newLine) {
                drawSpace(false);
            }
        } else {
            if (onNewMdLine && !firstSpaceDrawn && !firstInParagraph && !newLine) {
                drawSpace(true);
                --it;
                continue;
            }

            const auto length = pdfData.stringWidth(font, fontSize, fontScale, str);

            const auto width = length + (it + 1 == last && footnoteAtEnd ? footnoteWidth : 0.0);

            if (pdfData.m_layout.isFit(width)) {
                newLine = false;

                if (draw) {
                    if (background.isValid()) {
                        pdfData.setColor(background);
                        pdfData.drawRectangle(pdfData.m_layout.startX(length),
                                              pdfData.m_layout.y() + pdfData.fontDescent(font, fontSize, fontScale) + d,
                                              length,
                                              pdfData.lineSpacing(font, fontSize, fontScale),
                                              PoDoFo::PdfPathDrawMode::Fill);
                        pdfData.restoreColor();
                    }

                    pdfData.setColor(color);

                    if (it->second) {
                        std::reverse(it->first.begin(), it->first.end());
                    }

                    pdfData.drawText(pdfData.m_layout.startX(length), pdfData.m_layout.y() + d,
                                     createUtf8String(it->first),
                                     font, fontSize * fontScale, 1.0, strikeout);
                    pdfData.restoreColor();

                    ret.append(qMakePair(pdfData.m_layout.currentRect(length, lineHeight), pdfData.currentPageIndex()));
                } else {
                    cw.append({length + (it + 1 == last && footnoteAtEnd ? footnoteWidth : 0.0), lineHeight, false, false, true, it->first});
                }

                pdfData.m_layout.addX(length);
            }
            // Need to move to new line.
            else {
                auto splitAndDraw = [&](QString s, bool rtl) {
                    while (s.length()) {
                        QString tmp;
                        const auto i = countCharsForAvailableSpace(s, pdfData.m_layout.availableWidth(),
                                                                   font, pdfData, fontSize, fontScale, tmp);

                        s.remove(0, i);

                        const auto w = pdfData.stringWidth(font, fontSize, fontScale, createUtf8String(tmp));

                        if (draw) {
                            pdfData.setColor(color);

                            if (pdfData.m_layout.isRightToLeft()) {
                                std::reverse(tmp.begin(), tmp.end());
                            }

                            pdfData.drawText(pdfData.m_layout.startX(w), pdfData.m_layout.y() + d, createUtf8String(tmp),
                                             font, fontSize * fontScale, 1.0, strikeout);
                            pdfData.restoreColor();

                            ret.append(qMakePair(pdfData.m_layout.currentRect(w, lineHeight), pdfData.currentPageIndex()));
                        } else {
                            cw.append({w, lineHeight, false, false, true, tmp});
                        }

                        newLine = false;

                        pdfData.m_layout.addX(w);

                        if (s.length()) {
                            newLineFn();
                        }
                    }

                    if (!draw && it + 1 == last && footnoteAtEnd) {
                        cw.append({footnoteWidth, lineHeight, false, false, true, QString::number(footnoteNum)});
                    }
                }; // splitAndDraw

                if (width > fullWidth * 2.0 / 3.0) {
                    QString tmp;

                    if (countCharsForAvailableSpace(it->first, pdfData.m_layout.availableWidth(),
                                                    font, pdfData, fontSize, fontScale, tmp) > 4) {
                        splitAndDraw(it->first, it->second);
                    } else {
                        if (width < fullWidth || qAbs(width - fullWidth) < 0.01) {
                            newLineFn();

                            --it;
                        } else {
                            newLineFn();

                            splitAndDraw(it->first, it->second);
                        }
                    }
                } else {
                    newLineFn();

                    --it;
                }
            }
        }
    }

    return ret;
}

QVector<QPair<QRectF, unsigned int>> PdfRenderer::drawInlinedCode(PdfAuxData &pdfData,
                                                                  const RenderOpts &renderOpts,
                                                                  MD::Code<MD::QStringTrait> *item,
                                                                  std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                  bool &newLine,
                                                                  double offset,
                                                                  bool firstInParagraph,
                                                                  CustomWidth &cw,
                                                                  double scale,
                                                                  const QColor &color,
                                                                  RTLFlag *rtl)
{
    Q_UNUSED(rtl)

    auto *textFont = createFont(renderOpts.m_textFont, false, false,
                                renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    auto *font = createFont(renderOpts.m_codeFont,
                            item->opts() & MD::TextOption::BoldText,
                            item->opts() & MD::TextOption::ItalicText,
                            renderOpts.m_codeFontSize,
                            pdfData.m_doc,
                            scale,
                            pdfData);

    return drawString(pdfData,
                      renderOpts,
                      item->text(),
                      font,
                      renderOpts.m_codeFontSize,
                      scale,
                      font,
                      renderOpts.m_codeFontSize,
                      scale,
                      pdfData.lineSpacing(textFont, renderOpts.m_textFontSize, scale),
                      doc,
                      newLine,
                      nullptr,
                      0.0,
                      0.0,
                      nullptr,
                      m_footnoteNum,
                      offset,
                      firstInParagraph,
                      cw,
                      renderOpts.m_syntax->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding),
                      item->opts() & MD::TextOption::StrikethroughText,
                      item->startLine(),
                      item->startColumn(),
                      item->endLine(),
                      item->endColumn(),
                      color,
                      textFont,
                      renderOpts.m_textFontSize,
                      scale,
                      rtl);
}

void PdfRenderer::moveToNewLine(PdfAuxData &pdfData, double xOffset, double yOffset, double yOffsetMultiplier, double yOffsetOnNewPage)
{
    Q_UNUSED(xOffset)

    pdfData.m_layout.moveXToBegin();
    pdfData.m_layout.addY(yOffset * yOffsetMultiplier);

    if (pdfData.m_layout.y() < pdfData.currentPageAllowedY() && qAbs(pdfData.m_layout.y() - pdfData.currentPageAllowedY()) > 0.1) {
        createPage(pdfData);

        pdfData.m_layout.moveXToBegin();
        pdfData.m_layout.addY(yOffsetOnNewPage);
    }
}

namespace /* anonymous */
{

QVector<WhereDrawn> toWhereDrawn(const QVector<QPair<QRectF, unsigned int>> &rects, double pageHeight)
{
    struct AuxData {
        double m_minY = 0.0;
        double m_maxY = 0.0;
    }; // struct AuxData

    QMap<int, AuxData> map;

    for (const auto &r : rects) {
        if (!map.contains(r.second)) {
            map[r.second] = {pageHeight, 0.0};
        }

        if (r.first.y() < map[r.second].m_minY) {
            map[r.second].m_minY = r.first.y();
        }

        if (r.first.height() + r.first.y() > map[r.second].m_maxY) {
            map[r.second].m_maxY = r.first.height() + r.first.y();
        }
    }

    QVector<WhereDrawn> ret;

    for (auto it = map.cbegin(), last = map.cend(); it != last; ++it) {
        ret.append({it.key(), it.value().m_minY, it.value().m_maxY - it.value().m_minY});
    }

    return ret;
}

double totalHeight(const QVector<WhereDrawn> &where)
{
    return std::accumulate(where.cbegin(), where.cend(), 0.0, [](const double &val, const WhereDrawn &cur) -> double {
        return (val + cur.m_height);
    });
}

bool isRightToLeft(MD::Item<MD::QStringTrait> *i)
{
    auto isTextRightToLeft = [](MD::Item<MD::QStringTrait> *i) -> bool {
        return static_cast<MD::Text<MD::QStringTrait>*>(i)->text().isRightToLeft();
    };

    switch (i->type() ) {
    case MD::ItemType::Text:
        return isTextRightToLeft(i);

    case MD::ItemType::Link:
    {
        auto l = static_cast<MD::Link<MD::QStringTrait>*>(i);

        if (!l->p()->isEmpty()) {
            if (l->p()->items().front()->type() == MD::ItemType::Text) {
                return isTextRightToLeft(l->p()->items().front().get());
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
        break;

    default:
        return false;
    }
}

bool isRightToLeft(MD::Paragraph<MD::QStringTrait> *p)
{
    if (!p->isEmpty()) {
        return isRightToLeft(p->items().front().get());
    } else {
        return false;
    }
}

} /* namespace anonymous */

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawParagraph(PdfAuxData &pdfData,
                                                                  const RenderOpts &renderOpts,
                                                                  MD::Paragraph<MD::QStringTrait> *item,
                                                                  std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                  double offset,
                                                                  bool withNewLine,
                                                                  CalcHeightOpt heightCalcOpt,
                                                                  double scale,
                                                                  const QColor &color,
                                                                  bool scaleImagesToLineHeight,
                                                                  RTLFlag *rtl)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    const auto wasRightToLeft = pdfData.m_layout.isRightToLeft();

    QVector<QPair<QRectF, unsigned int>> rects;

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate) {
            return {};
        }
    }

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        emit status(tr("Drawing paragraph."));
    }

    auto *font = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    auto *footnoteFont = font;

    const auto lineHeight = pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale);

    pdfData.m_lineHeight = lineHeight;

    const auto isParagraphRightToLeft = isRightToLeft(item);
    const auto rightToLeft = (rtl && !rtl->isCheck() ? rtl->isRightToLeft() : isParagraphRightToLeft);
    const auto autoOffset = pdfData.m_layout.addOffset(offset, !rightToLeft);

    pdfData.m_layout.setRightToLeft(isParagraphRightToLeft);
    pdfData.m_layout.moveXToBegin();

    bool newLine = false;
    CustomWidth cw;

    bool lineBreak = false;
    bool firstInParagraph = true;

    // Calculate words/lines/spaces widthes.
    for (auto it = item->items().begin(), last = item->items().end(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        int nextFootnoteNum = m_footnoteNum;

        if (it + 1 != last && (it + 1)->get()->type() == MD::ItemType::FootnoteRef) {
            auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>((it + 1)->get());

            const auto fit = doc->footnotesMap().find(ref->id());

            if (fit != doc->footnotesMap().cend()) {
                auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());

                if (anchorIt != pdfData.m_footnotesAnchorsMap.cend()) {
                    nextFootnoteNum = anchorIt->second;
                }
            }
        }

        switch ((*it)->type()) {
        case MD::ItemType::Text: {
            auto text = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

            drawText(pdfData,
                     renderOpts,
                     text,
                     doc,
                     newLine,
                     footnoteFont,
                     renderOpts.m_textFontSize * scale,
                     s_footnoteScale,
                     (it + 1 != last ? (it + 1)->get() : nullptr),
                     nextFootnoteNum,
                     offset,
                     (firstInParagraph || lineBreak),
                     cw,
                     scale,
                     color,
                     rtl);
            lineBreak = false;
            firstInParagraph = false;
        } break;

        case MD::ItemType::Code:
            drawInlinedCode(pdfData,
                            renderOpts,
                            static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                            doc,
                            newLine,
                            offset,
                            (firstInParagraph || lineBreak),
                            cw,
                            scale,
                            Qt::black,
                            rtl);
            lineBreak = false;
            firstInParagraph = false;
            break;

        case MD::ItemType::Link:
            drawLink(pdfData,
                     renderOpts,
                     static_cast<MD::Link<MD::QStringTrait> *>(it->get()),
                     doc,
                     newLine,
                     footnoteFont,
                     renderOpts.m_textFontSize * scale,
                     s_footnoteScale,
                     (it + 1 != last ? (it + 1)->get() : nullptr),
                     nextFootnoteNum,
                     offset,
                     (firstInParagraph || lineBreak),
                     cw,
                     scale,
                     rtl);
            lineBreak = false;
            firstInParagraph = false;
            break;

        case MD::ItemType::Image:
            drawImage(pdfData,
                      renderOpts,
                      static_cast<MD::Image<MD::QStringTrait> *>(it->get()),
                      doc,
                      newLine,
                      offset,
                      (firstInParagraph || lineBreak),
                      cw,
                      1.0,
                      renderOpts.m_imageAlignment,
                      scaleImagesToLineHeight);
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
            break;

        case MD::ItemType::Math:
            drawMathExpr(pdfData,
                         renderOpts,
                         static_cast<MD::Math<MD::QStringTrait> *>(it->get()),
                         doc,
                         newLine,
                         offset,
                         (std::next(it) != last),
                         (firstInParagraph || lineBreak),
                         cw,
                         scale);
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
            break;

        case MD::ItemType::LineBreak: {
            lineBreak = true;
            cw.append({0.0, lineHeight, false, true, false, ""});
            pdfData.m_layout.moveXToBegin();
        } break;

        case MD::ItemType::FootnoteRef: {
            auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>(it->get());

            const auto fit = doc->footnotesMap().find(ref->id());

            if (fit != doc->footnotesMap().cend()) {
                auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());

                if (anchorIt == pdfData.m_footnotesAnchorsMap.cend()) {
                    pdfData.m_footnotesAnchorsMap.insert(fit->second.get(), {pdfData.m_currentFile, m_footnoteNum++});
                }
            } else {
                auto text = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

                drawText(pdfData,
                         renderOpts,
                         text,
                         doc,
                         newLine,
                         footnoteFont,
                         renderOpts.m_textFontSize * scale,
                         s_footnoteScale,
                         (it + 1 != last ? (it + 1)->get() : nullptr),
                         nextFootnoteNum,
                         offset,
                         (firstInParagraph || lineBreak),
                         cw,
                         scale,
                         color);
            }

            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        default:
            break;
        }
    }

    if (!cw.isNewLineAtEnd()) {
        cw.append({0.0, lineHeight, false, true, false, ""});
    }

    cw.calcScale(pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
                 pdfData.m_layout.margins().m_right - offset);

    cw.setDrawing();

    switch (heightCalcOpt) {
    case CalcHeightOpt::Minimum: {
        QVector<WhereDrawn> r;
        r.append(
            {-1,
             0.0,
             ((withNewLine && !pdfData.m_firstOnPage) || (withNewLine && pdfData.m_drawFootnotes) ?
                lineHeight + cw.firstItemHeight() : cw.firstItemHeight())});

        return {r, {}};
    }

    case CalcHeightOpt::Full: {
        QVector<WhereDrawn> r;

        double h = 0.0;
        double max = 0.0;

        for (auto it = cw.cbegin(), last = cw.cend(); it != last; ++it) {
            if (it == cw.cbegin() && ((withNewLine && !pdfData.m_firstOnPage) || (withNewLine && pdfData.m_drawFootnotes))) {
                h += lineHeight;
            }

            if (h + it->m_height > max) {
                max = h + it->m_height;
            }

            if (it->m_isNewLine) {
                r.append({-1, 0.0, max});
                max = 0.0;
                h = 0.0;
            }
        }

        return {r, {}};
    }

    default:
        break;
    }

    if (rtl) {
        pdfData.m_layout.setRightToLeft(rtl->isRightToLeft());
    }

    if ((withNewLine && !pdfData.m_firstOnPage && heightCalcOpt == CalcHeightOpt::Unknown)
        || (withNewLine && pdfData.m_drawFootnotes && heightCalcOpt == CalcHeightOpt::Unknown)) {
        moveToNewLine(pdfData, offset, lineHeight + cw.height(), 1.0, (pdfData.m_drawFootnotes ? lineHeight + cw.height() : cw.height()));
    } else {
        moveToNewLine(pdfData, offset, cw.height(), 1.0, cw.height());
    }

    pdfData.m_layout.moveXToBegin();

    bool extraOnFirstLine = true;
    newLine = false;

    const auto firstLineY = pdfData.m_layout.y();
    const auto firstLinePageIdx = pdfData.currentPageIndex();
    const auto firstLineHeight = cw.height();

    pdfData.m_continueParagraph = true;

    lineBreak = false;
    firstInParagraph = true;

    // Actual drawing.
    for (auto it = item->items().begin(), last = item->items().end(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        int nextFootnoteNum = m_footnoteNum;

        if (it + 1 != last && (it + 1)->get()->type() == MD::ItemType::FootnoteRef) {
            auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>((it + 1)->get());

            const auto fit = doc->footnotesMap().find(ref->id());

            if (fit != doc->footnotesMap().cend()) {
                auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());

                if (anchorIt != pdfData.m_footnotesAnchorsMap.cend()) {
                    nextFootnoteNum = anchorIt->second;
                }
            }
        }

        switch ((*it)->type()) {
        case MD::ItemType::Text: {
            auto text = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

            rects.append(drawText(pdfData,
                                  renderOpts,
                                  text,
                                  doc,
                                  newLine,
                                  nullptr,
                                  0.0,
                                  1.0,
                                  nullptr,
                                  nextFootnoteNum,
                                  offset,
                                  (firstInParagraph || lineBreak),
                                  cw,
                                  scale,
                                  color,
                                  rtl));
            lineBreak = false;
            firstInParagraph = false;
        } break;

        case MD::ItemType::Code: {
            rects.append(drawInlinedCode(pdfData,
                                         renderOpts,
                                         static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                                         doc,
                                         newLine,
                                         offset,
                                         (firstInParagraph || lineBreak),
                                         cw,
                                         scale));
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        case MD::ItemType::Link: {
            auto link = static_cast<MD::Link<MD::QStringTrait> *>(it->get());

            if ((!link->img()->isEmpty() && !link->p()->isEmpty() &&
                 link->p()->getItemAt(0)->type() == MD::ItemType::Image) && extraOnFirstLine) {
                pdfData.m_layout.addY(cw.height(), -1.0);
            }

            rects.append(drawLink(pdfData,
                                  renderOpts,
                                  link,
                                  doc,
                                  newLine,
                                  nullptr,
                                  0.0,
                                  1.0,
                                  nullptr,
                                  nextFootnoteNum,
                                  offset,
                                  (firstInParagraph || lineBreak),
                                  cw,
                                  scale,
                                  rtl));
            lineBreak = false;
            firstInParagraph = false;
        } break;

        case MD::ItemType::Image: {
            if (extraOnFirstLine) {
                pdfData.m_layout.addY(cw.height(), -1.0);
            }

            rects.append(drawImage(pdfData,
                                   renderOpts,
                                   static_cast<MD::Image<MD::QStringTrait> *>(it->get()),
                                   doc,
                                   newLine,
                                   offset,
                                   (firstInParagraph || lineBreak),
                                   cw,
                                   1.0,
                                   renderOpts.m_imageAlignment,
                                   scaleImagesToLineHeight));
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        case MD::ItemType::Math: {
            pdfData.setColor(color);
            rects.append(drawMathExpr(pdfData,
                                      renderOpts,
                                      static_cast<MD::Math<MD::QStringTrait> *>(it->get()),
                                      doc,
                                      newLine,
                                      offset,
                                      (std::next(it) != last),
                                      (firstInParagraph || lineBreak),
                                      cw,
                                      scale));
            pdfData.restoreColor();
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        case MD::ItemType::LineBreak: {
            lineBreak = true;
            moveToNewLine(pdfData, offset, lineHeight, 1.0, lineHeight);
        } break;

        case MD::ItemType::FootnoteRef: {
            lineBreak = false;

            auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>(it->get());

            const auto fit = doc->footnotesMap().find(ref->id());

            if (fit != doc->footnotesMap().cend()) {
                auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());
                int num = m_footnoteNum;

                if (anchorIt != pdfData.m_footnotesAnchorsMap.cend()) {
                    num = anchorIt->second;
                }

                const auto str = createUtf8String(QString::number(num));

                const auto w = pdfData.stringWidth(footnoteFont, renderOpts.m_textFontSize * s_footnoteScale, scale, str);

                rects.append(qMakePair(pdfData.m_layout.currentRect(w, lineHeight), pdfData.currentPageIndex()));

                m_unresolvedFootnotesLinks.insert(ref->id(), qMakePair(pdfData.m_layout.currentRect(w, lineHeight),
                                                                       pdfData.currentPageIndex()));

                pdfData.setColor(renderOpts.m_linkColor);

                pdfData.drawText(pdfData.m_layout.startX(w),
                                 pdfData.m_layout.y() + lineHeight - pdfData.lineSpacing(footnoteFont,
                                                                                         renderOpts.m_textFontSize * s_footnoteScale, scale)
                                     - pdfData.fontDescent(footnoteFont, renderOpts.m_textFontSize * s_footnoteScale, scale),
                                 str,
                                 footnoteFont,
                                 renderOpts.m_textFontSize * s_footnoteScale * scale,
                                 1.0,
                                 false);

                pdfData.restoreColor();

                pdfData.m_layout.addX(w);

                addFootnote(ref->id(), fit->second, pdfData, renderOpts, doc);
            } else {
                auto text = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

                rects.append(drawText(pdfData,
                                      renderOpts,
                                      text,
                                      doc,
                                      newLine,
                                      nullptr,
                                      0.0,
                                      1.0,
                                      nullptr,
                                      nextFootnoteNum,
                                      offset,
                                      (firstInParagraph || lineBreak),
                                      cw,
                                      scale,
                                      color));
            }

            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        default:
            break;
        }

        extraOnFirstLine = firstInParagraph;
    }

    pdfData.m_continueParagraph = false;

    pdfData.m_layout.setRightToLeft(wasRightToLeft);

    return {toWhereDrawn(normalizeRects(rects), pdfData.m_layout.pageHeight()),
        {firstLinePageIdx, firstLineY, firstLineHeight}};
}

QPair<QRectF, unsigned int> PdfRenderer::drawMathExpr(PdfAuxData &pdfData,
                                                      const RenderOpts &renderOpts,
                                                      MD::Math<MD::QStringTrait> *item,
                                                      std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                      bool &newLine,
                                                      double offset,
                                                      bool hasNext,
                                                      bool firstInParagraph,
                                                      CustomWidth &cw,
                                                      double scale)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    float fontSize = (float) renderOpts.m_textFontSize;

    {
        PoDoFoPaintDevice pd(pdfData.m_fontsCache);
        fontSize = fontSize / 72.f * (float) pd.physicalDpiY();
    }

    auto latexRender = std::unique_ptr<tex::TeXRender>(tex::LaTeX::parse(
            item->expr().toStdWString(),
            0,
            fontSize,
            fontSize / 3.f,
            tex::black));

    QSizeF pxSize = {}, size = {};
    double descent = 0.0;

    auto *font = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    {
        PoDoFoPaintDevice pd(pdfData.m_fontsCache);
        QPainter p(&pd);
        tex::Graphics2D_qt g2(&p);
        latexRender->draw(g2, 0, 0);
        pxSize = {(qreal)latexRender->getWidth(), (qreal)latexRender->getHeight()};
        size = {pxSize.width() / (qreal) pd.physicalDpiX() * 72.0, pxSize.height() / (qreal) pd.physicalDpiY() * 72.0};
        descent -= pdfData.fontDescent(font, renderOpts.m_textFontSize, scale);
    }

    const auto lineHeight = pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale);

    newLine = false;

    bool draw = true;

    if (!cw.isDrawing()) {
        draw = false;
    }

    if (cw.isDrawing() && !item->isInline()) {
        if (!firstInParagraph) {
            cw.moveToNextLine();
        } else {
            pdfData.m_layout.addY(cw.height(), -1.0);
        }

        pdfData.m_layout.moveXToBegin();
    }

    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

    double h = (cw.isDrawing() ? cw.height() : 0.0);
    double calculatedHeight = 0.0;

    if (!item->isInline()) {
        newLine = true;

        double x = 0.0;
        double imgScale = 1.0;
        const double availableWidth = pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
                pdfData.m_layout.margins().m_right - offset;
        double availableHeight = pdfData.m_layout.y() - pdfData.currentPageAllowedY();

        if (size.width() - availableWidth > 0.01) {
            imgScale = (availableWidth / size.width()) * scale;
        }

        const double pageHeight = pdfData.topY(pdfData.currentPageIndex()) - pdfData.m_layout.margins().m_bottom;

        if (size.height() * imgScale - pageHeight > 0.01) {
            imgScale = (pageHeight / (size.height() * imgScale)) * scale;

            if (!pdfData.m_firstOnPage && draw) {
                createPage(pdfData);

                pdfData.m_layout.addX(offset);
            }

            if (draw) {
                pdfData.freeSpaceOn(pdfData.currentPageIndex());
            }
        } else if (size.height() * imgScale - availableHeight > 0.01) {
            if (draw) {
                createPage(pdfData);

                pdfData.freeSpaceOn(pdfData.currentPageIndex());

                pdfData.m_layout.addX(offset);
            }
        }

        if (draw) {
            pdfData.m_firstOnPage = false;

            if (availableWidth - size.width() * imgScale > 0.01) {
                x = (availableWidth - size.width() * imgScale) / 2.0;
            }

            PoDoFoPaintDevice pd(pdfData.m_fontsCache);
            pd.setPdfPainter(*(*pdfData.m_painters)[pdfData.m_currentPainterIdx].get(), *pdfData.m_doc);
            QPainter p(&pd);

            pdfData.m_layout.addX(x);

            tex::Graphics2D_qt g2(&p);
            latexRender->draw(g2, pdfData.m_layout.startX(size.width() * imgScale) / 72.0 * pd.physicalDpiX(),
                    (pdfData.m_layout.pageHeight() - pdfData.m_layout.y() + descent * imgScale) / 72.0
                              * pd.physicalDpiY());

            const QRectF r = {pdfData.m_layout.startX(size.width() * imgScale),
                              pdfData.m_layout.y() - descent * imgScale,
                              size.width() * imgScale,
                              size.height() * imgScale};
            const auto idx = pdfData.currentPageIndex();

            pdfData.m_layout.addY(h);

            cw.moveToNextLine();

            if (hasNext)
                moveToNewLine(pdfData, offset, lineHeight, 1.0, lineHeight);

            return {r, idx};
        } else {
            calculatedHeight += size.height() * imgScale + descent * imgScale;

            pdfData.m_layout.moveXToBegin();

            if (!firstInParagraph) {
                cw.append({0.0, 0.0, false, true, false, ""});
            }

            cw.append({0.0, calculatedHeight, false, true, false, ""});
        }
    } else {
        auto sscale = 100.0;

        if (draw) {
            sscale = cw.scale();
        }

        const auto spaceWidth = pdfData.stringWidth(font, renderOpts.m_textFontSize, scale, " ");

        pdfData.m_layout.addX(spaceWidth * sscale / 100.0);

        const double availableTotalWidth = pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
                pdfData.m_layout.margins().m_right - offset;

        if (size.width() - pdfData.m_layout.availableWidth() > 0.01) {
            if (draw) {
                cw.moveToNextLine();
                h = cw.height();

                moveToNewLine(pdfData, offset, h, 1.0, h);
            } else {
                cw.append({0.0, lineHeight, false, true, true, ""});
                pdfData.m_layout.moveXToBegin();
            }
        } else if (!draw) {
            cw.append({spaceWidth, lineHeight, true, false, true, " "});
        }

        double imgScale = 1.0;

        if (size.width() - availableTotalWidth > 0.01) {
            imgScale = (pdfData.m_layout.availableWidth() / size.width()) * scale;
        }

        double availableHeight = pdfData.m_layout.y() - pdfData.currentPageAllowedY();

        const double pageHeight = pdfData.topY(pdfData.currentPageIndex()) - pdfData.m_layout.margins().m_bottom;

        if (size.height() * imgScale - pageHeight > 0.01) {
            imgScale = (pageHeight / (size.height() * imgScale)) * scale;

            if (draw) {
                createPage(pdfData);

                pdfData.freeSpaceOn(pdfData.currentPageIndex());

                pdfData.m_layout.addX(offset);
            }
        } else if (size.height() * imgScale - availableHeight > 0.01) {
            if (draw) {
                createPage(pdfData);

                pdfData.freeSpaceOn(pdfData.currentPageIndex());

                pdfData.m_layout.addX(offset);
            }
        }

        if (draw) {
            pdfData.m_firstOnPage = false;

            PoDoFoPaintDevice pd(pdfData.m_fontsCache);
            pd.setPdfPainter(*(*pdfData.m_painters)[pdfData.m_currentPainterIdx].get(), *pdfData.m_doc);
            QPainter p(&pd);

            tex::Graphics2D_qt g2(&p);
            latexRender->draw(g2,
                    pdfData.m_layout.startX(size.width() * imgScale) / 72.0 * pd.physicalDpiX(),
                                   (pdfData.m_layout.pageHeight() - pdfData.m_layout.y() - h +
                                    (h - size.height() * imgScale)) / 72.0 * pd.physicalDpiY());

            const QRectF r = {pdfData.m_layout.startX(size.width() * imgScale),
                              pdfData.m_layout.y() + h - (h - size.height() * imgScale),
                              size.width() * imgScale,
                              size.height() * imgScale};
            pdfData.m_layout.addX(size.width() * imgScale);
            const auto idx = pdfData.currentPageIndex();

            return {r, idx};
        } else {
            pdfData.m_layout.addX(size.width() * imgScale);

            cw.append({size.width() * imgScale, size.height() * imgScale + descent * imgScale, false, false, hasNext, ""});
        }
    }

    return {};
}

void PdfRenderer::reserveSpaceForFootnote(PdfAuxData &pdfData,
                                          const RenderOpts &renderOpts,
                                          const QVector<WhereDrawn> &h,
                                          const double &currentY,
                                          int currentPage,
                                          double lineHeight,
                                          bool addExtraLine)
{
    const auto topY = pdfData.topFootnoteY(currentPage);
    const auto available = currentY - topY - (qAbs(topY) < 0.01 ? pdfData.m_layout.margins().m_bottom : 0.0);

    auto height = totalHeight(h) + (addExtraLine ? lineHeight : 0.0);
    auto extra = 0.0;

    if (!pdfData.m_reserved.contains(currentPage)) {
        extra = pdfData.m_extraInFootnote;
    }

    auto add = [&pdfData](const double &height, int currentPage) {
        if (pdfData.m_reserved.contains(currentPage)) {
            pdfData.m_reserved[currentPage] += height;
        } else {
            pdfData.m_reserved.insert(currentPage, height + pdfData.m_layout.margins().m_bottom);
        }
    };

    if (height + extra < available) {
        add(height + extra, currentPage);
    } else {
        height = extra + (addExtraLine ? lineHeight : 0.0);

        for (int i = 0; i < h.size(); ++i) {
            const auto tmp = h[i].m_height;

            if (height + tmp < available) {
                height += tmp;
            } else {
                if (qAbs(height - extra) > 0.01) {
                    add(height, currentPage);
                }

                reserveSpaceForFootnote(pdfData,
                                        renderOpts,
                                        h.mid(i),
                                        pdfData.m_layout.topY(),
                                        currentPage + 1,
                                        lineHeight,
                                        true);

                break;
            }
        }
    }
}

QVector<WhereDrawn> PdfRenderer::drawFootnote(PdfAuxData &pdfData,
                                              const RenderOpts &renderOpts,
                                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                              const QString &footnoteRefId,
                                              MD::Footnote<MD::QStringTrait> *note,
                                              CalcHeightOpt heightCalcOpt,
                                              double *lineHeight,
                                              RTLFlag *rtl)
{
    QVector<WhereDrawn> ret;

    pdfData.m_startLine = note->startLine();
    pdfData.m_startPos = note->startColumn();
    pdfData.m_endLine = note->endLine();
    pdfData.m_endPos = note->endColumn();

    if (heightCalcOpt == CalcHeightOpt::Unknown && pdfData.m_footnotesAnchorsMap.contains(note)) {
        pdfData.m_currentFile = pdfData.m_footnotesAnchorsMap[note].first;
    }

    static const double c_offset = 2.0;

    auto *font = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
                            pdfData.m_doc, s_footnoteScale, pdfData);

    auto footnoteOffset = c_offset * 2.0 / s_mmInPt
        + pdfData.stringWidth(font, renderOpts.m_textFontSize, s_footnoteScale,
                              createUtf8String(QString::number(doc->footnotesMap().size())));

    if (lineHeight) {
        *lineHeight = pdfData.lineSpacing(font, renderOpts.m_textFontSize, s_footnoteScale);
    }

    bool first = true;
    bool firstItemIsRightToLeft = false;

    for (auto it = note->items().cbegin(), last = note->items().cend(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        switch ((*it)->type()) {
        case MD::ItemType::Heading:
            ret.append(drawHeading(pdfData,
                                   renderOpts,
                                   static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                                   doc,
                                   footnoteOffset,
                                   // If there is another item after heading we need to know its min
                                   // height to glue heading with it.
                                   (it + 1 != last ? minNecessaryHeight(pdfData, renderOpts, *(it + 1), doc, 0.0, s_footnoteScale) : 0.0),
                                   heightCalcOpt,
                                   s_footnoteScale,
                                   true,
                                   rtl)
                           .first);

            pdfData.m_continueParagraph = true;

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::Paragraph:
            ret.append(drawParagraph(pdfData,
                                     renderOpts,
                                     static_cast<MD::Paragraph<MD::QStringTrait> *>(it->get()),
                                     doc,
                                     footnoteOffset,
                                     true,
                                     heightCalcOpt,
                                     s_footnoteScale,
                                     Qt::black,
                                     false,
                                     rtl)
                           .first);

            pdfData.m_continueParagraph = true;

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::Code:
            ret.append(
                drawCode(pdfData, renderOpts, static_cast<MD::Code<MD::QStringTrait> *>(
                             it->get()), doc, footnoteOffset, heightCalcOpt, s_footnoteScale).first);
            pdfData.m_continueParagraph = true;

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::Blockquote:
            ret.append(drawBlockquote(pdfData,
                                      renderOpts,
                                      static_cast<MD::Blockquote<MD::QStringTrait> *>(it->get()),
                                      doc,
                                      footnoteOffset,
                                      heightCalcOpt,
                                      s_footnoteScale,
                                      rtl)
                           .first);

            pdfData.m_continueParagraph = true;

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::List: {
            auto *list = static_cast<MD::List<MD::QStringTrait> *>(it->get());
            const auto bulletWidth = maxListNumberWidth(list);

            ret.append(drawList(pdfData, renderOpts, list, doc, bulletWidth, footnoteOffset,
                                heightCalcOpt, s_footnoteScale, false, rtl).first);

            pdfData.m_continueParagraph = true;

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);
        } break;

        case MD::ItemType::Table:
            ret.append(
                drawTable(pdfData, renderOpts, static_cast<MD::Table<MD::QStringTrait> *>(
                              it->get()), doc, footnoteOffset, heightCalcOpt, s_footnoteScale, rtl)
                    .first);

            pdfData.m_continueParagraph = true;

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        default:
            break;
        }

        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        pdfData.m_layout.setRightToLeft(firstItemIsRightToLeft);

        // Draw footnote number.
        if (it == note->items().cbegin() && heightCalcOpt == CalcHeightOpt::Unknown) {
            const auto str = createUtf8String(QString::number(pdfData.m_currentFootnote));
            const auto w = pdfData.stringWidth(font, renderOpts.m_textFontSize, s_footnoteScale, str);
            const auto y = ret.constFirst().m_y + ret.constFirst().m_height
                - pdfData.lineSpacing(font, renderOpts.m_textFontSize, s_footnoteScale)
                - pdfData.fontDescent(font, renderOpts.m_textFontSize, s_footnoteScale);
            const auto x = pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                    (footnoteOffset - c_offset - (pdfData.m_layout.isRightToLeft() ? 0.0 : w));
            const auto p = ret.constFirst().m_pageIdx;

            auto dest = pdfData.m_doc->CreateDestination();
            dest->SetDestination(pdfData.m_doc->GetPages().GetPageAt(p),
                                 x,
                                 y + pdfData.lineSpacing(font, renderOpts.m_textFontSize, s_footnoteScale)
                                     + pdfData.fontDescent(font, renderOpts.m_textFontSize, s_footnoteScale),
                                 0.0);
            m_dests.insert(footnoteRefId, std::move(dest));

            pdfData.m_currentPainterIdx = p;

            pdfData.drawText(x, y, str, font, renderOpts.m_textFontSize * s_footnoteScale, 1.0, false);

            pdfData.m_currentPainterIdx = pdfData.m_footnotePageIdx;

            ++pdfData.m_currentFootnote;
        }
    }

    pdfData.m_layout.setRightToLeft(false);

    pdfData.m_continueParagraph = false;

    return ret;
}

QVector<WhereDrawn> PdfRenderer::footnoteHeight(PdfAuxData &pdfData,
                                                const RenderOpts &renderOpts,
                                                std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                MD::Footnote<MD::QStringTrait> *note,
                                                double *lineHeight)
{
    return drawFootnote(pdfData, renderOpts, doc, "", note, CalcHeightOpt::Full, lineHeight);
}

QPair<QRectF, unsigned int> PdfRenderer::drawImage(PdfAuxData &pdfData,
                                                   const RenderOpts &renderOpts,
                                                   MD::Image<MD::QStringTrait> *item,
                                                   std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                   bool &newLine,
                                                   double offset,
                                                   bool firstInParagraph,
                                                   CustomWidth &cw,
                                                   double scale,
                                                   ImageAlignment alignment,
                                                   bool scaleImagesToLineHeight)
{
    Q_UNUSED(doc)

    bool draw = true;
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    if (!cw.isDrawing())
        draw = false;

    emit status(tr("Loading image."));

    auto *font = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    const auto lineHeight = pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale);

    const auto img = loadImage(item, *pdfData.m_resvgOpts.get(),
                               lineHeight / 72.0 * pdfData.m_dpi, scaleImagesToLineHeight, !scaleImagesToLineHeight);

    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

    if (!img.isNull()) {
        auto pdfImg = pdfData.m_doc->CreateImage();
        pdfImg->LoadFromBuffer({img.data(), static_cast<size_t>(img.size())});

        const double iWidth = std::round((double)pdfImg->GetWidth() / (double)pdfData.m_dpi * 72.0);
        const double iHeight = std::round((double)pdfImg->GetHeight() / (double)pdfData.m_dpi * 72.0);

        newLine = false;

        double x = 0.0;
        double imgScale = (scaleImagesToLineHeight ? lineHeight / iHeight : 1.0);
        const double totalAvailableWidth = pdfData.m_layout.pageWidth()
                - pdfData.m_layout.margins().m_left - pdfData.m_layout.margins().m_right - offset;
        double availableHeight = pdfData.m_layout.y() - pdfData.currentPageAllowedY();
        const double pageHeight = pdfData.topY(pdfData.currentPageIndex()) - pdfData.m_layout.margins().m_bottom;
        const bool onLine = (totalAvailableWidth / 5.0 > iWidth && iHeight < lineHeight * 2.0);
        const auto spaceWidth = pdfData.stringWidth(font, renderOpts.m_textFontSize, scale, " ");
        bool addSpace = onLine && !firstInParagraph;
        double height = 0.0;

        const auto availableAfter = pdfData.m_layout.availableWidth() - (iWidth * imgScale +
            (addSpace ? spaceWidth * (draw ? cw.scale() / 100.0 : 1.0) : 0.0));

        if (!onLine && !firstInParagraph || (onLine && availableAfter < 0 && qAbs(availableAfter) > 0.1)) {
            newLine = true;

            if (draw) {
                cw.moveToNextLine();
            } else {
                if (!onLine) {
                    height += lineHeight;
                }

                cw.append({0.0, 0.0, false, true, onLine, ""});
            }

            if (draw) {
                moveToNewLine(pdfData, offset, (onLine ? cw.height() : lineHeight), 1.0, lineHeight);
            } else {
                pdfData.m_layout.moveXToBegin();
            }

            addSpace = false;
        }

        if (!onLine && !scaleImagesToLineHeight) {
            if (iWidth > totalAvailableWidth) {
                imgScale = (totalAvailableWidth / iWidth);
            }

            if (iHeight * imgScale > pageHeight) {
                imgScale = (pageHeight / (iHeight * imgScale));

                if (!pdfData.m_firstOnPage && draw) {
                    createPage(pdfData);

                    pdfData.m_layout.addX(offset);
                }

                pdfData.freeSpaceOn(pdfData.currentPageIndex());
            } else if (iHeight * imgScale > availableHeight) {
                if (draw) {
                    createPage(pdfData);

                    pdfData.freeSpaceOn(pdfData.currentPageIndex());

                    pdfData.m_layout.addX(offset);
                }
            }

            if (iWidth * imgScale < totalAvailableWidth) {
                switch (alignment) {
                case ImageAlignment::Left:
                    x = pdfData.m_layout.isRightToLeft() ? pdfData.m_layout.x() - iWidth * imgScale * scale -
                                                           pdfData.m_layout.margins().m_left : 0.0;
                    break;

                case ImageAlignment::Center:
                    x = (totalAvailableWidth - iWidth * imgScale * scale) / 2.0;
                    break;

                case ImageAlignment::Right:
                    x = pdfData.m_layout.isRightToLeft() ? 0.0 :
                        pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_right - iWidth * imgScale * scale -
                                                           pdfData.m_layout.x();
                    break;

                default:
                    break;
                }
            }
        }

        const double dpiScale = (double)pdfImg->GetWidth() / iWidth;
        double dy = 0.0;
        imgScale *= scale;

        if (draw) {
            if (!onLine) {
                pdfData.m_layout.addY(iHeight * imgScale);
            } else if (firstInParagraph) {
                pdfData.m_layout.addY(cw.height());
            }

            if (onLine && addSpace) {
                pdfData.m_layout.addX(spaceWidth * cw.scale() / 100.0);
            }

            dy = (onLine ? (cw.height() - iHeight * imgScale) / 2.0 : 0.0);

            pdfData.m_layout.addX(x);

            pdfData.drawImage(pdfData.m_layout.startX(iWidth * imgScale), pdfData.m_layout.y() + dy,
                              pdfImg.get(), imgScale / dpiScale, imgScale / dpiScale);
        } else {
            if (onLine && addSpace) {
                cw.append({spaceWidth, lineHeight, true, false, true, " "});
                pdfData.m_layout.addX(spaceWidth);
            }

            height += iHeight * imgScale;
        }

        QRectF r(pdfData.m_layout.startX(iWidth * imgScale), pdfData.m_layout.y() + dy,
                 iWidth * imgScale, iHeight * imgScale);

        if (onLine) {
            pdfData.m_layout.addX(iWidth * imgScale);
        } else {
            pdfData.m_layout.moveXToBegin();
        }

        if (draw && !onLine) {
            newLine = true;

            moveToNewLine(pdfData, offset, lineHeight, 1.0, lineHeight);

            cw.moveToNextLine();
        }

        if (!draw) {
            cw.append({iWidth * imgScale, height, false, !onLine, false, ""});
        }

        return qMakePair(r, pdfData.currentPageIndex());
    } else {
        throw PdfRendererError(tr("Unable to load image: %1.\n\n"
                                  "If this image is in Web, please be sure you are connected to the Internet. I'm "
                                  "sorry for the inconvenience.")
                                   .arg(item->url()));
    }
}

//
// LoadImageFromNetwork
//

LoadImageFromNetwork::LoadImageFromNetwork(const QUrl &url, QThread *thread,
                                           const ResvgOptions &opts, double height, bool scale)
    : m_thread(thread)
    , m_reply(nullptr)
    , m_url(url)
    , m_opts(opts)
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
    emit start();
}

void LoadImageFromNetwork::loadImpl()
{
    QNetworkAccessManager *m = new QNetworkAccessManager(this);
    QNetworkRequest r(m_url);
    r.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m->get(r);

    connect(m_reply, &QNetworkReply::finished, this, &LoadImageFromNetwork::loadFinished);
    connect(m_reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(
                &QNetworkReply::errorOccurred), this, &LoadImageFromNetwork::loadError);
}

void LoadImageFromNetwork::loadFinished()
{
    const auto data = m_reply->readAll();
    const auto svg = QString(data.mid(0, 4).toLower());

    if (svg == QStringLiteral("<svg") || svg == QStringLiteral("<?xm")) {
        ResvgRenderer r(data, m_opts);

        if (r.isValid()) {
            double s = m_height / (double)r.defaultSize().height();
            m_img =
                r.renderToImage(m_scale ? QSize(qRound((double)r.defaultSize().width() * s), qRound((double)r.defaultSize().height() * s)) : r.defaultSize());
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

QByteArray PdfRenderer::loadImage(MD::Image<MD::QStringTrait> *item, const ResvgOptions &opts, double height, bool scale, bool cache)
{
    if (cache && m_imageCache.contains(item->url())) {
        return m_imageCache[item->url()];
    }

    QImage img;

    if (QFileInfo::exists(item->url())) {
        if (item->url().toLower().endsWith(QStringLiteral("svg")) || item->url().toLower().endsWith(QStringLiteral("svgz"))) {
            ResvgRenderer r(item->url(), opts);

            if (r.isValid()) {
                double s = height / (double)r.defaultSize().height();
                img =
                    r.renderToImage(scale ? QSize(qRound((double)r.defaultSize().width() * s),
                                                  qRound((double)r.defaultSize().height() * s)) : r.defaultSize());
            }
        } else {
            img = QImage(item->url());
        }
    } else if (!QUrl(item->url()).isRelative()) {
        QThread thread;

        LoadImageFromNetwork load(QUrl(item->url()), &thread, opts, height, scale);

        load.moveToThread(&thread);
        thread.start();
        load.load();
        thread.wait();

        img = load.image();

#ifdef MD_PDF_TESTING
        if (img.isNull()) {
            terminate();

            QWARN("Got empty image from network.");
        }
#endif
    } else {
        throw PdfRendererError(tr("Hmm, I don't know how to load this image: %1.\n\n"
                                  "This image is not a local existing file, and not in the Web. Check your Markdown.")
                                   .arg(item->url()));
    }

    if (img.isNull()) {
        throw PdfRendererError(tr("Unable to load image: %1.\n\n"
                                  "If this image is in Web, please be sure you are connected to the Internet. I'm "
                                  "sorry for the inconvenience.")
                                   .arg(item->url()));
    }

    QByteArray data;
    QBuffer buf(&data);

    QString fmt = QStringLiteral("png");

    if (item->url().endsWith(QStringLiteral("jpg")) || item->url().endsWith(QStringLiteral("jpeg"))) {
        fmt = QStringLiteral("jpg");
    }

    img.save(&buf, fmt.toLatin1().constData());

    if (cache) {
        m_imageCache.insert(item->url(), data);
    }

    return data;
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawCode(PdfAuxData &pdfData,
                                                             const RenderOpts &renderOpts,
                                                             MD::Code<MD::QStringTrait> *item,
                                                             std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                             double offset,
                                                             CalcHeightOpt heightCalcOpt,
                                                             double scale)
{
    Q_UNUSED(doc)

    const auto wasRightToLeft = pdfData.m_layout.isRightToLeft();
    const auto autoOffset = pdfData.m_layout.addOffset(offset, !wasRightToLeft);
    pdfData.m_layout.setRightToLeft(false);

    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    if (item->text().isEmpty()) {
        return {};
    }

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        emit status(tr("Drawing code."));
    }

    auto *textFont = createFont(renderOpts.m_textFont, false, false,
                                renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    const auto textLHeight = pdfData.lineSpacing(textFont, renderOpts.m_textFontSize, scale);

    QStringList lines;

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        if (pdfData.m_layout.y() - (textLHeight * 2.0) < pdfData.currentPageAllowedY()
            && qAbs(pdfData.m_layout.y() - (textLHeight * 2.0) - pdfData.currentPageAllowedY()) > 0.1) {
            createPage(pdfData);
        } else {
            pdfData.m_layout.addY(textLHeight * 2.0);
        }

        pdfData.m_layout.moveXToBegin();
    }

    lines = item->text().split(QLatin1Char('\n'), Qt::KeepEmptyParts);

    for (auto it = lines.begin(), last = lines.end(); it != last; ++it) {
        it->replace(QStringLiteral("\t"), QStringLiteral("    "));
    }

    auto *font = createFont(renderOpts.m_codeFont, false, false, renderOpts.m_codeFontSize, pdfData.m_doc, scale, pdfData);

    const auto lineHeight = pdfData.lineSpacing(font, renderOpts.m_codeFontSize, scale);

    switch (heightCalcOpt) {
    case CalcHeightOpt::Minimum: {
        QVector<WhereDrawn> r;
        r.append({-1, 0.0, textLHeight * 2.0 + lineHeight});

        return {r, {}};
    }

    case CalcHeightOpt::Full: {
        QVector<WhereDrawn> r;
        r.append({-1, 0.0, textLHeight * 2.0 + lineHeight});

        auto i = 1;

        while (i < lines.size()) {
            r.append({-1, 0.0, lineHeight});
            ++i;
        }

        return {r, {}};
    }

    default:
        break;
    }

    int i = 0;

    QVector<WhereDrawn> ret;

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate)
            return {};
    }

    pdfData.m_syntax->setDefinition(pdfData.m_syntax->definitionForName(item->syntax().toLower()));
    const auto colored = pdfData.m_syntax->prepare(lines);
    int currentWord = 0;
    const auto spaceWidth = pdfData.stringWidth(font, renderOpts.m_codeFontSize, scale, " ");

    const auto firstLinePageIdx = pdfData.currentPageIndex();
    const auto firstLineY = pdfData.m_layout.y();
    const auto firstLineHeight = lineHeight;

    while (i < lines.size()) {
        auto y = pdfData.m_layout.y();
        int j = i + 1;
        double h = 0.0;

        while ((y - lineHeight > pdfData.currentPageAllowedY() ||
                qAbs(y - lineHeight - pdfData.currentPageAllowedY()) < 0.1) && j < lines.size()) {
            h += lineHeight;
            y -= lineHeight;
            ++j;
        }

        if (i < j) {
            pdfData.setColor(renderOpts.m_syntax->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding));
            pdfData.drawRectangle(pdfData.m_layout.startX(pdfData.m_layout.availableWidth()),
                                  y + pdfData.fontDescent(font, renderOpts.m_codeFontSize, scale),
                                  pdfData.m_layout.availableWidth(),
                                  h + lineHeight,
                                  PoDoFo::PdfPathDrawMode::Fill);
            pdfData.restoreColor();

            ret.append({pdfData.currentPageIndex(), y + pdfData.fontDescent(font, renderOpts.m_codeFontSize, scale),
                        h + lineHeight});
        }

        for (; i < j; ++i) {
            pdfData.m_layout.moveXToBegin();

            while (true) {
                if (currentWord == colored.size() || colored[currentWord].line != i) {
                    break;
                }

                pdfData.setColor(colored[currentWord].format.textColor(pdfData.m_syntax->theme()));

                const auto length = colored[currentWord].endPos - colored[currentWord].startPos + 1;

                Font *f = font;

                const auto italic = colored[currentWord].format.isItalic(pdfData.m_syntax->theme());
                const auto bold = colored[currentWord].format.isBold(pdfData.m_syntax->theme());

                if (italic || bold) {
                    f = createFont(renderOpts.m_codeFont, bold, italic, renderOpts.m_codeFontSize, pdfData.m_doc, scale, pdfData);
                }

                pdfData.drawText(pdfData.m_layout.startX(spaceWidth * length),
                                 pdfData.m_layout.y(),
                                 createUtf8String(lines.at(i).mid(colored[currentWord].startPos, length)),
                                 f,
                                 renderOpts.m_codeFontSize * scale,
                                 1.0,
                                 false);

                pdfData.m_layout.addX(spaceWidth * length);

                pdfData.restoreColor();

                ++currentWord;
            }

            pdfData.m_layout.addY(lineHeight);
        }

        if (i < lines.size()) {
            createPage(pdfData);
            pdfData.m_layout.moveXToBegin();
            pdfData.m_layout.addY(lineHeight);
        }
    }

    pdfData.m_layout.setRightToLeft(wasRightToLeft);

    return {ret, {firstLinePageIdx, firstLineY, firstLineHeight}};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawBlockquote(PdfAuxData &pdfData,
                                                                   const RenderOpts &renderOpts,
                                                                   MD::Blockquote<MD::QStringTrait> *item,
                                                                   std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                   double offset,
                                                                   CalcHeightOpt heightCalcOpt,
                                                                   double scale,
                                                                   RTLFlag *rtl)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    QVector<WhereDrawn> ret;

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        emit status(tr("Drawing blockquote."));
    }

    bool first = true;
    WhereDrawn firstLine = {};

    QColor color;

    if (pdfData.m_highlightedBlockquotes.contains(item)) {
        color = pdfData.m_highlightedBlockquotes[item];
    }

    // Draw items.
    for (auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate)
                return {};
        }

        switch ((*it)->type()) {
        case MD::ItemType::Heading: {
            if (heightCalcOpt != CalcHeightOpt::Minimum) {
                const auto where =
                    drawHeading(pdfData,
                                renderOpts,
                                static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                                doc,
                                offset + s_blockquoteBaseOffset,
                                (it + 1 != last ? minNecessaryHeight(pdfData, renderOpts, *(it + 1), doc,
                                                                     offset + s_blockquoteBaseOffset, scale) : 0.0),
                                heightCalcOpt,
                                scale,
                                true,
                                rtl);

                ret.append(where.first);

                if (first) {
                    firstLine = where.second;
                    first = false;
                }
            } else {
                ret.append({-1, 0.0, 0.0});

                return {ret, {}};
            }
        } break;

        case MD::ItemType::Paragraph: {
            const auto where = drawParagraph(pdfData,
                                             renderOpts,
                                             static_cast<MD::Paragraph<MD::QStringTrait> *>(it->get()),
                                             doc,
                                             offset + s_blockquoteBaseOffset,
                                             true,
                                             heightCalcOpt,
                                             scale,
                                             (color.isValid() && first ? color : Qt::black),
                                             (color.isValid() && first ? true : false),
                                             rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        case MD::ItemType::Code: {
            const auto where =
                drawCode(pdfData, renderOpts, static_cast<MD::Code<MD::QStringTrait> *>(
                             it->get()), doc, offset + s_blockquoteBaseOffset, heightCalcOpt, scale);

            setRTLFlagToFalseIfCheck(rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        case MD::ItemType::Blockquote: {
            const auto where = drawBlockquote(pdfData,
                                              renderOpts,
                                              static_cast<MD::Blockquote<MD::QStringTrait> *>(it->get()),
                                              doc,
                                              offset + s_blockquoteBaseOffset,
                                              heightCalcOpt,
                                              scale,
                                              rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        case MD::ItemType::List: {
            auto *list = static_cast<MD::List<MD::QStringTrait> *>(it->get());
            const auto bulletWidth = maxListNumberWidth(list);

            const auto where = drawList(pdfData, renderOpts, list, doc, bulletWidth,
                                        offset + s_blockquoteBaseOffset, heightCalcOpt, scale,
                                        false, rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        case MD::ItemType::Table: {
            const auto where = drawTable(pdfData,
                                         renderOpts,
                                         static_cast<MD::Table<MD::QStringTrait> *>(it->get()),
                                         doc,
                                         offset + s_blockquoteBaseOffset,
                                         heightCalcOpt,
                                         scale,
                                         rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        default:
            break;
        }

        if (heightCalcOpt == CalcHeightOpt::Minimum) {
            return {ret, {}};
        }
    }

    if (heightCalcOpt == CalcHeightOpt::Full) {
        return {ret, {}};
    }

    struct AuxData {
        double m_y = 0.0;
        double m_height = 0.0;
    }; // struct AuxData

    QMap<int, AuxData> map;

    for (const auto &where : std::as_const(ret)) {
        if (!map.contains(where.m_pageIdx)) {
            map.insert(where.m_pageIdx, {where.m_y, where.m_height});
        }

        if (map[where.m_pageIdx].m_y > where.m_y) {
            map[where.m_pageIdx].m_height = map[where.m_pageIdx].m_y + map[where.m_pageIdx].m_height - where.m_y;
            map[where.m_pageIdx].m_y = where.m_y;
        }
    }

    // Draw blockquote left vertival bar.
    for (auto it = map.cbegin(), last = map.cend(); it != last; ++it) {
        pdfData.m_currentPainterIdx = it.key();
        pdfData.setColor(color.isValid() ? color : renderOpts.m_borderColor);
        pdfData.drawRectangle(pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                              (offset + (pdfData.m_layout.isRightToLeft() ? s_blockquoteMarkWidth : 0.0)), it.value().m_y,
                              s_blockquoteMarkWidth, it.value().m_height, PoDoFo::PdfPathDrawMode::Fill);
        pdfData.restoreColor();
    }

    pdfData.m_currentPainterIdx = pdfData.currentPageIndex();

    return {ret, firstLine};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawList(PdfAuxData &pdfData,
                                                             const RenderOpts &renderOpts,
                                                             MD::List<MD::QStringTrait> *item,
                                                             std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                             int bulletWidth,
                                                             double offset,
                                                             CalcHeightOpt heightCalcOpt,
                                                             double scale,
                                                             bool nested,
                                                             RTLFlag *rtl)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    QVector<WhereDrawn> ret;

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate)
            return {};
    }

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        emit status(tr("Drawing list."));
    }

    int idx = 1;
    ListItemType prevListItemType = ListItemType::Unknown;
    bool first = true;
    WhereDrawn firstLine;

    for (auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it) {
        if ((*it)->type() == MD::ItemType::ListItem) {
            const auto where = drawListItem(pdfData,
                                            renderOpts,
                                            static_cast<MD::ListItem<MD::QStringTrait> *>(it->get()),
                                            doc,
                                            idx,
                                            prevListItemType,
                                            bulletWidth,
                                            offset,
                                            heightCalcOpt,
                                            scale,
                                            first && !nested,
                                            first,
                                            rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        }

        if (heightCalcOpt == CalcHeightOpt::Minimum) {
            break;
        }
    }

    if (!ret.isEmpty()) {
        ret.front().m_height += pdfData.lineSpacing(createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize,
                                                               pdfData.m_doc, scale, pdfData),
                                                  renderOpts.m_textFontSize,
                                                  scale);
    }

    return {ret, firstLine};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawListItem(PdfAuxData &pdfData,
                                                                 const RenderOpts &renderOpts,
                                                                 MD::ListItem<MD::QStringTrait> *item,
                                                                 std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                 int &idx,
                                                                 ListItemType &prevListItemType,
                                                                 int bulletWidth,
                                                                 double offset,
                                                                 CalcHeightOpt heightCalcOpt,
                                                                 double scale,
                                                                 bool firstInList,
                                                                 bool firstItem,
                                                                 RTLFlag *rtl)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    auto *font = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    const auto lineHeight = pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale);
    const auto orderedListNumberWidth =
        pdfData.stringWidth(font, renderOpts.m_textFontSize, scale, "9") * bulletWidth
            + pdfData.stringWidth(font, renderOpts.m_textFontSize, scale, ".");
    const auto spaceWidth = pdfData.stringWidth(font, renderOpts.m_textFontSize, scale, " ");
    const auto unorderedMarkWidth = lineHeight * 0.25;

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        offset += orderedListNumberWidth + spaceWidth;
    }

    QVector<WhereDrawn> ret;

    bool addExtraSpace = false;
    bool first = true;
    WhereDrawn firstLine;

    for (auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        switch ((*it)->type()) {
        case MD::ItemType::Heading: {
            if (heightCalcOpt != CalcHeightOpt::Minimum) {
                const auto where = drawHeading(pdfData,
                                               renderOpts,
                                               static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                                               doc,
                                               offset,
                                               (it + 1 != last ? minNecessaryHeight(pdfData, renderOpts, *(it + 1), doc, offset, scale) : 0.0),
                                               heightCalcOpt,
                                               scale,
                                               (it == item->items().cbegin() && firstInList),
                                               rtl);

                ret.append(where.first);

                if (first) {
                    firstLine = where.second;
                    first = false;
                }
            } else {
                ret.append({-1, 0.0, 0.0});
            }
        } break;

        case MD::ItemType::Paragraph: {
            const auto where = drawParagraph(pdfData,
                                             renderOpts,
                                             static_cast<MD::Paragraph<MD::QStringTrait> *>(it->get()),
                                             doc,
                                             offset,
                                             (it == item->items().cbegin() && firstInList) || it != item->items().cbegin(),
                                             heightCalcOpt,
                                             scale,
                                             Qt::black,
                                             false,
                                             rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }

            addExtraSpace = (it != item->items().cbegin());
        } break;

        case MD::ItemType::Code: {
            const auto where = drawCode(pdfData, renderOpts, static_cast<MD::Code<MD::QStringTrait> *>(
                                            it->get()), doc, offset, heightCalcOpt, scale);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }

            addExtraSpace = false;
        } break;

        case MD::ItemType::Blockquote: {
            const auto where =
                drawBlockquote(pdfData, renderOpts, static_cast<MD::Blockquote<MD::QStringTrait> *>(
                                   it->get()), doc, offset, heightCalcOpt, scale, rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }

            addExtraSpace = (it != item->items().cbegin());
        } break;

        case MD::ItemType::List: {
            auto list = static_cast<MD::List<MD::QStringTrait> *>(it->get());

            const auto where = drawList(pdfData, renderOpts, list, doc, maxListNumberWidth(list),
                                        offset, heightCalcOpt, scale, true, rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        case MD::ItemType::Table: {
            const auto where = drawTable(pdfData, renderOpts, static_cast<MD::Table<MD::QStringTrait> *>(it->get()),
                                         doc, offset, heightCalcOpt, scale, rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        default:
            break;
        }

        if (heightCalcOpt == CalcHeightOpt::Minimum) {
            break;
        }
    }

    if (addExtraSpace) {
        ret.append({pdfData.currentPageIndex(), pdfData.m_layout.y(), lineHeight});

        if (heightCalcOpt != CalcHeightOpt::Full) {
            moveToNewLine(pdfData, offset, lineHeight, 1.0, lineHeight);
        }
    }

    const auto wasRightToLeft = pdfData.m_layout.isRightToLeft();

    if (rtl && !rtl->isCheck()) {
        pdfData.m_layout.setRightToLeft(rtl->isRightToLeft());
    }

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        if (firstLine.m_pageIdx >= 0) {
            pdfData.m_currentPainterIdx = firstLine.m_pageIdx;

            if (item->isTaskList()) {
                pdfData.setColor(Qt::black);
                pdfData.drawRectangle(pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                                      (offset - (orderedListNumberWidth + spaceWidth)),
                                      firstLine.m_y + qAbs(firstLine.m_height - orderedListNumberWidth) / 2.0,
                                      orderedListNumberWidth,
                                      orderedListNumberWidth,
                                      PoDoFo::PdfPathDrawMode::Stroke);

                if (item->isChecked()) {
                    const auto d = orderedListNumberWidth * 0.2;

                    pdfData.drawRectangle(pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                                          (offset + d - (orderedListNumberWidth + spaceWidth)),
                                          firstLine.m_y + qAbs(firstLine.m_height - orderedListNumberWidth) / 2.0 + d,
                                          orderedListNumberWidth - 2.0 * d,
                                          orderedListNumberWidth - 2.0 * d,
                                          PoDoFo::PdfPathDrawMode::Fill);
                }

                pdfData.restoreColor();
            } else if (item->listType() == MD::ListItem<MD::QStringTrait>::Ordered) {
                if (prevListItemType == ListItemType::Unordered || firstItem) {
                    idx = item->startNumber();
                } else if (prevListItemType == ListItemType::Ordered) {
                    ++idx;
                }

                prevListItemType = ListItemType::Ordered;

                const QString idxText = QString::number(idx) + QLatin1Char('.');

                pdfData.drawText(pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                                 (offset - (orderedListNumberWidth + spaceWidth)),
                                 firstLine.m_y - pdfData.fontDescent(font, renderOpts.m_textFontSize, scale),
                                 createUtf8String(idxText),
                                 font,
                                 renderOpts.m_textFontSize * scale,
                                 1.0,
                                 false);
            } else {
                prevListItemType = ListItemType::Unordered;

                pdfData.setColor(Qt::black);
                const auto r = unorderedMarkWidth / 2.0;
                (*pdfData.m_painters)[pdfData.m_currentPainterIdx]->DrawCircle(
                            pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                            (offset + r - (orderedListNumberWidth + spaceWidth)),
                            firstLine.m_y + qAbs(firstLine.m_height - unorderedMarkWidth) / 2.0
                                + unorderedMarkWidth / 2.0,
                            r,
                            PoDoFo::PdfPathDrawMode::Fill);
                pdfData.restoreColor();
            }

            pdfData.m_currentPainterIdx = pdfData.currentPageIndex();
        }
    }

    pdfData.m_layout.setRightToLeft(wasRightToLeft);

    return {ret, firstLine};
}

int PdfRenderer::maxListNumberWidth(MD::List<MD::QStringTrait> *list) const
{
    int counter = 0;
    bool first = true;

    for (auto it = list->items().cbegin(), last = list->items().cend(); it != last; ++it) {
        if ((*it)->type() == MD::ItemType::ListItem) {
            auto *item = static_cast<MD::ListItem<MD::QStringTrait> *>(it->get());

            if (item->listType() == MD::ListItem<MD::QStringTrait>::Ordered) {
                if (first) {
                    counter = item->startNumber();
                } else {
                    ++counter;
                }
            }
        }

        first = false;
    }

    int ret = 0;

    while (counter) {
        counter /= 10;
        ++ret;
    }

    return ret ? ret : 1;
}

bool operator!=(const PdfRenderer::FontAttribs &f1, const PdfRenderer::FontAttribs &f2)
{
    return (f1.m_family != f2.m_family || f1.m_bold != f2.m_bold || f1.m_italic != f2.m_italic
            || f1.m_strikethrough != f2.m_strikethrough || f1.m_size != f2.m_size);
}

bool operator==(const PdfRenderer::FontAttribs &f1, const PdfRenderer::FontAttribs &f2)
{
    return (f1.m_family == f2.m_family && f1.m_bold == f2.m_bold && f1.m_italic == f2.m_italic
            && f1.m_strikethrough == f2.m_strikethrough && f1.m_size == f2.m_size);
}

void PdfRenderer::createAuxCell(const RenderOpts &renderOpts,
                                PdfAuxData &pdfData,
                                CellData &data,
                                MD::Item<MD::QStringTrait> *item,
                                std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                const QString &url,
                                const QColor &color)
{
    auto handleText = [&]() {
        auto *t = static_cast<MD::Text<MD::QStringTrait> *>(item);

        auto words = splitString(t->text(), false);

        if (t->text().isRightToLeft()) {
            orderWords(words);
        }

        for (auto it = words.begin(), last = words.end(); it != last; ++it) {
            CellItem item;

            item.m_word = it->first;
            item.m_isRightToLeft = it->second;
            item.m_font = {renderOpts.m_textFont,
                         (bool)(t->opts() & MD::TextOption::BoldText),
                         (bool)(t->opts() & MD::TextOption::ItalicText),
                         (bool)(t->opts() & MD::TextOption::StrikethroughText),
                         renderOpts.m_textFontSize};

            if (!url.isEmpty()) {
                item.m_url = url;
            }

            if (color.isValid()) {
                item.m_color = color;
            }

            data.m_items.append(item);
        }
    };

    switch (item->type()) {
    case MD::ItemType::Text:
        handleText();
        break;

    case MD::ItemType::Code: {
        auto *c = static_cast<MD::Code<MD::QStringTrait> *>(item);

        auto words = splitString(c->text(), false);

        if (data.m_isRightToLeft) {
            orderWords(words);
        }

        for (auto it = words.begin(), last = words.end(); it != last; ++it) {
            CellItem item;
            item.m_code = true;
            item.m_word = it->first;
            item.m_font = {renderOpts.m_codeFont, false, false, false, renderOpts.m_codeFontSize};
            item.m_background = renderOpts.m_syntax->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding);

            if (!url.isEmpty()) {
                item.m_url = url;
            }

            if (color.isValid()) {
                item.m_color = color;
            }

            data.m_items.append(item);
        }
    } break;

    case MD::ItemType::Link: {
        auto *l = static_cast<MD::Link<MD::QStringTrait> *>(item);

        QString url = l->url();

        const auto lit = doc->labeledLinks().find(url);

        if (lit != doc->labeledLinks().cend()) {
            url = lit->second->url();
        }

        if (!l->p()->isEmpty()) {
            for (auto pit = l->p()->items().cbegin(), plast = l->p()->items().cend(); pit != plast; ++pit) {
                createAuxCell(renderOpts, pdfData, data, pit->get(), doc, url, renderOpts.m_linkColor);
            }
        } else if (!l->img()->isEmpty()) {
            CellItem item;
            item.m_image = loadImage(l->img().get(), *pdfData.m_resvgOpts.get());
            item.m_url = url;
            item.m_font = {renderOpts.m_textFont,
                         (bool)(l->opts() & MD::TextOption::BoldText),
                         (bool)(l->opts() & MD::TextOption::ItalicText),
                         (bool)(l->opts() & MD::TextOption::StrikethroughText),
                         renderOpts.m_textFontSize};

            data.m_items.append(item);
        } else if (!l->text().isEmpty()) {
            auto words = splitString(l->text(), false);

            if (l->text().isRightToLeft()) {
                orderWords(words);
            }

            for (auto it = words.begin(), last = words.end(); it != last; ++it) {
                CellItem item;
                item.m_word = it->first;
                item.m_isRightToLeft = it->second;
                item.m_font = {renderOpts.m_textFont,
                             (bool)(l->opts() & MD::TextOption::BoldText),
                             (bool)(l->opts() & MD::TextOption::ItalicText),
                             (bool)(l->opts() & MD::TextOption::StrikethroughText),
                             renderOpts.m_textFontSize};
                item.m_url = url;
                item.m_color = renderOpts.m_linkColor;

                data.m_items.append(item);
            }
        } else {
            CellItem item;
            item.m_font = {renderOpts.m_textFont,
                         (bool)(l->opts() & MD::TextOption::BoldText),
                         (bool)(l->opts() & MD::TextOption::ItalicText),
                         (bool)(l->opts() & MD::TextOption::StrikethroughText),
                         renderOpts.m_textFontSize};
            item.m_url = url;
            item.m_color = renderOpts.m_linkColor;

            data.m_items.append(item);
        }
    } break;

    case MD::ItemType::Image: {
        auto *i = static_cast<MD::Image<MD::QStringTrait> *>(item);

        CellItem item;

        emit status(tr("Loading image."));

        item.m_image = loadImage(i, *pdfData.m_resvgOpts.get());
        item.m_font = {renderOpts.m_textFont, false, false, false, renderOpts.m_textFontSize};

        if (!url.isEmpty()) {
            item.m_url = url;
        }

        data.m_items.append(item);
    } break;

    case MD::ItemType::FootnoteRef: {
        auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>(item);

        const auto fit = doc->footnotesMap().find(ref->id());

        if (fit != doc->footnotesMap().cend()) {
            CellItem item;
            item.m_font = {renderOpts.m_textFont, false, false, false, renderOpts.m_textFontSize};

            auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());
            int num = m_footnoteNum;

            if (anchorIt == pdfData.m_footnotesAnchorsMap.cend()) {
                pdfData.m_footnotesAnchorsMap.insert(fit->second.get(), {pdfData.m_currentFile, m_footnoteNum++});
            } else {
                num = anchorIt->second;
            }

            item.m_footnote = QString::number(num);
            item.m_footnoteRef = ref->id();
            item.m_footnoteObj = fit->second;

            if (!url.isEmpty()) {
                item.m_url = url;
            }

            if (color.isValid()) {
                item.m_color = color;
            }

            data.m_items.append(item);
        } else {
            handleText();
        }
    } break;

    default:
        break;
    }
}

QVector<QVector<PdfRenderer::CellData>> PdfRenderer::createAuxTable(PdfAuxData &pdfData,
                                                                    const RenderOpts &renderOpts,
                                                                    MD::Table<MD::QStringTrait> *item,
                                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                    double scale)
{
    Q_UNUSED(pdfData)
    Q_UNUSED(scale)

    const auto columnsCount = item->columnsCount();

    QVector<QVector<CellData>> auxTable;
    auxTable.resize(columnsCount);

    for (auto rit = item->rows().cbegin(), rlast = item->rows().cend(); rit != rlast; ++rit) {
        int i = 0;

        for (auto cit = (*rit)->cells().cbegin(), clast = (*rit)->cells().cend(); cit != clast; ++cit) {
            if (i == columnsCount) {
                break;
            }

            CellData data;
            data.m_alignment = item->columnAlignment(i);
            data.m_isRightToLeft = ((*cit)->isEmpty() ? false : isRightToLeft((*cit)->items().front().get()));

            for (auto it = (*cit)->items().cbegin(), last = (*cit)->items().cend(); it != last; ++it) {
                createAuxCell(renderOpts, pdfData, data, it->get(), doc);
            }

            auxTable[i].append(data);

            ++i;
        }

        for (; i < columnsCount; ++i) {
            auxTable[i].append(CellData());
        }
    }

    return auxTable;
}

void PdfRenderer::calculateCellsSize(PdfAuxData &pdfData,
                                     QVector<QVector<CellData>> &auxTable,
                                     double spaceWidth,
                                     double offset,
                                     double lineHeight,
                                     double scale)
{
    QVector<double> columnWidthes;
    columnWidthes.resize(auxTable.size());

    const auto availableWidth = pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left
            - pdfData.m_layout.margins().m_right - offset;

    const auto width = availableWidth / auxTable.size();

    for (auto it = auxTable.begin(), last = auxTable.end(); it != last; ++it) {
        for (auto cit = it->begin(), clast = it->end(); cit != clast; ++cit) {
            cit->setWidth(width - s_tableMargin * 2.0);
        }
    }

    for (auto it = auxTable.begin(), last = auxTable.end(); it != last; ++it) {
        for (auto cit = it->begin(), clast = it->end(); cit != clast; ++cit) {
            cit->heightToWidth(lineHeight, spaceWidth, scale, pdfData, this);
        }
    }
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawTable(PdfAuxData &pdfData,
                                                              const RenderOpts &renderOpts,
                                                              MD::Table<MD::QStringTrait> *item,
                                                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                              double offset,
                                                              CalcHeightOpt heightCalcOpt,
                                                              double scale,
                                                              RTLFlag *rtl)
{
    QVector<WhereDrawn> ret;

    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate) {
            return {};
        }
    }

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        emit status(tr("Drawing table."));
    }

    auto *font = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize,
                            pdfData.m_doc, scale, pdfData);

    const auto lineHeight = pdfData.lineSpacing(font, renderOpts.m_textFontSize, scale);
    const auto spaceWidth = pdfData.stringWidth(font, renderOpts.m_textFontSize, scale, " ");

    auto auxTable = createAuxTable(pdfData, renderOpts, item, doc, scale);

    calculateCellsSize(pdfData, auxTable, spaceWidth, offset, lineHeight, scale);

    const auto r0h = rowHeight(auxTable, 0);
    const bool justHeader = auxTable.at(0).size() == 1;
    const auto r1h = (!justHeader ? rowHeight(auxTable, 1) : 0);

    switch (heightCalcOpt) {
    case CalcHeightOpt::Minimum: {
        ret.append({-1,
                    0.0,
                    r0h + r1h + (s_tableMargin * (justHeader ? 2.0 : 4.0)) + lineHeight
                        - (pdfData.fontDescent(font, renderOpts.m_textFontSize, scale) * (justHeader ? 1.0 : 2.0))});

        return {ret, {}};
    }

    case CalcHeightOpt::Full: {
        ret.append({-1,
                    0.0,
                    r0h + r1h + (s_tableMargin * (justHeader ? 2.0 : 4.0)) + lineHeight
                        - (pdfData.fontDescent(font, renderOpts.m_textFontSize, scale) * (justHeader ? 1.0 : 2.0))});

        for (int i = 2; i < auxTable.at(0).size(); ++i) {
            ret.append({-1, 0.0, rowHeight(auxTable, i) + s_tableMargin * 2.0
                        - pdfData.fontDescent(font, renderOpts.m_textFontSize, scale)});
        }

        return {ret, {}};
    }

    default:
        break;
    }

    const auto nonSplittableHeight =
        (r0h + r1h + (s_tableMargin * (justHeader ? 2.0 : 4.0))
         - (pdfData.fontDescent(font, renderOpts.m_textFontSize, scale) * (justHeader ? 1.0 : 2.0)));

    if (pdfData.m_layout.y() - nonSplittableHeight < pdfData.currentPageAllowedY()
        && qAbs(pdfData.m_layout.y() - nonSplittableHeight - pdfData.currentPageAllowedY()) > 0.1) {
        createPage(pdfData);

        pdfData.freeSpaceOn(pdfData.currentPageIndex());
    }

    moveToNewLine(pdfData, offset, lineHeight, 1.0, lineHeight);

    QVector<QPair<QString, std::shared_ptr<MD::Footnote<MD::QStringTrait>>>> footnotes;
    bool first = true;
    WhereDrawn firstLine;

    const auto wasRightToLeft = pdfData.m_layout.isRightToLeft();
    pdfData.m_layout.setRightToLeft(auxTable[0][0].m_isRightToLeft);

    for (int row = 0; row < auxTable[0].size(); ++row) {
        const auto where = drawTableRow(auxTable, row, pdfData, offset, lineHeight, renderOpts, doc, footnotes, scale);

        ret.append(where.first);

        if (first) {
            firstLine = where.second;
            first = false;
        }
    }

    pdfData.m_layout.setRightToLeft(wasRightToLeft);

    for (const auto &f : std::as_const(footnotes)) {
        addFootnote(f.first, f.second, pdfData, renderOpts, doc);
    }

    return {ret, firstLine};
}

void PdfRenderer::addFootnote(const QString &refId,
                              std::shared_ptr<MD::Footnote<MD::QStringTrait>> f,
                              PdfAuxData &pdfData,
                              const RenderOpts &renderOpts,
                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc)
{
    std::function<void(MD::Block<MD::QStringTrait> * b, PdfAuxData & pdfData)> findFootnoteRefs;

    findFootnoteRefs = [&](MD::Block<MD::QStringTrait> *b, PdfAuxData &pdfData) {
        for (auto it = b->items().cbegin(), last = b->items().cend(); it != last; ++it) {
            auto cb = dynamic_cast<MD::Block<MD::QStringTrait> *>(it->get());

            if (cb) {
                findFootnoteRefs(cb, pdfData);
            } else {
                switch ((*it)->type()) {
                case MD::ItemType::Heading:
                    findFootnoteRefs(static_cast<MD::Heading<MD::QStringTrait> *>(it->get())->text().get(), pdfData);
                    break;

                case MD::ItemType::Table: {
                    auto t = static_cast<MD::Table<MD::QStringTrait> *>(it->get());

                    for (const auto &r : t->rows()) {
                        for (const auto &c : r->cells()) {
                            findFootnoteRefs(c.get(), pdfData);
                        }
                    }
                } break;

                case MD::ItemType::FootnoteRef: {
                    auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>(it->get());

                    const auto fit = doc->footnotesMap().find(ref->id());

                    if (fit != doc->footnotesMap().cend()) {
                        this->addFootnote(ref->id(), fit->second, pdfData, renderOpts, doc);
                    }
                } break;

                default:
                    break;
                }
            }
        }
    };

    if (std::find_if(m_footnotes.cbegin(),
                     m_footnotes.cend(),
                     [&refId](const auto &p) {
                         return p.first == refId;
                     })
        == m_footnotes.cend()) {
        m_footnotes.append({refId, f});

        PdfAuxData tmpData = pdfData;
        tmpData.m_layout.m_coords = {{pdfData.m_layout.margins().m_left, pdfData.m_layout.margins().m_right,
                             pdfData.m_layout.margins().m_top, pdfData.m_layout.margins().m_bottom},
                          pdfData.m_page->GetRect().Width,
                          pdfData.m_page->GetRect().Height,
                          0.0,
                          pdfData.m_page->GetRect().Height - pdfData.m_layout.margins().m_top};
        tmpData.m_layout.moveXToBegin();

        double lineHeight = 0.0;
        auto h = footnoteHeight(tmpData, renderOpts, doc, f.get(), &lineHeight);

        reserveSpaceForFootnote(pdfData, renderOpts, h, pdfData.m_layout.y(), pdfData.m_currentPageIdx, lineHeight);

        pdfData.m_footnotesAnchorsMap = tmpData.m_footnotesAnchorsMap;

        findFootnoteRefs(f.get(), pdfData);
    }
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawTableRow(QVector<QVector<CellData>> &table,
                                                                 int row,
                                                                 PdfAuxData &pdfData,
                                                                 double offset,
                                                                 double lineHeight,
                                                                 const RenderOpts &renderOpts,
                                                                 std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                 QVector<QPair<QString, std::shared_ptr<MD::Footnote<MD::QStringTrait>>>> &footnotes,
                                                                 double scale)
{
    QVector<WhereDrawn> ret;

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate) {
            return {};
        }
    }

    emit status(tr("Drawing table row."));

    auto *textFont = createFont(renderOpts.m_textFont, false, false, renderOpts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    const auto startPage = pdfData.currentPageIndex();
    const auto startY = pdfData.m_layout.y();
    auto endPage = startPage;
    auto endY = startY;
    int currentPage = startPage;
    const auto firstLinePageIdx = startPage;
    const auto firstLineHeight = lineHeight;
    const auto firstLineY = startY - firstLineHeight - s_tableMargin;

    TextToDraw text;
    QMap<QString, QVector<QPair<QRectF, unsigned int>>> links;

    int column = 0;

    // Draw cells.
    for (auto it = table.cbegin(), last = table.cend(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        emit status(tr("Drawing table cell."));

        text.m_alignment = it->at(0).m_alignment;
        text.m_availableWidth = it->at(0).m_width;
        text.m_lineHeight = lineHeight;

        pdfData.m_currentPainterIdx = startPage;

        currentPage = startPage;

        const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());
        auto startX = (pdfData.m_layout.isRightToLeft() ? pdfData.m_layout.rightBorderXWithOffset() :
                                                          pdfData.m_layout.leftBorderXWithOffset());
        text.m_isRightToLeft = it->at(row).m_isRightToLeft;

        for (int i = 0; i < column; ++i) {
            startX += pdfData.m_layout.xIncrementDirection() * (table[i][0].m_width + s_tableMargin * 2.0);
        }

        if (pdfData.m_layout.isRightToLeft() != it->at(row).m_isRightToLeft) {
            startX += pdfData.m_layout.xIncrementDirection() * (table[column][0].m_width + s_tableMargin * 2.0);
        }

        startX += pdfData.m_layout.xIncrementDirection() * s_tableMargin;

        double x = startX;
        double y = startY - s_tableMargin;

        if (y < pdfData.currentPageAllowedY() && qAbs(y - pdfData.currentPageAllowedY()) > 0.1) {
            newPageInTable(pdfData, currentPage, endPage, endY);

            y = pdfData.topY(currentPage);

            if (pdfData.m_drawFootnotes) {
                y -= pdfData.m_extraInFootnote;
            }
        }

        bool textBefore = false;
        const CellItem *lastItemInCell = it->at(row).m_items.isEmpty() ? nullptr : &(it->at(row).m_items.back());
        const bool wasTextInLastPos = lastItemInCell
            ? (!lastItemInCell->m_word.isEmpty() || (lastItemInCell->m_image.isEmpty()
                                                     && !lastItemInCell->m_url.isEmpty())
               || !lastItemInCell->m_footnote.isEmpty())
            : false;

        bool addMargin = false;

        for (auto c = it->at(row).m_items.cbegin(), clast = it->at(row).m_items.cend(); c != clast; ++c) {
            if (!c->m_image.isNull() && !text.m_text.isEmpty()) {
                drawTextLineInTable(renderOpts, x, y, text, lineHeight, pdfData, links,
                                    textFont, currentPage, endPage, endY, footnotes, scale);
                addMargin = false;
            }

            if (!c->m_image.isNull()) {
                if (textBefore) {
                    y -= lineHeight;
                }

                if (addMargin) {
                    y -= s_tableMargin;
                }

                auto img = pdfData.m_doc->CreateImage();
                img->LoadFromBuffer({c->m_image.data(), static_cast<size_t>(c->m_image.size())});

                const double iWidth = std::round((double)img->GetWidth() / (double)pdfData.m_dpi * 72.0);
                const double iHeight = std::round((double)img->GetHeight() / (double)pdfData.m_dpi * 72.0);
                const double dpiScale = (double)img->GetWidth() / iWidth;

                auto ratio = (iWidth > it->at(0).m_width ? it->at(0).m_width / iWidth * scale : 1.0 * scale);

                auto h = iHeight * ratio;

                if (y - h < pdfData.currentPageAllowedY() && qAbs(y - h - pdfData.currentPageAllowedY()) > 0.1) {
                    newPageInTable(pdfData, currentPage, endPage, endY);

                    y = pdfData.topY(currentPage);

                    if (pdfData.m_drawFootnotes) {
                        y -= pdfData.m_extraInFootnote;
                    }
                }

                const auto availableHeight = pdfData.topY(currentPage) - pdfData.currentPageAllowedY();

                if (h > availableHeight) {
                    ratio = availableHeight / iHeight;
                }

                const auto w = iWidth * ratio;
                auto o = 0.0;

                if (w < table[column][0].m_width) {
                    switch (table[column][0].m_alignment) {
                    case MD::Table<MD::QStringTrait>::AlignLeft:
                        o = (it->at(row).m_isRightToLeft ? table[column][0].m_width - w : 0.0);
                        break;

                    case MD::Table<MD::QStringTrait>::AlignCenter:
                        o = (table[column][0].m_width - w) / 2.0;
                        break;

                    case MD::Table<MD::QStringTrait>::AlignRight:
                        o = (it->at(row).m_isRightToLeft ? 0.0 : table[column][0].m_width - w);
                        break;

                    default:
                        break;
                    }
                }

                y -= iHeight * ratio;

                pdfData.drawImage(x + (it->at(row).m_isRightToLeft ? -1.0 : 1.0) * o, y,
                                  img.get(), ratio / dpiScale, ratio / dpiScale);

                if (!c->m_url.isEmpty()) {
                    links[c->m_url].append(qMakePair(QRectF(x + (it->at(row).m_isRightToLeft ? -1.0 : 1.0) * o, y,
                                                            c->width(pdfData, this, scale), iHeight * ratio),
                                                   currentPage));
                }

                textBefore = false;
                addMargin = true;
            } else {
                addMargin = false;

                auto *font = createFont(c->m_font.m_family, c->m_font.m_bold, c->m_font.m_italic,
                                        c->m_font.m_size, pdfData.m_doc, scale, pdfData);

                auto w = pdfData.stringWidth(font, c->m_font.m_size, scale, createUtf8String(
                                                 c->m_word.isEmpty() ? c->m_url : c->m_word));

                double fw = 0.0;

                if (c + 1 != clast && !(c + 1)->m_footnote.isEmpty()) {
                    auto *f1 = createFont((c + 1)->m_font.m_family, (c + 1)->m_font.m_bold, (c + 1)->m_font.m_italic,
                                          (c + 1)->m_font.m_size, pdfData.m_doc, scale, pdfData);

                    fw = pdfData.stringWidth(f1, (c + 1)->m_font.m_size, scale, createUtf8String((c + 1)->m_footnote));
                    w += fw;
                }

                if (text.m_width + w < it->at(0).m_width || qAbs(text.m_width + w - it->at(0).m_width) < 0.01) {
                    text.m_text.append(*c);

                    if (c + 1 != clast && !(c + 1)->m_footnote.isEmpty()) {
                        text.m_text.append(*(c + 1));
                    }

                    text.m_width += w;
                } else {
                    if (!text.m_text.isEmpty()) {
                        drawTextLineInTable(renderOpts, x, y, text, lineHeight, pdfData, links, textFont, currentPage, endPage, endY, footnotes, scale);
                        text.m_text.append(*c);

                        if (c + 1 != clast && !(c + 1)->m_footnote.isEmpty()) {
                            text.m_text.append(*(c + 1));
                        }

                        text.m_width += w;
                    } else {
                        text.m_text.append(*c);
                        text.m_width += w;
                        drawTextLineInTable(renderOpts, x, y, text, lineHeight, pdfData, links, textFont, currentPage, endPage, endY, footnotes, scale);

                        if (c + 1 != clast && !(c + 1)->m_footnote.isEmpty()) {
                            text.m_text.append(*(c + 1));
                            text.m_width += fw;
                        }
                    }
                }

                if (c + 1 != clast && !(c + 1)->m_footnote.isEmpty()) {
                    ++c;
                }

                textBefore = true;
            }
        }

        if (!text.m_text.isEmpty()) {
            drawTextLineInTable(renderOpts, x, y, text, lineHeight, pdfData, links, textFont,
                                currentPage, endPage, endY, footnotes, scale);
        }

        y -= s_tableMargin - (wasTextInLastPos ? pdfData.fontDescent(textFont, renderOpts.m_textFontSize, scale) : 0.0);

        if (y < endY && currentPage == pdfData.currentPageIndex()) {
            endY = y;
        }

        ++column;
    }

    drawRowBorder(pdfData, startPage, ret, renderOpts, offset, table, startY, endY);

    pdfData.m_layout.setY(endY);
    pdfData.m_currentPainterIdx = pdfData.currentPageIndex();

    processLinksInTable(pdfData, links, doc);

    return {ret, {firstLinePageIdx, firstLineY, firstLineHeight}};
}

void PdfRenderer::drawRowBorder(PdfAuxData &pdfData,
                                int startPage,
                                QVector<WhereDrawn> &ret,
                                const RenderOpts &renderOpts,
                                double offset,
                                const QVector<QVector<CellData>> &table,
                                double startY,
                                double endY)
{
    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

    for (int i = startPage; i <= pdfData.currentPageIndex(); ++i) {
        pdfData.m_currentPainterIdx = i;

        pdfData.setColor(renderOpts.m_borderColor);

        auto startX = (pdfData.m_layout.isRightToLeft() ? pdfData.m_layout.rightBorderXWithOffset() :
                                                          pdfData.m_layout.leftBorderXWithOffset());
        auto endX = startX;

        for (int c = 0; c < table.size(); ++c) {
            endX += pdfData.m_layout.xIncrementDirection() * (table.at(c).at(0).m_width + s_tableMargin * 2.0);
        }

        if (i == startPage) {
            pdfData.drawLine(startX, startY, endX, startY);

            auto x = startX;
            auto y = endY;

            if (i == pdfData.currentPageIndex()) {
                pdfData.drawLine(startX, endY, endX, endY);
                pdfData.drawLine(x, startY, x, endY);
            } else {
                pdfData.drawLine(x, startY, x, pdfData.allowedY(i));
                y = pdfData.allowedY(i);
            }

            for (int c = 0; c < table.size(); ++c) {
                x += pdfData.m_layout.xIncrementDirection() * (table.at(c).at(0).m_width + s_tableMargin * 2.0);

                pdfData.drawLine(x, startY, x, y);
            }

            ret.append({i,
                        (i < pdfData.currentPageIndex() ? pdfData.allowedY(i) : endY),
                        (i < pdfData.currentPageIndex() ? startY - pdfData.allowedY(i) : startY - endY)});
        } else if (i < pdfData.currentPageIndex()) {
            auto x = startX;
            auto y = pdfData.allowedY(i);
            auto sy = pdfData.topY(i);

            if (pdfData.m_drawFootnotes) {
                sy -= pdfData.m_extraInFootnote + s_tableMargin;
            }

            pdfData.drawLine(x, sy, x, y);

            for (int c = 0; c < table.size(); ++c) {
                x += pdfData.m_layout.xIncrementDirection() * (table.at(c).at(0).m_width + s_tableMargin * 2.0);

                pdfData.drawLine(x, sy, x, y);
            }

            ret.append({i, pdfData.allowedY(i), sy - pdfData.allowedY(i)});
        } else {
            auto x = startX;
            auto y = endY;
            auto sy = pdfData.topY(i);

            if (pdfData.m_drawFootnotes) {
                sy -= pdfData.m_extraInFootnote + s_tableMargin;
            }

            pdfData.drawLine(x, sy, x, y);

            for (int c = 0; c < table.size(); ++c) {
                x += pdfData.m_layout.xIncrementDirection() * (table.at(c).at(0).m_width + s_tableMargin * 2.0);

                pdfData.drawLine(x, sy, x, y);
            }

            pdfData.drawLine(startX, y, endX, y);

            ret.append({pdfData.currentPageIndex(), endY, sy - endY});
        }

        pdfData.restoreColor();
    }
}

void PdfRenderer::drawTextLineInTable(const RenderOpts &renderOpts,
                                      double x,
                                      double &y,
                                      TextToDraw &text,
                                      double lineHeight,
                                      PdfAuxData &pdfData,
                                      QMap<QString, QVector<QPair<QRectF, unsigned int>>> &links,
                                      Font *font,
                                      int &currentPage,
                                      int &endPage,
                                      double &endY,
                                      QVector<QPair<QString, std::shared_ptr<MD::Footnote<MD::QStringTrait>>>> &footnotes,
                                      double scale)
{
    auto removeIfSpace = [this, &pdfData, scale](TextToDraw &text, qsizetype idx) {
        if (!text.m_text[idx].m_code && text.m_text[idx].m_word == QStringLiteral(" ")) {
            text.m_width -= text.m_text[idx].width(pdfData, this, scale);

            text.m_text.remove(idx);
        }
    };

    if (!text.m_text.isEmpty()) {
        removeIfSpace(text, 0);
    }

    if (!text.m_text.isEmpty()) {
        removeIfSpace(text, text.m_text.size() - 1);
    }

    if (text.m_text.isEmpty()) {
        return;
    }

    y -= lineHeight;

    if (y < pdfData.allowedY(currentPage)) {
        newPageInTable(pdfData, currentPage, endPage, endY);

        y = pdfData.topY(currentPage) - lineHeight;

        if (pdfData.m_drawFootnotes) {
            y -= pdfData.m_extraInFootnote;
        }
    }

    if (text.m_width < text.m_availableWidth || qAbs(text.m_width - text.m_availableWidth) < 0.01) {
        switch (text.m_alignment) {
        case MD::Table<MD::QStringTrait>::AlignRight:
            x += (text.m_isRightToLeft ? 0.0 : text.m_availableWidth - text.m_width);
            break;

        case MD::Table<MD::QStringTrait>::AlignCenter:
            x += (text.m_isRightToLeft ? -1.0 : 1.0) * ((text.m_availableWidth - text.m_width) / 2.0);
            break;

        case MD::Table<MD::QStringTrait>::AlignLeft:
            x -= (text.m_isRightToLeft ? text.m_availableWidth - text.m_width : 0.0);
            break;

        default:
            break;
        }
    } else {
        const auto str = (text.m_text.first().m_word.isEmpty() ? text.m_text.first().m_url : text.m_text.first().m_word);

        QString res;

        double w = 0.0;

        auto *f = createFont(text.m_text.first().m_font.m_family,
                             text.m_text.first().m_font.m_bold,
                             text.m_text.first().m_font.m_italic,
                             text.m_text.first().m_font.m_size,
                             pdfData.m_doc,
                             scale,
                             pdfData);

        for (const auto &ch : str) {
            w += pdfData.stringWidth(f, text.m_text.first().m_font.m_size, scale, createUtf8String(QString(ch)));

            if (w >= text.m_availableWidth) {
                break;
            } else {
                res.append(ch);
            }
        }

        text.m_text.first().m_word = res;
    }

    for (auto it = text.m_text.begin(), last = text.m_text.end(); it != last; ++it) {
        auto *f = createFont(it->m_font.m_family, it->m_font.m_bold, it->m_font.m_italic,
                             it->m_font.m_size, pdfData.m_doc, scale, pdfData);

        const auto w = it->width(pdfData, this, scale);

        if (it->m_background.isValid()) {
            pdfData.setColor(it->m_background);

            pdfData.drawRectangle(x - (text.m_isRightToLeft ? w : 0.0),
                                  y + pdfData.fontDescent(f, it->m_font.m_size, scale),
                                  w,
                                  pdfData.lineSpacing(f, it->m_font.m_size, scale),
                                  PoDoFo::PdfPathDrawMode::Fill);

            pdfData.restoreColor();
        }

        if (it->m_color.isValid()) {
            pdfData.setColor(it->m_color);
        }

        if (it->m_isRightToLeft) {
            std::reverse(it->m_word.begin(), it->m_word.end());
        }

        pdfData.drawText(x - (text.m_isRightToLeft ? w : 0.0), y,
                         createUtf8String(it->m_word.isEmpty() ? it->m_url : it->m_word), f,
                         it->m_font.m_size * scale, 1.0, it->m_font.m_strikethrough);

        pdfData.restoreColor();

        if (!it->m_url.isEmpty()) {
            links[it->m_url].append(qMakePair(QRectF(x - (text.m_isRightToLeft ? w : 0.0), y, w, lineHeight), currentPage));
        }

        x += (text.m_isRightToLeft ? -1.0 : 1.0) * w;

        if (it + 1 != last && !(it + 1)->m_footnote.isEmpty()) {
            ++it;

            const auto str = createUtf8String(it->m_footnote);

            const auto w = pdfData.stringWidth(f, it->m_font.m_size * s_footnoteScale, scale, str);

            m_unresolvedFootnotesLinks.insert(it->m_footnoteRef, qMakePair(QRectF(x, y, w, lineHeight),
                                                                         pdfData.currentPageIndex()));

            pdfData.setColor(renderOpts.m_linkColor);

            pdfData.drawText(x - (text.m_isRightToLeft ? w : 0.0),
                             y + lineHeight - pdfData.lineSpacing(f, it->m_font.m_size * s_footnoteScale, scale),
                             str,
                             f,
                             it->m_font.m_size * s_footnoteScale * scale,
                             1.0,
                             false);

            pdfData.restoreColor();

            x += (text.m_isRightToLeft ? -1.0 : 1.0) * w;

            footnotes.append({it->m_footnoteRef, it->m_footnoteObj});
        }
    }

    text.clear();
}

void PdfRenderer::newPageInTable(PdfAuxData &pdfData, int &currentPage, int &endPage, double &endY)
{
    if (currentPage + 1 > pdfData.currentPageIndex()) {
        createPage(pdfData);

        if (pdfData.currentPageIndex() > endPage) {
            endPage = pdfData.currentPageIndex();
            endY = pdfData.m_layout.y();
        }

        ++currentPage;
    } else {
        ++currentPage;

        pdfData.m_currentPainterIdx = currentPage;
    }
}

void PdfRenderer::processLinksInTable(PdfAuxData &pdfData,
                                      const QMap<QString, QVector<QPair<QRectF, unsigned int>>> &links,
                                      std::shared_ptr<MD::Document<MD::QStringTrait>> doc)
{
    for (auto it = links.cbegin(), last = links.cend(); it != last; ++it) {
        QString url = it.key();

        const auto lit = doc->labeledLinks().find(url);

        if (lit != doc->labeledLinks().cend()) {
            url = lit->second->url();
        }

        auto tmp = it.value();

        if (!tmp.isEmpty()) {
            QVector<QPair<QRectF, unsigned int>> rects;
            QPair<QRectF, unsigned int> r = tmp.first();

            for (auto rit = tmp.cbegin() + 1, rlast = tmp.cend(); rit != rlast; ++rit) {
                if (r.second == rit->second && qAbs(r.first.x() + r.first.width() - rit->first.x()) < 0.001
                    && qAbs(r.first.y() - rit->first.y()) < 0.001) {
                    r.first.setWidth(r.first.width() + rit->first.width());
                } else {
                    rects.append(r);
                    r = *rit;
                }
            }

            rects.append(r);

            if (!pdfData.m_anchors.contains(url) && pdfData.m_md->labeledHeadings().find(url) == pdfData.m_md->labeledHeadings().cend()) {
                for (const auto &r : std::as_const(rects)) {
                    auto &annot = pdfData.m_doc->GetPages()
                                      .GetPageAt(static_cast<unsigned int>(r.second))
                                      .GetAnnotations()
                                      .CreateAnnot<PoDoFo::PdfAnnotationLink>(Rect(r.first.x(), r.first.y(), r.first.width(), r.first.height()));
                    annot.SetBorderStyle(0.0, 0.0, 0.0);

                    auto action = pdfData.m_doc->CreateAction<PoDoFo::PdfActionURI>();
                    action->SetURI(PoDoFo::PdfString(std::string(url.toLatin1().data())));

                    annot.SetAction(*action.get());
                }
            } else {
                m_unresolvedLinks.insert(url, rects);
            }
        }
    }
}

double PdfRenderer::minNecessaryHeight(PdfAuxData &pdfData,
                                       const RenderOpts &renderOpts,
                                       std::shared_ptr<MD::Item<MD::QStringTrait>> item,
                                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                       double offset,
                                       double scale)
{
    QVector<WhereDrawn> ret;

    PdfAuxData tmp = pdfData;
    tmp.m_layout.setY(tmp.m_layout.topY());
    const auto autoOffset = tmp.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());
    tmp.m_layout.moveXToBegin();

    switch (item->type()) {
    case MD::ItemType::Heading:
        return 0.0;

    case MD::ItemType::Paragraph: {
        ret =
            drawParagraph(tmp, renderOpts, static_cast<MD::Paragraph<MD::QStringTrait> *>(item.get()), doc, offset, true, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::Code: {
        ret = drawCode(tmp, renderOpts, static_cast<MD::Code<MD::QStringTrait> *>(item.get()), doc, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::Blockquote: {
        ret = drawBlockquote(tmp, renderOpts, static_cast<MD::Blockquote<MD::QStringTrait> *>(item.get()), doc, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::List: {
        auto *list = static_cast<MD::List<MD::QStringTrait> *>(item.get());
        const auto bulletWidth = maxListNumberWidth(list);

        ret = drawList(tmp, m_opts, list, m_doc, bulletWidth, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::Table: {
        ret = drawTable(tmp, renderOpts, static_cast<MD::Table<MD::QStringTrait> *>(item.get()), doc, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    default:
        break;
    }

    if (!ret.isEmpty()) {
        return ret.constFirst().m_height;
    } else {
        return 0.0;
    }
}

} /* namespace Render */

} /* namespace MdPdf */
