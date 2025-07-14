/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "renderer.h"
#include "const.h"
#include "podofo_paintdevice.h"

// shared include.
#include "utils.h"

#ifdef MD_PDF_TESTING
#include <QtTest/QtTest>
#include <test_const.h>
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
#include <QScopedValueRollback>

// MicroTeX include.
#include <platform/qt/graphic_qt.h>
#include <latex.h>
#include <utils/exceptions.h>

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

const double PdfRenderer::PrevBaselineStateStack::s_scale = 1.5;
const double PdfRenderer::PrevBaselineStateStack::s_baselineScale = 0.5;

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

double PdfRenderer::CustomWidth::firstLineHeight() const
{
    if (!m_height.isEmpty()) {
        return m_height.first();
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

//
// PdfRenderer
//

PdfRenderer::PdfRenderer()
    : m_terminate(false)
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
        document.GetMetadata().SetCreator(PoDoFo::PdfString("This PDF was generated with Markdown Tools\n"
                                          "https://github.com/igormironchik/markdown-tools"));
        std::vector<std::shared_ptr<Painter>> painters;

        pdfData.m_doc = &document;
        pdfData.m_painters = &painters;

        pdfData.m_layout.margins().m_left = m_opts.m_left;
        pdfData.m_layout.margins().m_right = m_opts.m_right;
        pdfData.m_layout.margins().m_top = m_opts.m_top;
        pdfData.m_layout.margins().m_bottom = m_opts.m_bottom;
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

        pdfData.m_lineHeight = pdfData.lineSpacing(createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize,
            pdfData.m_doc, 1.0, pdfData), m_opts.m_textFontSize, 1.0);
        pdfData.m_extraInFootnote = pdfData.m_lineHeight / 3.0;

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

                if (m_terminate) {
                    break;
                }
            }

            switch ((*it)->type()) {
            case MD::ItemType::Heading:
                drawHeading(pdfData,
                            static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                            m_doc,
                            0.0,
                            // If there is another item after heading we need to know its min
                            // height to glue heading with it.
                            (it + 1 != last ? minNecessaryHeight(pdfData, *(it + 1), m_doc, 0.0, 1.0) : 0.0),
                            CalcHeightOpt::Unknown,
                            1.0,
                            true, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::Paragraph:
                drawParagraph(pdfData, static_cast<MD::Paragraph<MD::QStringTrait> *>(it->get()),
                              m_doc, 0.0, true, CalcHeightOpt::Unknown, 1.0, Qt::black, false, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::Code:
                drawCode(pdfData, static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                         m_doc, 0.0, CalcHeightOpt::Unknown, 1.0);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::Blockquote:
                drawBlockquote(pdfData, static_cast<MD::Blockquote<MD::QStringTrait> *>(it->get()),
                               m_doc, 0.0, CalcHeightOpt::Unknown, 1.0, &rtl);
                resetRTLFlagToDefaults(&rtl);
                break;

            case MD::ItemType::List: {
                auto *list = static_cast<MD::List<MD::QStringTrait> *>(it->get());
                const auto bulletWidth = maxListNumberWidth(list);

                drawList(pdfData, list, m_doc, bulletWidth, 0.0, CalcHeightOpt::Unknown, 1.0, false, &rtl);
                resetRTLFlagToDefaults(&rtl);
            } break;

            case MD::ItemType::Table:
                drawTable(pdfData, static_cast<MD::Table<MD::QStringTrait> *>(it->get()),
                          m_doc, 0.0, CalcHeightOpt::Unknown, 1.0);
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

        pdfData.m_lineHeight *= s_footnoteScale;

        if (!m_footnotes.isEmpty()) {
            pdfData.m_drawFootnotes = true;
            pdfData.m_layout.moveXToBegin();
            pdfData.m_layout.setY(pdfData.topFootnoteY(pdfData.m_reserved.firstKey()) - pdfData.m_extraInFootnote);

            pdfData.m_currentPainterIdx = pdfData.m_reserved.firstKey();
            pdfData.m_footnotePageIdx = pdfData.m_reserved.firstKey();

            drawHorizontalLine(pdfData);

            for (const auto &f : std::as_const(m_footnotes)) {
                drawFootnote(pdfData, m_doc, f.first, f.second.get(), CalcHeightOpt::Unknown);
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
    } catch (const tex::ex_tex &e) {
        handleException(pdfData, QString::fromLatin1("Error during drawing LaTeX math: %1").arg(e.what()));
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
                             .arg(msg,
                                  pdfData.m_currentFile,
                                  QString::number(pdfData.m_startPos + 1),
                                  QString::number(pdfData.m_startLine + 1),
                                  QString::number(pdfData.m_endPos + 1),
                                  QString::number(pdfData.m_endLine + 1));

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
    PoDoFo::PdfFontStyle style = PoDoFo::PdfFontStyle::Regular;

    if (bold) {
        style |= PoDoFo::PdfFontStyle::Bold;
    }
    if (italic) {
        style |= PoDoFo::PdfFontStyle::Italic;
    }

    params.Style = style;

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
        } else if (pdfData.m_tableDrawing) {
            pdfData.m_cachedPainters.insert(pdfData.m_currentPainterIdx, 0);
        }
    };

    auto createOrUseExisting = [&]() {
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

                if (pdfData.m_tableDrawing) {
                    pdfData.m_cachedPainters.insert(pdfData.m_currentPainterIdx, 0);
                }
            } else {
                create(pdfData);

                pdfData.m_layout.setY(pdfData.topFootnoteY(pdfData.m_footnotePageIdx));
            }

            pdfData.m_layout.addY(pdfData.m_extraInFootnote);

            drawHorizontalLine(pdfData);

            if (pdfData.m_continueParagraph) {
                pdfData.m_layout.addY(pdfData.m_lineHeight);
            }

            pdfData.m_firstOnPage = true;
        }
    };

    if (!pdfData.m_tableDrawing) {
        createOrUseExisting();
    } else {
        auto it = pdfData.m_cachedPainters.find(pdfData.m_currentPainterIdx);

        if (it != pdfData.m_cachedPainters.end()) {
            it = std::next(it);

            if (it != pdfData.m_cachedPainters.end()) {
                pdfData.m_currentPainterIdx = it.key();
                pdfData.m_layout.setY(pdfData.m_layout.topY());
                pdfData.m_layout.moveXToBegin();
            } else {
                createOrUseExisting();
            }
        } else {
            createOrUseExisting();
        }
    }

    if (pdfData.m_colorsStack.size() > 1) {
        pdfData.repeatColor();
    }
}

void PdfRenderer::drawHorizontalLine(PdfAuxData &pdfData)
{
    pdfData.setColor(m_opts.m_borderColor);
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
            drawParagraph(pdfData, item->text().get(), doc, offset, withNewLine,
                          heightCalcOpt, scale * (1.0 + (7 - item->level()) * 0.25), Qt::black, false, rtl);

        if (heightCalcOpt == CalcHeightOpt::Unknown && !item->label().isEmpty() && !where.first.isEmpty()) {
            auto tmpDest = pdfData.m_doc->CreateDestination();
            tmpDest->SetDestination(pdfData.m_doc->GetPages().GetPageAt(static_cast<unsigned int>(where.first.front().m_pageIdx)),
                                 pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() * offset,
                                 where.first.front().m_y + where.first.front().m_height,
                                 0.0);

            auto dest = std::shared_ptr<Destination>(std::move(tmpDest));

            for (const auto &label : std::as_const(item->labelVariants())) {
                m_dests.insert(label, dest);
            }
        }

        return where;
    } else {
        return {};
    }
}

void PdfRenderer::initSubSupScript(MD::ItemWithOpts<MD::QStringTrait> *item,
                                   PrevBaselineStateStack &state,
                                   double lineHeight,
                                   double descent)
{
    for (const auto &s : item->openStyles()) {
        switch(s.style()) {
        case 8:
            state.m_stack.push_back({state.m_stack.back().m_baselineDelta + state.nextBaselineDelta(true),
                                     state.nextScale(),
                                     state.nextLineHeight(lineHeight),
                                     state.nextDescent(descent)});
            break;

        case 16:
            state.m_stack.push_back({state.m_stack.back().m_baselineDelta - state.nextBaselineDelta(false),
                                     state.nextScale(),
                                     state.nextLineHeight(lineHeight),
                                     state.nextDescent(descent)});
            break;

        case 32:
            ++state.m_mark;
            break;

        default:
            break;
        }
    }
}

void
PdfRenderer::deinitSubSupScript(MD::ItemWithOpts<MD::QStringTrait> *item, PrevBaselineStateStack &state)
{
    for (const auto &s : item->closeStyles()) {
        switch(s.style()) {
        case 8:
        case 16:
            state.m_stack.pop_back();
            break;

        case 32:
            --state.m_mark;
            break;

        default:
            break;
        }
    }
}

QPair<QVector<QPair<QRectF,
                    unsigned int>>,
      PdfRenderer::PrevBaselineStateStack>
PdfRenderer::drawText(PdfAuxData &pdfData,
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
                      const PrevBaselineStateStack &previousBaseline,
                      const QColor &color,
                      RTLFlag *rtl)
{
    auto *spaceFont = createFont(m_opts.m_textFont, false, false,
                                 m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    auto *font = createFont(m_opts.m_textFont,
                            item->opts() & MD::TextOption::BoldText,
                            item->opts() & MD::TextOption::ItalicText,
                            m_opts.m_textFontSize,
                            pdfData.m_doc,
                            scale,
                            pdfData);

    auto current = previousBaseline;
    const auto lineHeight = pdfData.lineSpacing(font, m_opts.m_textFontSize, scale);

    initSubSupScript(static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item), current, lineHeight,
                     -pdfData.fontDescent(font, m_opts.m_textFontSize, scale));

    auto ret = drawString(pdfData,
                      item->text(),
                      spaceFont,
                      m_opts.m_textFontSize,
                      scale,
                      font,
                      m_opts.m_textFontSize,
                      scale,
                      lineHeight,
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
                      current,
                      color,
                      nullptr,
                      0.0,
                      0.0,
                      rtl);

    deinitSubSupScript(static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item), current);

    ret.second = current;

    return ret;
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

bool PdfRenderer::isTextOrOnlineAfter(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                                      MD::Block<MD::QStringTrait>::Items::const_iterator last,
                                      PdfAuxData &pdfData,
                                      double offset,
                                      double lineHeight,
                                      bool scaleImagesToLineHeight)
{
    it = skipRawHtmlAndSpaces(std::next(it), last);

    if (it != last) {
        return isTextOrOnline(it, false, pdfData, offset, lineHeight, scaleImagesToLineHeight);
    }

    return false;
}

bool PdfRenderer::isTextOrOnlineBefore(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                                       MD::Block<MD::QStringTrait>::Items::const_iterator begin,
                                       MD::Block<MD::QStringTrait>::Items::const_iterator last,
                                       PdfAuxData &pdfData,
                                       double offset,
                                       double lineHeight,
                                       bool scaleImagesToLineHeight)
{
    if (it != begin) {
        it = skipRawHtmlAndSpacesBackward(std::prev(it), begin, last);

        if (it != last) {
            return isTextOrOnline(it, true, pdfData, offset, lineHeight, scaleImagesToLineHeight);
        }
    }

    return false;
}

bool PdfRenderer::isNothingAfter(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                                 MD::Block<MD::QStringTrait>::Items::const_iterator last)
{
    if (it != last) {
        it = std::next(it);

        for (; it != last; ++it) {
            if (isNotHtmlNorSpace(it)) {
                return false;
            }
        }
    }

    return true;
}

MD::Item<MD::QStringTrait> *PdfRenderer::getPrevItem(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                                                     MD::Block<MD::QStringTrait>::Items::const_iterator begin,
                                                     MD::Block<MD::QStringTrait>::Items::const_iterator last)
{
    it = skipBackwardWithFunc(it, begin, last,
        &PdfRenderer::isNotHtml<MD::Block<MD::QStringTrait>::Items::const_iterator>);

    if (it != last) {
        return it->get();
    } else {
        return nullptr;
    }
}

MD::Block<MD::QStringTrait>::Items::const_iterator
PdfRenderer::skipRawHtmlAndSpacesBackward(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                                          MD::Block<MD::QStringTrait>::Items::const_iterator begin,
                                          MD::Block<MD::QStringTrait>::Items::const_iterator last)
{
    it = skipBackwardWithFunc(it, begin, last,
        &PdfRenderer::isNotHtmlNorSpace<MD::Block<MD::QStringTrait>::Items::const_iterator>);

    if (it != last) {
        if ((*it)->type() == MD::ItemType::RawHtml) {
            return last;
        } else {
            if (isSpace(it)) {
                return last;
            } else {
                return it;
            }
        }
    }

    return last;
}

QPair<QVector<QPair<QRectF,
                    unsigned int>>,
      PdfRenderer::PrevBaselineStateStack>
PdfRenderer::drawLink(PdfAuxData &pdfData,
                      MD::Link<MD::QStringTrait> *item,
                      std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                      bool &newLine,
                      Font *footnoteFont,
                      double footnoteFontSize,
                      double footnoteFontScale,
                      MD::Item<MD::QStringTrait> *prevItem,
                      MD::Item<MD::QStringTrait> *nextItem,
                      int footnoteNum,
                      double offset,
                      double lineHeight,
                      double spaceWidth,
                      bool firstInParagraph,
                      bool lastInParagraph,
                      bool isPrevText,
                      bool isNextText,
                      CustomWidth &cw,
                      double scale,
                      bool scaleImagesToLineHeight,
                      const PrevBaselineStateStack &previousBaseline,
                      RTLFlag *rtl)
{
    QVector<QPair<QRectF, unsigned int>> rects;
    auto current = previousBaseline;

    QString url = item->url();

    const auto lit = doc->labeledLinks().find(url);

    if (lit != doc->labeledLinks().cend()) {
        url = lit->second->url();
    }

    bool draw = true;

    if (!cw.isDrawing()) {
        draw = false;
    }

    auto *font = createFont(m_opts.m_textFont,
                            item->opts() & MD::TextOption::BoldText,
                            item->opts() & MD::TextOption::ItalicText,
                            m_opts.m_textFontSize,
                            pdfData.m_doc,
                            scale,
                            pdfData);

    if (!item->p()->isEmpty()) {
        for (auto it = item->p()->items().cbegin(), last = item->p()->items().cend(); it != last; ++it) {
            switch ((*it)->type()) {
            case MD::ItemType::Text: {
                auto *text = std::static_pointer_cast<MD::Text<MD::QStringTrait>>(*it).get();

                auto *spaceFont = createFont(m_opts.m_textFont, false, false,
                                             m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

                auto *font = createFont(m_opts.m_textFont,
                                        text->opts() & MD::BoldText || item->opts() & MD::BoldText,
                                        text->opts() & MD::ItalicText || item->opts() & MD::ItalicText,
                                        m_opts.m_textFontSize,
                                        pdfData.m_doc,
                                        scale,
                                        pdfData);

                const AutoSubSupScriptInit subSupInit(this,
                                                      static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(text),
                                                      current,
                                                      pdfData.lineSpacing(font, m_opts.m_textFontSize, scale),
                                                      -pdfData.fontDescent(font, m_opts.m_textFontSize, scale));

                const auto r = drawString(pdfData,
                                          text->text(),
                                          spaceFont,
                                          m_opts.m_textFontSize,
                                          scale,
                                          font,
                                          m_opts.m_textFontSize,
                                          scale,
                                          lineHeight,
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
                                          current,
                                          m_opts.m_linkColor,
                                          nullptr,
                                          0.0,
                                          0.0,
                                          rtl);
                rects.append(r.first);
            } break;

            case MD::ItemType::Code: {
                const auto r = drawInlinedCode(pdfData,
                                               static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                                               doc,
                                               newLine,
                                               offset,
                                               (it == item->p()->items().begin() && firstInParagraph),
                                               cw,
                                               scale,
                                               current,
                                               rtl,
                                               true);
                rects.append(r.first);
                current = r.second;

                setRTLFlagToFalseIfCheck(rtl);
            }
                break;

            case MD::ItemType::Image: {
                auto prev = (it != item->p()->items().begin() ?
                            getPrevItem(std::prev(it), item->p()->items().begin(), last) : nullptr);
                const auto r = drawImage(
                    pdfData,
                    static_cast<MD::Image<MD::QStringTrait> *>(it->get()),
                    doc,
                    newLine,
                    offset,
                    lineHeight,
                    spaceWidth,
                    (it == item->p()->items().begin() && firstInParagraph),
                    isNothingAfter(it, last) && lastInParagraph,
                    isTextOrOnlineBefore(it,
                                         item->p()->items().cbegin(),
                                         last,
                                         pdfData,
                                         offset,
                                         lineHeight,
                                         scaleImagesToLineHeight)
                        || isPrevText,
                    isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight) || isNextText,
                    cw,
                    1.0,
                    current,
                    (prev ? prev : prevItem),
                    (pdfData.m_tableDrawing ? ImageAlignment::Unknown : m_opts.m_imageAlignment));
                rects.append(r.first);
                current = r.second;

                setRTLFlagToFalseIfCheck(rtl);
            } break;

            default:
                break;
            }
        }
    } else if (item->img()->isEmpty()) {
        auto *spaceFont = createFont(m_opts.m_textFont, false, false,
                                     m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

        const AutoSubSupScriptInit subSupInit(this,
                                              static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item),
                                              current,
                                              pdfData.lineSpacing(font, m_opts.m_textFontSize, scale),
                                              -pdfData.fontDescent(font, m_opts.m_textFontSize, scale));

        const auto r = drawString(pdfData,
                                  url,
                                  spaceFont,
                                  m_opts.m_textFontSize,
                                  scale,
                                  font,
                                  m_opts.m_textFontSize,
                                  scale,
                                  lineHeight,
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
                                  current,
                                  m_opts.m_linkColor,
                                  nullptr,
                                  0.0,
                                  0.0,
                                  rtl);
        rects = r.first;
    }
    // Otherwise image link.
    else {
        const auto r = drawImage(pdfData,
                                 item->img().get(),
                                 doc,
                                 newLine,
                                 offset,
                                 lineHeight,
                                 spaceWidth,
                                 firstInParagraph,
                                 true,
                                 isPrevText,
                                 isNextText,
                                 cw,
                                 1.0,
                                 current,
                                 prevItem,
                                 (pdfData.m_tableDrawing ? ImageAlignment::Unknown : m_opts.m_imageAlignment));
        rects.append(r.first);
        current = r.second;

        setRTLFlagToFalseIfCheck(rtl);
    }

    rects = normalizeRects(rects);

    if (draw) {
        // If Web URL.
        if (!pdfData.m_anchors.contains(url) && pdfData.m_md->labeledHeadings().find(url) == pdfData.m_md->labeledHeadings().cend()) {
            for (const auto &r : std::as_const(rects)) {
                auto &annot = static_cast<PoDoFo::PdfAnnotationLink&>(pdfData.m_doc->GetPages()
                                  .GetPageAt(static_cast<unsigned int>(r.second))
                                  .GetAnnotations()
                                  .CreateAnnot<PoDoFo::PdfAnnotationLink>(Rect(r.first.x(), r.first.y(), r.first.width(), r.first.height())));
                annot.SetBorderStyle(0.0, 0.0, 0.0);

                auto actionSmart = pdfData.m_doc->CreateAction(PoDoFo::PdfActionType::URI);
                auto action = static_cast<PoDoFo::PdfActionURI*>(actionSmart.get());
                action->SetURI(PoDoFo::PdfString(url.toLatin1().constData()));

                annot.SetAction(*action);
            }
        }
        // Otherwise internal link.
        else {
            m_unresolvedLinks.insert(url, rects);
        }
    }

    return {rects, current};
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

inline bool isTheSameAlignment(ParagraphAlignment align, bool rightToLeft)
{
    switch (align) {
    case ParagraphAlignment::Left:
        return !rightToLeft;

    case ParagraphAlignment::Right:
        return rightToLeft;

    default:
        return false;
    }
}

} /* namespace anonymous */

void PdfRenderer::alignLine(PdfAuxData &pdfData, const CustomWidth &cw)
{
    if (cw.alignment() != ParagraphAlignment::FillWidth) {
        pdfData.m_layout.moveXToBegin();
        const double delta = (isTheSameAlignment(cw.alignment(), pdfData.m_layout.isRightToLeft()) ? 0.0 :
                            (cw.alignment() == ParagraphAlignment::Center ?
                                 (pdfData.m_layout.availableWidth() - cw.width()) / 2.0 :
                                 (pdfData.m_layout.availableWidth() - cw.width())));
        pdfData.m_layout.addX(delta);
    }
}

QPair<QVector<QPair<QRectF,
                    unsigned int>>,
      PdfRenderer::PrevBaselineStateStack>
PdfRenderer::drawString(PdfAuxData &pdfData,
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
                        const PrevBaselineStateStack &currentBaseline,
                        const QColor &color,
                        Font *regularSpaceFont,
                        double regularSpaceFontSize,
                        double regularSpaceFontScale,
                        RTLFlag *rtl)
{
    spaceFontSize *= currentBaseline.m_stack.back().m_scale;
    fontSize *= currentBaseline.m_stack.back().m_scale;
    footnoteFontSize *= currentBaseline.m_stack.back().m_scale;
    regularSpaceFontSize *= currentBaseline.m_stack.back().m_scale;
    lineHeight = currentBaseline.m_stack.back().m_lineHeight;

    Q_UNUSED(doc)
    Q_UNUSED(m_opts)

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

    if (cw.isDrawing()) {
        h = cw.height();
    }

    auto newLineFn = [&]() {
        newLine = true;

        if (draw) {
            cw.moveToNextLine();

            moveToNewLine(pdfData, offset, cw.height(), 1.0, cw.height());

            alignLine(pdfData, cw);

            h = cw.height();
        } else {
            cw.append({0.0, 0.0, false, true, true});
            pdfData.m_layout.moveXToBegin();
        }
    };

    QVector<QPair<QRectF, unsigned int>> ret;

    {
        QMutexLocker lock(&m_mutex);

        if (m_terminate) {
            return {ret, currentBaseline};
        }
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

        if (draw && cw.alignment() == ParagraphAlignment::FillWidth) {
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
                                     pdfData.m_currentPainterIdx));

                if (background.isValid() &&!useRegularSpace) {
                    pdfData.setColor(background);
                    pdfData.drawRectangle(pdfData.m_layout.startX(width),
                                          pdfData.m_layout.y() + cw.descent() + pdfData.fontDescent(font, fontSize, fontScale) + currentBaseline.m_stack.back().m_baselineDelta,
                                          width,
                                          pdfData.lineSpacing(font, fontSize, fontScale),
                                          PoDoFo::PdfPathDrawMode::Fill);
                    pdfData.restoreColor();
                }

                Font *font = (useRegularSpace && regularSpaceFont ? regularSpaceFont : spaceFont);
                const auto size = (useRegularSpace && regularSpaceFont ? regularSpaceFontSize * regularSpaceFontScale :
                                                                         spaceFontSize * spaceFontScale);
                pdfData.drawText(pdfData.m_layout.startX(width),
                                 pdfData.m_layout.y() + cw.descent() + currentBaseline.m_stack.back().m_baselineDelta,
                                 " ",
                                 font,
                                 size,
                                 scale / 100.0,
                                 strikeout);
            } else {
                const auto lineInfo = currentBaseline.fullLineHeight();

                cw.append({currentSpaceWidth,
                           lineInfo.first,
                           true,
                           false,
                           true,
                           " ",
                           (useRegularSpace && regularSpaceFont
                                ? -pdfData.fontDescent(regularSpaceFont, regularSpaceFontSize, regularSpaceFontScale)
                                    + lineInfo.second
                                : -pdfData.fontDescent(spaceFont, spaceFontSize, spaceFontScale) + lineInfo.second)});
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
                return {ret, currentBaseline};
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
                                              pdfData.m_layout.y() + cw.descent() + pdfData.fontDescent(font, fontSize, fontScale) + currentBaseline.m_stack.back().m_baselineDelta,
                                              length,
                                              pdfData.lineSpacing(font, fontSize, fontScale),
                                              PoDoFo::PdfPathDrawMode::Fill);
                        pdfData.restoreColor();
                    }

                    pdfData.setColor(color);

                    if (it->second) {
                        std::reverse(it->first.begin(), it->first.end());
                    }

                    pdfData.drawText(
                        pdfData.m_layout.startX(length),
                        pdfData.m_layout.y() + cw.descent() + currentBaseline.m_stack.back().m_baselineDelta,
                        createUtf8String(it->first),
                        font,
                        fontSize * fontScale,
                        1.0,
                        strikeout);
                    pdfData.restoreColor();

                    ret.append(qMakePair(pdfData.m_layout.currentRect(length, lineHeight), pdfData.m_currentPainterIdx));
                } else {
                    const auto lineInfo = currentBaseline.fullLineHeight();

                    cw.append({length + (it + 1 == last && footnoteAtEnd ? footnoteWidth : 0.0),
                               lineInfo.first,
                               false,
                               false,
                               true,
                               it->first,
                               -pdfData.fontDescent(font, fontSize, fontScale) + lineInfo.second});
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

                            pdfData.drawText(
                                pdfData.m_layout.startX(w),
                                pdfData.m_layout.y() + cw.descent() + currentBaseline.m_stack.back().m_baselineDelta,
                                createUtf8String(tmp),
                                font,
                                fontSize * fontScale,
                                1.0,
                                strikeout);
                            pdfData.restoreColor();

                            ret.append(qMakePair(pdfData.m_layout.currentRect(w, lineHeight), pdfData.m_currentPainterIdx));
                        } else {
                            const auto lineInfo = currentBaseline.fullLineHeight();

                            cw.append({w,
                                       lineInfo.first,
                                       false,
                                       false,
                                       true,
                                       tmp,
                                       -pdfData.fontDescent(font, fontSize, fontScale) + lineInfo.second});
                        }

                        newLine = false;

                        pdfData.m_layout.addX(w);

                        if (s.length()) {
                            newLineFn();
                        }
                    }

                    if (!draw && it + 1 == last && footnoteAtEnd) {
                        const auto lineInfo = currentBaseline.fullLineHeight();

                        cw.append({footnoteWidth,
                                   lineInfo.first,
                                   false,
                                   false,
                                   true,
                                   QString::number(footnoteNum),
                                   -pdfData.fontDescent(font, fontSize, fontScale) + lineInfo.second});
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

    return {ret, currentBaseline};
}

QPair<QVector<QPair<QRectF,
                    unsigned int>>,
      PdfRenderer::PrevBaselineStateStack>
PdfRenderer::drawInlinedCode(PdfAuxData &pdfData,
                             MD::Code<MD::QStringTrait> *item,
                             std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                             bool &newLine,
                             double offset,
                             bool firstInParagraph,
                             CustomWidth &cw,
                             double scale,
                             const PrevBaselineStateStack &previousBaseline,
                             RTLFlag *rtl,
                             bool inLink)
{
    Q_UNUSED(rtl)

    auto *textFont = createFont(m_opts.m_textFont, false, false,
                                m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    auto *font = createFont(m_opts.m_codeFont,
                            item->opts() & MD::TextOption::BoldText,
                            item->opts() & MD::TextOption::ItalicText,
                            m_opts.m_codeFontSize,
                            pdfData.m_doc,
                            scale,
                            pdfData);

    auto backgroundColor = m_opts.m_syntax->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding);
    auto textColor = m_opts.m_syntax->theme().textColor(KSyntaxHighlighting::Theme::Normal);

    if (inLink) {
        const auto linkColor = m_opts.m_linkColor.rgb();

        backgroundColor = qRgb((qRed(backgroundColor) + qRed(linkColor)) / 2,
                               (qGreen(backgroundColor) + qGreen(linkColor)) / 2,
                               (qBlue(backgroundColor) + qBlue(linkColor)) / 2);

        textColor = qRgb((qRed(textColor) + qRed(linkColor)) / 2,
                         (qGreen(textColor) + qGreen(linkColor)) / 2,
                         (qBlue(textColor) + qBlue(linkColor)) / 2);
    }

    auto current = previousBaseline;
    const auto lineHeight = pdfData.lineSpacing(textFont, m_opts.m_textFontSize, scale);

    initSubSupScript(static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item), current,
                     pdfData.lineSpacing(font, m_opts.m_codeFontSize, scale),
                     -pdfData.fontDescent(font, m_opts.m_codeFontSize, scale));

    auto ret = drawString(pdfData,
                      item->text(),
                      font,
                      m_opts.m_codeFontSize,
                      scale,
                      font,
                      m_opts.m_codeFontSize,
                      scale,
                      lineHeight,
                      doc,
                      newLine,
                      nullptr,
                      0.0,
                      0.0,
                      nullptr,
                      pdfData.m_footnoteNum,
                      offset,
                      firstInParagraph,
                      cw,
                      backgroundColor,
                      item->opts() & MD::TextOption::StrikethroughText,
                      item->startLine(),
                      item->startColumn(),
                      item->endLine(),
                      item->endColumn(),
                      current,
                      textColor,
                      textFont,
                      m_opts.m_textFontSize,
                      scale,
                      rtl);

    deinitSubSupScript(static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item), current);

    ret.second = current;

    return ret;
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

double totalHeight(const QVector<WhereDrawn> &where, bool withExtra = true)
{
    return std::accumulate(where.cbegin(), where.cend(), 0.0, [withExtra](const double &val, const WhereDrawn &cur) -> double {
        return (val + cur.m_height + (withExtra ? cur.m_extraHeight : 0.0));
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

bool isRightToLeft(MD::Block<MD::QStringTrait> *p)
{
    if (!p->isEmpty()) {
        return isRightToLeft(p->items().front().get());
    } else {
        return false;
    }
}

inline bool isLastInParagraph(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                              MD::Block<MD::QStringTrait>::Items::const_iterator last)
{
    it = std::next(it);

    for (; it != last; ++it) {
        switch ((*it)->type()) {
        case MD::ItemType::Text:
        case MD::ItemType::Code:
        case MD::ItemType::Link:
        case MD::ItemType::Image:
        case MD::ItemType::Math:
        case MD::ItemType::FootnoteRef:
            return false;

        default:
            break;
        }
    }

    return true;
}

} /* namespace anonymous */

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawParagraph(PdfAuxData &pdfData,
                                                                  MD::Paragraph<MD::QStringTrait> *item,
                                                                  std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                                  double offset,
                                                                  bool withNewLine,
                                                                  CalcHeightOpt heightCalcOpt,
                                                                  double scale,
                                                                  const QColor &color,
                                                                  bool scaleImagesToLineHeight,
                                                                  RTLFlag *rtl,
                                                                  ParagraphAlignment align)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    if (item->isEmpty()) {
        return {{}, {}};
    }

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

    auto *font = createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    auto *footnoteFont = font;

    const auto lineHeight = pdfData.lineSpacing(font, m_opts.m_textFontSize, scale);
    const auto spaceWidth = pdfData.stringWidth(font, m_opts.m_textFontSize, scale, " ");

    const auto isParagraphRightToLeft = isRightToLeft(item);
    const auto rightToLeft = (rtl && !rtl->isCheck() ? rtl->isRightToLeft() : isParagraphRightToLeft);
    const auto autoOffset = pdfData.m_layout.addOffset(offset, !rightToLeft);

    pdfData.m_layout.setRightToLeft(isParagraphRightToLeft);
    pdfData.m_layout.moveXToBegin();

    bool newLine = false;
    CustomWidth cw;

    bool lineBreak = false;
    bool firstInParagraph = true;

    PrevBaselineStateStack previous(lineHeight, -pdfData.fontDescent(font, m_opts.m_textFontSize, scale));

    // Calculate words/lines/spaces widthes.
    for (auto it = item->items().begin(), last = item->items().end(); it != last; ++it) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        int nextFootnoteNum = pdfData.m_footnoteNum;

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

            previous = drawText(pdfData,
                                text,
                                doc,
                                newLine,
                                footnoteFont,
                                m_opts.m_textFontSize * scale,
                                s_footnoteScale,
                                (it + 1 != last ? (it + 1)->get() : nullptr),
                                nextFootnoteNum,
                                offset,
                                (firstInParagraph || lineBreak),
                                cw,
                                scale,
                                previous,
                                color,
                                rtl)
                           .second;
            lineBreak = false;
            firstInParagraph = false;
        } break;

        case MD::ItemType::Code:
            previous = drawInlinedCode(pdfData,
                                       static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                                       doc,
                                       newLine,
                                       offset,
                                       (firstInParagraph || lineBreak),
                                       cw,
                                       scale,
                                       previous,
                                       rtl)
                           .second;
            lineBreak = false;
            firstInParagraph = false;
            break;

        case MD::ItemType::Link:
            previous = drawLink(pdfData,
                                static_cast<MD::Link<MD::QStringTrait> *>(it->get()),
                                doc,
                                newLine,
                                footnoteFont,
                                m_opts.m_textFontSize * scale,
                                s_footnoteScale,
                                (it != item->items().begin() ? std::prev(it)->get() : nullptr),
                                (std::next(it) != last ? std::next(it)->get() : nullptr),
                                nextFootnoteNum,
                                offset,
                                lineHeight,
                                spaceWidth,
                                (firstInParagraph || lineBreak),
                                isLastInParagraph(it, last),
                                isTextOrOnlineBefore(it,
                                                     item->items().begin(),
                                                     last,
                                                     pdfData,
                                                     offset,
                                                     lineHeight,
                                                     scaleImagesToLineHeight),
                                isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight),
                                cw,
                                scale,
                                scaleImagesToLineHeight,
                                previous,
                                rtl)
                           .second;
            lineBreak = false;
            firstInParagraph = false;
            break;

        case MD::ItemType::Image:
            previous = drawImage(pdfData,
                                 static_cast<MD::Image<MD::QStringTrait> *>(it->get()),
                                 doc,
                                 newLine,
                                 offset,
                                 lineHeight,
                                 spaceWidth,
                                 (firstInParagraph || lineBreak),
                                 isLastInParagraph(it, last),
                                 isTextOrOnlineBefore(it,
                                                      item->items().begin(),
                                                      last,
                                                      pdfData,
                                                      offset,
                                                      lineHeight,
                                                      scaleImagesToLineHeight),
                                 isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight),
                                 cw,
                                 1.0,
                                 previous,
                                 (it != item->items().begin() ? getPrevItem(std::prev(it), item->items().begin(), last)
                                                              : nullptr),
                                 (pdfData.m_tableDrawing ? ImageAlignment::Unknown : m_opts.m_imageAlignment),
                                 scaleImagesToLineHeight)
                           .second;
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
            break;

        case MD::ItemType::Math:
            previous = drawMathExpr(pdfData,
                                    static_cast<MD::Math<MD::QStringTrait> *>(it->get()),
                                    doc,
                                    (it != item->items().begin() ? std::prev(it)->get() : nullptr),
                                    newLine,
                                    offset,
                                    isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight),
                                    (firstInParagraph || lineBreak),
                                    cw,
                                    scale,
                                    previous)
                           .second;
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
            break;

        case MD::ItemType::LineBreak: {
            lineBreak = true;
            cw.append({0.0, 0.0, false, true, false, ""});
            pdfData.m_layout.moveXToBegin();
        } break;

        case MD::ItemType::FootnoteRef: {
            auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>(it->get());

            const auto fit = doc->footnotesMap().find(ref->id());

            if (fit != doc->footnotesMap().cend()) {
                auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());

                if (anchorIt == pdfData.m_footnotesAnchorsMap.cend()) {
                    pdfData.m_footnotesAnchorsMap.insert(fit->second.get(),
                                                         {pdfData.m_currentFile, pdfData.m_footnoteNum++});
                }
            } else {
                auto text = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

                previous = drawText(pdfData,
                                    text,
                                    doc,
                                    newLine,
                                    footnoteFont,
                                    m_opts.m_textFontSize * scale,
                                    s_footnoteScale,
                                    (it + 1 != last ? (it + 1)->get() : nullptr),
                                    nextFootnoteNum,
                                    offset,
                                    (firstInParagraph || lineBreak),
                                    cw,
                                    scale,
                                    previous,
                                    color)
                               .second;
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
        cw.append({0.0, 0.0, false, true, false});
    }

    cw.calcScale(pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
                 pdfData.m_layout.margins().m_right - offset);

    cw.setDrawing();
    cw.setAlignment(align);

    switch (heightCalcOpt) {
    case CalcHeightOpt::Minimum: {
        QVector<WhereDrawn> r;
        r.append(
            {-1,
             0.0,
             cw.firstLineHeight(),
             ((withNewLine && !pdfData.m_firstOnPage) || (withNewLine && pdfData.m_drawFootnotes) ?
                             lineHeight : 0.0)});

        return {r, {}};
    }

    case CalcHeightOpt::Full: {
        QVector<WhereDrawn> r;

        const double extra = (((withNewLine && !pdfData.m_firstOnPage) || (withNewLine && pdfData.m_drawFootnotes)) ?
                            lineHeight : 0.0);

        for (auto it = cw.cbegin(), last = cw.cend(); it != last; ++it) {
            r.append({-1, 0.0, *it});
        }

        if (!r.isEmpty()) {
            r.front().m_extraHeight = extra;
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

    alignLine(pdfData, cw);

    bool extraOnFirstLine = true;
    newLine = false;

    const auto firstLineY = pdfData.m_layout.y();
    const auto firstLinePageIdx = pdfData.m_currentPainterIdx;
    const auto firstLineHeight = cw.height();

    QScopedValueRollback continueParagraph(pdfData.m_continueParagraph, true);

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

        int nextFootnoteNum = pdfData.m_footnoteNum;

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

            const auto r = drawText(pdfData,
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
                                    previous,
                                    color,
                                    rtl);
            rects.append(r.first);
            previous = r.second;
            lineBreak = false;
            firstInParagraph = false;
        } break;

        case MD::ItemType::Code: {
            const auto r = drawInlinedCode(pdfData,
                                           static_cast<MD::Code<MD::QStringTrait> *>(it->get()),
                                           doc,
                                           newLine,
                                           offset,
                                           (firstInParagraph || lineBreak),
                                           cw,
                                           scale,
                                           previous);
            rects.append(r.first);
            previous = r.second;
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

            const auto r = drawLink(pdfData,
                                    static_cast<MD::Link<MD::QStringTrait> *>(it->get()),
                                    doc,
                                    newLine,
                                    footnoteFont,
                                    m_opts.m_textFontSize * scale,
                                    s_footnoteScale,
                                    (it != item->items().begin() ? std::prev(it)->get() : nullptr),
                                    (std::next(it) != last ? std::next(it)->get() : nullptr),
                                    nextFootnoteNum,
                                    offset,
                                    lineHeight,
                                    spaceWidth,
                                    (firstInParagraph || lineBreak),
                                    isLastInParagraph(it, last),
                                    isTextOrOnlineBefore(it,
                                                         item->items().begin(),
                                                         last,
                                                         pdfData,
                                                         offset,
                                                         lineHeight,
                                                         scaleImagesToLineHeight),
                                    isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight),
                                    cw,
                                    scale,
                                    scaleImagesToLineHeight,
                                    previous,
                                    rtl);
            rects.append(r.first);
            previous = r.second;
            lineBreak = false;
            firstInParagraph = false;
        } break;

        case MD::ItemType::Image: {
            if (extraOnFirstLine) {
                pdfData.m_layout.addY(cw.height(), -1.0);
            }

            const auto r = drawImage(
                pdfData,
                static_cast<MD::Image<MD::QStringTrait> *>(it->get()),
                doc,
                newLine,
                offset,
                lineHeight,
                spaceWidth,
                (firstInParagraph || lineBreak),
                isLastInParagraph(it, last),
                isTextOrOnlineBefore(it,
                                     item->items().begin(),
                                     last,
                                     pdfData,
                                     offset,
                                     lineHeight,
                                     scaleImagesToLineHeight),
                isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight),
                cw,
                1.0,
                previous,
                (it != item->items().begin() ? getPrevItem(std::prev(it), item->items().begin(), last) : nullptr),
                (pdfData.m_tableDrawing ? ImageAlignment::Unknown : m_opts.m_imageAlignment),
                scaleImagesToLineHeight);
            rects.append(r.first);
            previous = r.second;
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        case MD::ItemType::Math: {
            pdfData.setColor(color);
            const auto r =
                drawMathExpr(pdfData,
                             static_cast<MD::Math<MD::QStringTrait> *>(it->get()),
                             doc,
                             (it != item->items().begin() ? std::prev(it)->get() : nullptr),
                             newLine,
                             offset,
                             isTextOrOnlineAfter(it, last, pdfData, offset, lineHeight, scaleImagesToLineHeight),
                             (firstInParagraph || lineBreak),
                             cw,
                             scale,
                             previous);
            rects.append(r.first);
            previous = r.second;
            pdfData.restoreColor();
            lineBreak = false;
            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        case MD::ItemType::LineBreak: {
            lineBreak = true;
            moveToNewLine(pdfData, offset, lineHeight, 1.0, lineHeight);
            cw.moveToNextLine();
        } break;

        case MD::ItemType::FootnoteRef: {
            lineBreak = false;

            auto *ref = static_cast<MD::FootnoteRef<MD::QStringTrait> *>(it->get());

            const auto fit = doc->footnotesMap().find(ref->id());

            if (fit != doc->footnotesMap().cend()) {
                auto anchorIt = pdfData.m_footnotesAnchorsMap.constFind(fit->second.get());
                int num = pdfData.m_footnoteNum;

                if (anchorIt != pdfData.m_footnotesAnchorsMap.cend()) {
                    num = anchorIt->second;
                }

                const auto str = createUtf8String(QString::number(num));

                const auto w = pdfData.stringWidth(footnoteFont, m_opts.m_textFontSize * s_footnoteScale, scale, str);

                rects.append(qMakePair(pdfData.m_layout.currentRect(w, lineHeight), pdfData.m_currentPainterIdx));

                m_unresolvedFootnotesLinks.insert(ref->id(), qMakePair(pdfData.m_layout.currentRect(w, lineHeight),
                                                                       pdfData.m_currentPainterIdx));

                pdfData.setColor(m_opts.m_linkColor);

                pdfData.drawText(pdfData.m_layout.startX(w),
                                 pdfData.m_layout.y() + lineHeight - pdfData.lineSpacing(footnoteFont,
                                                                                         m_opts.m_textFontSize * s_footnoteScale, scale)
                                     - pdfData.fontDescent(footnoteFont, m_opts.m_textFontSize * s_footnoteScale, scale),
                                 str,
                                 footnoteFont,
                                 m_opts.m_textFontSize * s_footnoteScale * scale,
                                 1.0,
                                 false);

                pdfData.restoreColor();

                pdfData.m_layout.addX(w);

                addFootnote(ref->id(), fit->second, pdfData, doc);
            } else {
                auto text = static_cast<MD::Text<MD::QStringTrait> *>(it->get());

                const auto r = drawText(pdfData,
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
                                        previous,
                                        color);
                rects.append(r.first);
                previous = r.second;
            }

            firstInParagraph = false;
            setRTLFlagToFalseIfCheck(rtl);
        } break;

        default:
            break;
        }

        extraOnFirstLine = firstInParagraph;
    }

    pdfData.m_layout.setRightToLeft(wasRightToLeft);

    const auto where = toWhereDrawn(normalizeRects(rects), pdfData.m_layout.pageHeight());

    return {where, {firstLinePageIdx, firstLineY, firstLineHeight}};
}

QPair<QPair<QRectF,
            unsigned int>,
      PdfRenderer::PrevBaselineStateStack>
PdfRenderer::drawMathExpr(PdfAuxData &pdfData,
                          MD::Math<MD::QStringTrait> *item,
                          std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                          MD::Item<MD::QStringTrait> *prevItem,
                          bool &newLine,
                          double offset,
                          bool isNextText,
                          bool firstInParagraph,
                          CustomWidth &cw,
                          double scale,
                          const PrevBaselineStateStack &previousBaseline)
{
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    PrevBaselineStateStack current = previousBaseline;

    float fontSize = (float) m_opts.m_textFontSize;

    {
        PoDoFoPaintDevice pd(pdfData);
        fontSize = fontSize / 72.f * (float) pd.physicalDpiY();
    }

    auto *font = createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);
    const auto lineHeight = pdfData.lineSpacing(font, m_opts.m_textFontSize, scale);

    auto latexRender = std::unique_ptr<tex::TeXRender>(
        tex::LaTeX::parse(item->expr().toStdWString(),
                          0,
                          fontSize * (item->isInline() ? current.m_stack.back().m_scale : 1.0),
                          fontSize / 3.f * (item->isInline() ? current.m_stack.back().m_scale : 1.0),
                          tex::black));

    QSizeF pxSize = {}, size = {};
    double descent = 0.0;

    {
        PoDoFoPaintDevice pd(pdfData);
        QPainter p(&pd);
        tex::Graphics2D_qt g2(&p);
        latexRender->draw(g2, 0, 0);
        pxSize = {(qreal)latexRender->getWidth(), (qreal)latexRender->getHeight()};
        size = {pxSize.width() / (qreal) pd.physicalDpiX() * 72.0, pxSize.height() / (qreal) pd.physicalDpiY() * 72.0};
        descent = (item->isInline() ? ((1.0 - latexRender->getBaseline()) * size.height()) :
                (-pdfData.fontDescent(font, m_opts.m_textFontSize, scale)));
    }

    const AutoSubSupScriptInit subSupInit(this,
                                          static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item),
                                          current,
                                          size.height(),
                                          descent);

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

        if (!firstInParagraph) {
            moveToNewLine(pdfData, 0.0, lineHeight, 1.0, 0.0);
        }

        pdfData.m_layout.moveXToBegin();
    }

    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

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

        const double pageHeight = pdfData.topY(pdfData.m_currentPainterIdx) - pdfData.m_layout.margins().m_bottom;

        if (size.height() * imgScale - pageHeight > 0.01) {
            imgScale = (pageHeight / (size.height() * imgScale)) * scale;

            if (!pdfData.m_firstOnPage && draw) {
                createPage(pdfData);

                pdfData.m_layout.addX(offset);
            }

            if (draw) {
                pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);
            }
        } else if (size.height() * imgScale - availableHeight > 0.01) {
            if (draw) {
                createPage(pdfData);

                pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);

                pdfData.m_layout.addX(offset);
            }
        }

        if (draw) {
            pdfData.m_firstOnPage = false;

            if (availableWidth - size.width() * imgScale > 0.01) {
                x = (availableWidth - size.width() * imgScale) / 2.0;
            }

            PoDoFoPaintDevice pd(pdfData);
            pd.enableDrawing();
            QPainter p(&pd);

            pdfData.m_layout.addX(x);

            // y - is a top of a line.
            tex::Graphics2D_qt g2(&p);
            latexRender->draw(g2, pdfData.m_layout.startX(size.width() * imgScale) / 72.0 * pd.physicalDpiX(),
                    (pdfData.m_layout.pageHeight() - pdfData.m_layout.y() + descent * imgScale) / 72.0
                              * pd.physicalDpiY());

            const QRectF r = {pdfData.m_layout.startX(size.width() * imgScale),
                              pdfData.m_layout.y() - descent * imgScale,
                              size.width() * imgScale,
                              size.height() * imgScale};
            const auto idx = pdfData.m_currentPainterIdx;

            pdfData.m_layout.addY(cw.isDrawing() ? cw.height() : 0.0);

            cw.moveToNextLine();

            if (isNextText)
                moveToNewLine(pdfData, offset, lineHeight + cw.height(), 1.0, cw.height());

            return {{r, idx}, current};
        } else {
            calculatedHeight += size.height() * imgScale + descent * imgScale;

            pdfData.m_layout.moveXToBegin();

            if (!firstInParagraph) {
                cw.append({0.0, 0.0, false, true, false});
            }

            cw.append({0.0, calculatedHeight, false, true, false});
        }
    } else {
        // y - is bottom of line.
        const double availableTotalWidth = pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
                pdfData.m_layout.margins().m_right - offset;

        if (size.width() * current.currentScale() - pdfData.m_layout.availableWidth() > 0.01) {
            if (draw) {
                cw.moveToNextLine();

                moveToNewLine(pdfData, offset, cw.height(), 1.0, cw.height());
            } else {
                cw.append({0.0, 0.0, false, true, true});
                pdfData.m_layout.moveXToBegin();
            }
        } else {
            auto addSpace = [&]()
            {
                const auto spaceScale = draw ? (cw.scale() / 100.0) : 1.0;
                const auto spaceWidth = pdfData.stringWidth(font, m_opts.m_textFontSize * current.currentScale(), scale, " ") * spaceScale;

                if (pdfData.m_layout.isFit(spaceWidth)) {
                    if (draw) {
                        pdfData.drawText(pdfData.m_layout.startX(spaceWidth), pdfData.m_layout.y() + cw.descent() + current.m_stack.back().m_baselineDelta, " ",
                                         font, m_opts.m_textFontSize, spaceScale, false);
                    } else {
                        const auto lineInfo = current.fullLineHeight();

                        cw.append({spaceWidth, lineHeight, true, false, true, " ",
                                   -pdfData.fontDescent(font, m_opts.m_textFontSize, spaceScale) + lineInfo.second});
                    }

                    pdfData.m_layout.addX(spaceWidth);
                }
            };

            if (prevItem) {
                switch (prevItem->type()) {
                case MD::ItemType::Code:
                case MD::ItemType::Math:
                    addSpace();
                    break;

                case MD::ItemType::Text:
                {
                    auto t = static_cast<MD::Text<MD::QStringTrait>*>(prevItem);

                    if (!t->text().isEmpty() && !t->text().back().isSpace()) {
                        addSpace();
                    }
                }
                    break;

                default:
                    break;
                }
            }
        }

        double imgScale = 1.0;

        if (size.width() - availableTotalWidth > 0.01) {
            imgScale = (pdfData.m_layout.availableWidth() / size.width()) * scale;
        }

        double availableHeight = pdfData.m_layout.y() - pdfData.currentPageAllowedY();

        const double pageHeight = pdfData.topY(pdfData.m_currentPainterIdx) - pdfData.m_layout.margins().m_bottom;

        if (size.height() * imgScale - pageHeight > 0.01) {
            imgScale = (pageHeight / (size.height() * imgScale)) * scale;

            if (draw) {
                createPage(pdfData);

                pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);

                pdfData.m_layout.addX(offset);
            }
        } else if (size.height() * imgScale - availableHeight > 0.01) {
            if (draw) {
                createPage(pdfData);

                pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);

                pdfData.m_layout.addX(offset);
            }
        }

        if (draw) {
            pdfData.m_firstOnPage = false;

            PoDoFoPaintDevice pd(pdfData);
            pd.enableDrawing();
            QPainter p(&pd);

            const auto height = size.height() * imgScale;

            tex::Graphics2D_qt g2(&p);
            latexRender->draw(g2,
                              pdfData.m_layout.startX(size.width() * imgScale) / 72.0 * pd.physicalDpiX(),
                              (pdfData.m_layout.pageHeight()
                               - pdfData.m_layout.y()
                               - cw.descent()
                               - (height - descent)
                               + current.m_stack.back().m_baselineDelta)
                                  / 72.0
                                  * pd.physicalDpiY());

            const QRectF r = {
                pdfData.m_layout.startX(size.width() * imgScale),
                pdfData.m_layout.y() + cw.descent() + (height - descent) - current.m_stack.back().m_baselineDelta,
                size.width() * imgScale,
                height};

            pdfData.m_layout.addX(size.width() * imgScale);
            const auto idx = pdfData.m_currentPainterIdx;

            return {{r, idx}, current};
        } else {
            pdfData.m_layout.addX(size.width() * imgScale);

            const auto lineInfo = current.fullLineHeight();

            cw.append({size.width() * imgScale,
                       size.height() * imgScale + current.m_stack.back().m_baselineDelta,
                       false,
                       false,
                       isNextText,
                       "",
                       descent * imgScale + lineInfo.second});
        }
    }

    return {{}, current};
}

void PdfRenderer::reserveSpaceForFootnote(PdfAuxData &pdfData,
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
            const auto tmp = h[i].m_height + h[i].m_extraHeight;

            if (height + tmp < available) {
                height += tmp;
            } else {
                if (qAbs(height - extra) > 0.01) {
                    add(height, currentPage);
                }

                reserveSpaceForFootnote(pdfData,
                                        h.mid(i),
                                        pdfData.m_layout.topY(),
                                        currentPage + 1,
                                        lineHeight,
                                        (i > 0 && qAbs(h[i].m_extraHeight) < 0.01 ? true : false));

                break;
            }
        }
    }
}

QVector<WhereDrawn> PdfRenderer::drawFootnote(PdfAuxData &pdfData,
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

    auto *font = createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize,
                            pdfData.m_doc, s_footnoteScale, pdfData);

    auto footnoteOffset = c_offset * 2.0 / s_mmInPt
        + pdfData.stringWidth(font, m_opts.m_textFontSize, s_footnoteScale,
                              createUtf8String(QString::number(doc->footnotesMap().size())));

    if (lineHeight) {
        *lineHeight = pdfData.lineSpacing(font, m_opts.m_textFontSize, s_footnoteScale);
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
                                   static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                                   doc,
                                   footnoteOffset,
                                   // If there is another item after heading we need to know its min
                                   // height to glue heading with it.
                                   (it + 1 != last ? minNecessaryHeight(pdfData, *(it + 1), doc, 0.0, s_footnoteScale) : 0.0),
                                   heightCalcOpt,
                                   s_footnoteScale,
                                   true,
                                   rtl)
                           .first);

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::Paragraph:
            ret.append(drawParagraph(pdfData,
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

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::Code:
            ret.append(
                drawCode(pdfData, static_cast<MD::Code<MD::QStringTrait> *>(
                             it->get()), doc, footnoteOffset, heightCalcOpt, s_footnoteScale).first);

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::Blockquote:
            ret.append(drawBlockquote(pdfData,
                                      static_cast<MD::Blockquote<MD::QStringTrait> *>(it->get()),
                                      doc,
                                      footnoteOffset,
                                      heightCalcOpt,
                                      s_footnoteScale,
                                      rtl)
                           .first);

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);

            break;

        case MD::ItemType::List: {
            auto *list = static_cast<MD::List<MD::QStringTrait> *>(it->get());
            const auto bulletWidth = maxListNumberWidth(list);

            ret.append(drawList(pdfData, list, doc, bulletWidth, footnoteOffset,
                                heightCalcOpt, s_footnoteScale, false, rtl).first);

            if (first) {
                firstItemIsRightToLeft = (rtl ? rtl->isRightToLeft() : false);
                first = false;
            }

            resetRTLFlagToDefaults(rtl);
        } break;

        case MD::ItemType::Table:
            ret.append(
                drawTable(pdfData, static_cast<MD::Table<MD::QStringTrait> *>(
                              it->get()), doc, footnoteOffset, heightCalcOpt, s_footnoteScale)
                    .first);

            if (first) {
                firstItemIsRightToLeft = false;
                first = false;
            }

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
            const auto w = pdfData.stringWidth(font, m_opts.m_textFontSize, s_footnoteScale, str);
            const auto y = ret.constFirst().m_y + ret.constFirst().m_height
                - pdfData.lineSpacing(font, m_opts.m_textFontSize, s_footnoteScale)
                - pdfData.fontDescent(font, m_opts.m_textFontSize, s_footnoteScale);
            const auto x = pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                    (footnoteOffset - c_offset - (pdfData.m_layout.isRightToLeft() ? 0.0 : w));
            const auto p = ret.constFirst().m_pageIdx;

            auto dest = pdfData.m_doc->CreateDestination();
            dest->SetDestination(pdfData.m_doc->GetPages().GetPageAt(p),
                                 x,
                                 y + pdfData.lineSpacing(font, m_opts.m_textFontSize, s_footnoteScale)
                                     + pdfData.fontDescent(font, m_opts.m_textFontSize, s_footnoteScale),
                                 0.0);
            m_dests.insert(footnoteRefId, std::move(dest));

            pdfData.m_currentPainterIdx = p;

            pdfData.drawText(x, y, str, font, m_opts.m_textFontSize * s_footnoteScale, 1.0, false);

            pdfData.m_currentPainterIdx = pdfData.m_footnotePageIdx;

            ++pdfData.m_currentFootnote;
        }
    }

    pdfData.m_layout.setRightToLeft(false);

    return ret;
}

QVector<WhereDrawn> PdfRenderer::footnoteHeight(PdfAuxData &pdfData,
                                                std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                MD::Footnote<MD::QStringTrait> *note,
                                                double *lineHeight)
{
    return drawFootnote(pdfData, doc, "", note, CalcHeightOpt::Full, lineHeight);
}

bool PdfRenderer::isOnlineImage(double totalAvailableWidth,
                                double iWidth,
                                double iHeight,
                                double lineHeight)
{
    return (totalAvailableWidth / 5.0 > iWidth && iHeight < lineHeight * 2.0);
}

bool PdfRenderer::isOnlineImage(PdfAuxData &pdfData,
                                MD::Image<MD::QStringTrait> *item,
                                double offset,
                                double lineHeight,
                                bool scaleImagesToLineHeight)
{
    const auto img = loadImage(item, *pdfData.m_resvgOpts.get(),
                               lineHeight / 72.0 * m_opts.m_dpi, scaleImagesToLineHeight, !scaleImagesToLineHeight);

    if (!img.isNull()) {
        auto pdfImg = pdfData.m_doc->CreateImage();
        pdfImg->LoadFromBuffer({img.data(), static_cast<size_t>(img.size())});

        const double iWidth = std::round((double)pdfImg->GetWidth() / (double)m_opts.m_dpi * 72.0);
        const double iHeight = std::round((double)pdfImg->GetHeight() / (double)m_opts.m_dpi * 72.0);

        const double totalAvailableWidth = pdfData.m_layout.pageWidth()
                - pdfData.m_layout.margins().m_left - pdfData.m_layout.margins().m_right - offset;
        return isOnlineImage(totalAvailableWidth, iWidth, iHeight, lineHeight);
    } else {
        return true;
    }
}

bool PdfRenderer::isOnlineImageOrOnlineImageInLink(PdfAuxData &pdfData,
                                                   MD::Item<MD::QStringTrait> *item,
                                                   double offset,
                                                   double lineHeight,
                                                   bool scaleImagesToLineHeight)
{
    if (item) {
        if (item->type() == MD::ItemType::Image) {
            auto i = static_cast<MD::Image<MD::QStringTrait>*>(item);
            return isOnlineImage(pdfData, i, offset, lineHeight, scaleImagesToLineHeight);
        } else if (item->type() == MD::ItemType::Link) {
            auto l = static_cast<MD::Link<MD::QStringTrait>*>(item);

            if (!l->p()->isEmpty()) {
                for (auto it = l->p()->items().crbegin(), last = l->p()->items().crend(); it != last; ++it) {
                    if ((*it)->type() == MD::ItemType::RawHtml) {
                        continue;
                    } else if ((*it)->type() == MD::ItemType::Image) {
                        return isOnlineImage(pdfData, static_cast<MD::Image<MD::QStringTrait>*>(it->get()),
                                             offset, lineHeight, scaleImagesToLineHeight);
                    } else {
                        return false;
                    }
                }

                return false;
            } else if (l->img()->isEmpty()) {
                return false;
            } else {
                return isOnlineImage(pdfData, l->img().get(), offset, lineHeight, scaleImagesToLineHeight);
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
}

ParagraphAlignment imageToParagraphAlignment(ImageAlignment alignment)
{
    switch (alignment) {
    case ImageAlignment::Unknown:
        return ParagraphAlignment::Unknown;

    case ImageAlignment::Left:
        return ParagraphAlignment::Left;

    case ImageAlignment::Center:
        return ParagraphAlignment::Center;

    case ImageAlignment::Right:
        return ParagraphAlignment::Right;

    default:
        return ParagraphAlignment::Unknown;
    }
}

QPair<QPair<QRectF,
            unsigned int>,
      PdfRenderer::PrevBaselineStateStack>
PdfRenderer::drawImage(PdfAuxData &pdfData,
                       MD::Image<MD::QStringTrait> *item,
                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                       bool &newLine,
                       double offset,
                       double lineHeight,
                       double spaceWidth,
                       bool firstInParagraph,
                       bool lastInParagraph,
                       bool isPrevText,
                       bool isNextText,
                       CustomWidth &cw,
                       double scale,
                       const PrevBaselineStateStack &previousBaseline,
                       MD::Item<MD::QStringTrait> *prevItem,
                       ImageAlignment alignment,
                       bool scaleImagesToLineHeight)
{
    Q_UNUSED(doc)

    bool draw = true;
    pdfData.m_startLine = item->startLine();
    pdfData.m_startPos = item->startColumn();
    pdfData.m_endLine = item->endLine();
    pdfData.m_endPos = item->endColumn();

    PrevBaselineStateStack current = previousBaseline;

    if (!cw.isDrawing())
        draw = false;

    emit status(tr("Loading image."));

    const auto img = loadImage(item, *pdfData.m_resvgOpts.get(),
                               lineHeight / 72.0 * m_opts.m_dpi, scaleImagesToLineHeight, !scaleImagesToLineHeight);

    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

    if (!img.isNull()) {
        auto pdfImg = pdfData.m_doc->CreateImage();
        pdfImg->LoadFromBuffer({img.data(), static_cast<size_t>(img.size())});

        const double iWidth = std::round((double)pdfImg->GetWidth() / (double)m_opts.m_dpi * 72.0);
        const double iHeight = std::round((double)pdfImg->GetHeight() / (double)m_opts.m_dpi * 72.0);

        double imgScale = (scaleImagesToLineHeight ? lineHeight / iHeight : 1.0);
        const double totalAvailableWidth = pdfData.m_layout.pageWidth()
                - pdfData.m_layout.margins().m_left - pdfData.m_layout.margins().m_right - offset;
        const bool onLine = isOnlineImage(totalAvailableWidth, iWidth, iHeight, lineHeight);
        bool addSpace = onLine && !firstInParagraph && !newLine &&
                (isOnlineImageOrOnlineImageInLink(pdfData, prevItem, offset, lineHeight, scaleImagesToLineHeight) ||
                 (prevItem ? prevItem->endLine() != item->startLine() : false));
        double height = (!onLine ? lineHeight : 0.0);
        const auto availableAfter = pdfData.m_layout.availableWidth() - (iWidth * imgScale +
                (addSpace ? spaceWidth * (draw ? cw.scale() / 100.0 : 1.0) : 0.0));

        if ((!onLine && !firstInParagraph) || (onLine && (availableAfter < 0) && (qAbs(availableAfter) > 0.1)) ||
            (isPrevText && !onLine)) {

            if (draw) {
                cw.moveToNextLine();
            } else {
                if (!onLine) {
                    height += lineHeight;
                }

                cw.append({0.0, 0.0, false, true, onLine});
            }

            if (draw) {
                moveToNewLine(pdfData, offset, (onLine ? cw.height() : lineHeight), 1.0, lineHeight);
            } else {
                pdfData.m_layout.moveXToBegin();
            }

            addSpace = false;
        }

        double availableHeight = pdfData.m_layout.y() - pdfData.currentPageAllowedY();
        const double pageHeight = pdfData.topY(pdfData.m_currentPainterIdx) - pdfData.m_layout.margins().m_bottom;

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

                pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);
            } else if (iHeight * imgScale > availableHeight) {
                if (draw) {
                    createPage(pdfData);

                    pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);

                    pdfData.m_layout.addX(offset);
                }
            }
        }

        const double dpiScale = (double)pdfImg->GetWidth() / iWidth;
        double dy = 0.0;
        imgScale *= scale;

        const AutoSubSupScriptInit subSupInit(this, static_cast<MD::ItemWithOpts<MD::QStringTrait> *>(item), current,
                                              iHeight * imgScale, 0.0);

        if (draw) {
            if (!onLine) {
                newLine = true;
                pdfData.m_layout.addY(iHeight * imgScale);
            } else if (firstInParagraph) {
                newLine = false;
                pdfData.m_layout.addY(cw.height());
            }

            if (onLine && addSpace) {
                pdfData.m_layout.addX(spaceWidth * cw.scale() / 100.0);
            }

            dy = (onLine
                      ? (cw.height() - cw.descent() - iHeight * imgScale) / 2.0 + current.m_stack.back().m_baselineDelta
                      : 0.0);

            alignLine(pdfData, cw);

            // y - is bottom.
            pdfData.drawImage(pdfData.m_layout.startX(iWidth * imgScale), pdfData.m_layout.y() + dy,
                              pdfImg.get(), imgScale / dpiScale, imgScale / dpiScale);
        } else {
            if (onLine && addSpace) {
                cw.append({spaceWidth, 0.0, true, false, true, " "});
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

        if (!onLine) {
            newLine = true;

            if (draw) {
                cw.moveToNextLine();
            }
        }

        if (draw && !onLine && !lastInParagraph && isNextText) {
            moveToNewLine(pdfData, offset, lineHeight + cw.height(), 1.0, cw.height());
        }

        if (!draw) {
            cw.append({iWidth * imgScale,
                       std::max(height, lineHeight) + current.m_stack.back().m_baselineDelta,
                       false,
                       !onLine,
                       false,
                       "",
                       0.0,
                       (!onLine ? imageToParagraphAlignment(alignment) : ParagraphAlignment::Unknown)});
        }

        return {{r, pdfData.m_currentPainterIdx}, current};
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

    auto *textFont = createFont(m_opts.m_textFont, false, false,
                                m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    const auto textLHeight = pdfData.lineSpacing(textFont, m_opts.m_textFontSize, scale);

    auto *font = createFont(m_opts.m_codeFont, false, false, m_opts.m_codeFontSize, pdfData.m_doc, scale, pdfData);

    const auto lineHeight = pdfData.lineSpacing(font, m_opts.m_codeFontSize, scale);

    QStringList lines;

    QScopedValueRollback continueParagraph(pdfData.m_continueParagraph, true);

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        if ((pdfData.m_layout.y() - (pdfData.m_firstOnPage ? 0.0 : textLHeight) - lineHeight) < pdfData.currentPageAllowedY()
            && qAbs(pdfData.m_layout.y() - (textLHeight * 2.0) - pdfData.currentPageAllowedY()) > 0.1) {
            createPage(pdfData);
        } else if (!pdfData.m_firstOnPage) {
            pdfData.m_layout.addY(textLHeight);
        }

        pdfData.m_layout.moveXToBegin();
    }

    lines = item->text().split(QLatin1Char('\n'), Qt::KeepEmptyParts);

    for (auto it = lines.begin(), last = lines.end(); it != last; ++it) {
        it->replace(QStringLiteral("\t"), QStringLiteral("    "));
    }

    switch (heightCalcOpt) {
    case CalcHeightOpt::Minimum: {
        QVector<WhereDrawn> r;
        r.append({-1, 0.0, lineHeight, (pdfData.m_firstOnPage ? 0.0 : textLHeight)});

        return {r, {}};
    }

    case CalcHeightOpt::Full: {
        QVector<WhereDrawn> r;
        r.append({-1, 0.0, lineHeight, (pdfData.m_firstOnPage ? 0.0 : textLHeight)});

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

        if (m_terminate) {
            return {};
        }
    }

    m_opts.m_syntax->setDefinition(m_opts.m_syntax->definitionForName(item->syntax().toLower()));
    const auto colored = m_opts.m_syntax->prepare(lines);
    int currentWord = 0;
    const auto spaceWidth = pdfData.stringWidth(font, m_opts.m_codeFontSize, scale, " ");

    const auto firstLinePageIdx = pdfData.m_currentPainterIdx;
    const auto firstLineY = pdfData.m_layout.y();
    const auto firstLineHeight = lineHeight;

    while (i < lines.size()) {
        auto y = pdfData.m_layout.y();
        int j = i;
        double h = 0.0;

        while ((y - lineHeight > pdfData.currentPageAllowedY() ||
                qAbs(y - lineHeight - pdfData.currentPageAllowedY()) < 0.1) && j < lines.size()) {
            h += lineHeight;
            y -= lineHeight;
            ++j;
        }

        if (i < j) {
            pdfData.setColor(m_opts.m_syntax->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding));
            pdfData.drawRectangle(pdfData.m_layout.startX(pdfData.m_layout.availableWidth()),
                                  y,
                                  pdfData.m_layout.availableWidth(),
                                  h,
                                  PoDoFo::PdfPathDrawMode::Fill);
            pdfData.restoreColor();

            ret.append({pdfData.m_currentPainterIdx, y, h});
        }

        for (; i < j; ++i) {
            pdfData.m_layout.moveXToBegin();

            pdfData.m_layout.addY(lineHeight);

            while (true) {
                if (currentWord == colored.size() || colored[currentWord].line != i) {
                    break;
                }

                pdfData.setColor(colored[currentWord].format.textColor(m_opts.m_syntax->theme()));

                const auto length = colored[currentWord].endPos - colored[currentWord].startPos + 1;

                Font *f = font;

                const auto italic = colored[currentWord].format.isItalic(m_opts.m_syntax->theme());
                const auto bold = colored[currentWord].format.isBold(m_opts.m_syntax->theme());

                if (italic || bold) {
                    f = createFont(m_opts.m_codeFont, bold, italic, m_opts.m_codeFontSize, pdfData.m_doc, scale, pdfData);
                }

                pdfData.drawText(pdfData.m_layout.startX(spaceWidth * length),
                                 pdfData.m_layout.y() - pdfData.fontDescent(font, m_opts.m_codeFontSize, scale),
                                 createUtf8String(lines.at(i).mid(colored[currentWord].startPos, length)),
                                 f,
                                 m_opts.m_codeFontSize * scale,
                                 1.0,
                                 false);

                pdfData.m_layout.addX(spaceWidth * length);

                pdfData.restoreColor();

                ++currentWord;
            }
        }

        if (i < lines.size()) {
            createPage(pdfData);
            pdfData.m_layout.moveXToBegin();
        }
    }

    pdfData.m_layout.setRightToLeft(wasRightToLeft);
    pdfData.m_firstOnPage = false;

    return {ret, {firstLinePageIdx, firstLineY, firstLineHeight}};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawBlockquote(PdfAuxData &pdfData,
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

    QScopedValueRollback continueParagraph(pdfData.m_continueParagraph, true);

    // Draw items.
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
                const auto where =
                    drawHeading(pdfData,
                                static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                                doc,
                                offset + s_blockquoteBaseOffset,
                                (it + 1 != last ? minNecessaryHeight(pdfData, *(it + 1), doc,
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
                drawCode(pdfData, static_cast<MD::Code<MD::QStringTrait> *>(
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

            const auto where = drawList(pdfData, list, doc, bulletWidth,
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
                                         static_cast<MD::Table<MD::QStringTrait> *>(it->get()),
                                         doc,
                                         offset + s_blockquoteBaseOffset,
                                         heightCalcOpt,
                                         scale);

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

    const auto painterIdx = pdfData.m_currentPainterIdx;

    // Draw blockquote left vertival bar.
    for (auto it = map.cbegin(), last = map.cend(); it != last; ++it) {
        pdfData.m_currentPainterIdx = it.key();
        pdfData.setColor(color.isValid() ? color : m_opts.m_borderColor);
        pdfData.drawRectangle(pdfData.m_layout.borderStartX() + pdfData.m_layout.xIncrementDirection() *
                              (offset + (pdfData.m_layout.isRightToLeft() ? s_blockquoteMarkWidth : 0.0)), it.value().m_y,
                              s_blockquoteMarkWidth, it.value().m_height, PoDoFo::PdfPathDrawMode::Fill);
        pdfData.restoreColor();
    }

    pdfData.m_currentPainterIdx = painterIdx;

    return {ret, firstLine};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawList(PdfAuxData &pdfData,
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

        if (m_terminate) {
            return {};
        }
    }

    if (heightCalcOpt == CalcHeightOpt::Unknown) {
        emit status(tr("Drawing list."));
    }

    int idx = 1;
    ListItemType prevListItemType = ListItemType::Unknown;
    bool first = true;
    WhereDrawn firstLine;

    QScopedValueRollback continueParagraph(pdfData.m_continueParagraph, true);

    for (auto it = item->items().cbegin(), last = item->items().cend(); it != last; ++it) {
        if ((*it)->type() == MD::ItemType::ListItem) {
            const auto where = drawListItem(pdfData,
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

    return {ret, firstLine};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawListItem(PdfAuxData &pdfData,
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

    auto *font = createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize, pdfData.m_doc, scale, pdfData);

    const auto lineHeight = pdfData.lineSpacing(font, m_opts.m_textFontSize, scale);
    const auto orderedListNumberWidth =
        pdfData.stringWidth(font, m_opts.m_textFontSize, scale, "9") * bulletWidth
            + pdfData.stringWidth(font, m_opts.m_textFontSize, scale, ".");
    const auto spaceWidth = pdfData.stringWidth(font, m_opts.m_textFontSize, scale, " ");
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
                                               static_cast<MD::Heading<MD::QStringTrait> *>(it->get()),
                                               doc,
                                               offset,
                                               (it + 1 != last ? minNecessaryHeight(pdfData, *(it + 1), doc, offset, scale) : 0.0),
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
            const auto where = drawCode(pdfData, static_cast<MD::Code<MD::QStringTrait> *>(
                                            it->get()), doc, offset, heightCalcOpt, scale);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }

            addExtraSpace = (it != item->items().cbegin());
        } break;

        case MD::ItemType::Blockquote: {
            const auto where =
                drawBlockquote(pdfData, static_cast<MD::Blockquote<MD::QStringTrait> *>(
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

            const auto where = drawList(pdfData, list, doc, maxListNumberWidth(list),
                                        offset, heightCalcOpt, scale, true, rtl);

            ret.append(where.first);

            if (first) {
                firstLine = where.second;
                first = false;
            }
        } break;

        case MD::ItemType::Table: {
            const auto where = drawTable(pdfData, static_cast<MD::Table<MD::QStringTrait> *>(it->get()),
                                         doc, offset, heightCalcOpt, scale);

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
        ret.append({pdfData.m_currentPainterIdx, pdfData.m_layout.y(), lineHeight});

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
            const auto painterIdx = pdfData.m_currentPainterIdx;
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
                                 firstLine.m_y - pdfData.fontDescent(font, m_opts.m_textFontSize, scale),
                                 createUtf8String(idxText),
                                 font,
                                 m_opts.m_textFontSize * scale,
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

            pdfData.m_currentPainterIdx = painterIdx;
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

double PdfRenderer::rowHeight(PdfAuxData &pdfData,
                              std::shared_ptr<MD::TableRow<MD::QStringTrait>> row,
                              double width,
                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                              double scale)
{
    if (!row->cells().size()) {
        return 0.0;
    }

    double height = 0.0;
    auto leftMargin = pdfData.m_layout.margins().m_left;
    auto rightMargin = pdfData.m_layout.margins().m_right;
    pdfData.m_layout.margins().m_left = pdfData.m_layout.pageWidth() - width;
    pdfData.m_layout.margins().m_right = 0.0;

    for (const auto &c : row->cells()) {
        auto p = std::make_shared<MD::Paragraph<MD::QStringTrait>>();
        p->applyBlock(*c.get());

        const auto r = drawParagraph(pdfData, p.get(), doc, 0.0, false, CalcHeightOpt::Full, scale);

        const auto tmp = totalHeight(r.first, false) + 2.0 * s_tableMargin;

        if (tmp > height) {
            height = tmp;
        }
    }

    pdfData.m_layout.margins().m_left = leftMargin;
    pdfData.m_layout.margins().m_right = rightMargin;

    return height;
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawTable(PdfAuxData &pdfData,
                                                              MD::Table<MD::QStringTrait> *item,
                                                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                              double offset,
                                                              CalcHeightOpt heightCalcOpt,
                                                              double scale)
{
    QVector<WhereDrawn> ret;

    if (!item->rows().size() || !item->rows().at(0)->cells().size()) {
        return {ret, {}};
    }

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

    auto *font = createFont(m_opts.m_textFont, false, false, m_opts.m_textFontSize,
                            pdfData.m_doc, scale, pdfData);
    const auto lineHeight = pdfData.lineSpacing(font, m_opts.m_textFontSize, scale);
    const auto columnWidth = (pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
            pdfData.m_layout.margins().m_right - offset) / (double) item->rows().at(0)->cells().size() -
            s_tableMargin * 2.0;
    const bool justHeader = item->rows().size() == 1;
    const auto r0h = rowHeight(pdfData, item->rows().at(0), columnWidth, doc, scale);
    const auto r1h = (!justHeader ? rowHeight(pdfData, item->rows().at(1), columnWidth, doc, scale) : 0.0);

    switch (heightCalcOpt) {
    case CalcHeightOpt::Minimum: {
        ret.append({-1, 0.0, r0h + r1h, (!pdfData.m_firstOnPage ? lineHeight : 0.0)});

        return {ret, {}};
    }

    case CalcHeightOpt::Full: {
        ret.append({-1, 0.0, r0h + r1h, (!pdfData.m_firstOnPage ? lineHeight : 0.0)});

        for(long long int i = 2; i < item->rows().size(); ++i) {
            ret.append({-1, 0.0, rowHeight(pdfData, item->rows().at(i), columnWidth, doc, scale)});
        }

        return {ret, {}};
    }

    default:
        break;
    }

    const auto nonSplittableHeight = r0h + r1h;

    QScopedValueRollback continueParagraph(pdfData.m_continueParagraph, true);

    if ((pdfData.m_layout.y() - nonSplittableHeight) < pdfData.currentPageAllowedY() &&
        qAbs(pdfData.m_layout.y() - nonSplittableHeight - pdfData.currentPageAllowedY()) > 0.1) {
        createPage(pdfData);

        pdfData.freeSpaceOn(pdfData.m_currentPainterIdx);
    }

    if (!pdfData.m_firstOnPage) {
        moveToNewLine(pdfData, offset, lineHeight, 1.0, 0.0);
    }

    const auto rightToLeft = isRightToLeft(item->rows().at(0)->cells().at(0).get());

    bool first = true;
    WhereDrawn firstLine;

    const auto columnsCount = item->rows().at(0)->cells().size();

    QScopedValueRollback tableDrawing(pdfData.m_tableDrawing, true);

    for (const auto &row : std::as_const(item->rows())) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        const auto where = drawTableRow(row, pdfData, doc, item, offset, scale, columnWidth, rightToLeft,
                                        columnsCount);

        ret.append(where.first);

        if (first) {
            firstLine = where.second;
            first = false;
        }
    }

    pdfData.m_firstOnPage = false;
    pdfData.m_cachedPainters.clear();

    return {ret, firstLine};
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawTableRow(std::shared_ptr<MD::TableRow<MD::QStringTrait>> row,
                                                    PdfAuxData &pdfData,
                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                    MD::Table<MD::QStringTrait> *table,
                                                    double offset,
                                                    double scale,
                                                    double columnWidth,
                                                    bool rightToLeftTable,
                                                    int columnsCount)
{
    QVector<WhereDrawn> ret;
    const auto y = pdfData.m_layout.y();

    const auto startPage = pdfData.m_currentPainterIdx;
    int endPage = startPage;
    int currentPage = startPage;

    auto endY = pdfData.m_layout.pageHeight();

    pdfData.m_startLine = row->startLine();
    pdfData.m_startPos = row->startColumn();
    pdfData.m_endLine = row->endLine();
    pdfData.m_endPos = row->endColumn();

    int i = 0;

    const auto leftMargin = pdfData.m_layout.margins().m_left;
    const auto rightMargin = pdfData.m_layout.margins().m_right;

    WhereDrawn firstLine;

    pdfData.m_cachedPainters.clear();
    pdfData.m_cachedPainters.insert(startPage, 0);

    for (const auto &c : std::as_const(row->cells())) {
        {
            QMutexLocker lock(&m_mutex);

            if (m_terminate) {
                return {};
            }
        }

        pdfData.m_currentPainterIdx = startPage;
        pdfData.m_layout.setY(y);
        pdfData.m_layout.addY(s_tableMargin);

        pdfData.m_layout.margins().m_left = (rightToLeftTable ?
                                                 pdfData.m_layout.pageWidth() - rightMargin -
                                                 (columnWidth + s_tableMargin * 2.0) * (i + 1) :
                                                 leftMargin + i * (columnWidth + s_tableMargin * 2.0)) +
                (pdfData.m_layout.isRightToLeft() ? -offset : offset) + s_tableMargin;
        pdfData.m_layout.margins().m_right = pdfData.m_layout.pageWidth() - pdfData.m_layout.margins().m_left -
                columnWidth;

        const auto w = drawTableCell(c, pdfData, doc, table->columnAlignment(i), scale);

        double tmpY = 0.0;

        if (!w.first.isEmpty()) {
            tmpY = w.first.last().m_y - s_tableMargin;
            endPage = w.first.last().m_pageIdx;

            if (i == 0) {
                firstLine = w.second;
            }
        } else {
            tmpY = y - s_tableMargin * 2.0;

            firstLine.m_pageIdx = startPage;
            firstLine.m_y = y - s_tableMargin * 2.0;
            firstLine.m_height = s_tableMargin * 2.0;

            if (firstLine.m_y < pdfData.m_layout.margins().m_bottom) {
                createPage(pdfData);
                firstLine.m_y = pdfData.m_layout.margins().m_bottom;

                tmpY = pdfData.m_layout.pageHeight() - pdfData.m_layout.margins().m_top -
                        (s_tableMargin * 2.0 - (y - pdfData.m_layout.margins().m_bottom)) -
                        (pdfData.m_drawFootnotes ? pdfData.m_extraInFootnote : 0.0);
                firstLine.m_height = y - pdfData.m_layout.margins().m_bottom;
            }

            endPage = pdfData.m_currentPainterIdx;
        }

        if ((tmpY < endY && endPage == currentPage) || endPage > currentPage) {
            endY = tmpY;
            currentPage = endPage;
        }

        ++i;

        if (i == columnsCount) {
            break;
        }
    }

    pdfData.m_layout.setY(endY);
    pdfData.m_layout.margins().m_left = leftMargin;
    pdfData.m_layout.margins().m_right = rightMargin;

    drawRowBorder(pdfData, startPage, ret, offset, y, endY, columnWidth, columnsCount);

    pdfData.m_firstOnPage = false;

    return {ret, firstLine};
}

inline ParagraphAlignment columnAlignmentToParagraphAlignment(MD::Table<MD::QStringTrait>::Alignment align)
{
    switch (align) {
    case MD::Table<MD::QStringTrait>::AlignLeft:
        return ParagraphAlignment::Left;

    case MD::Table<MD::QStringTrait>::AlignRight:
        return ParagraphAlignment::Right;

    case MD::Table<MD::QStringTrait>::AlignCenter:
        return ParagraphAlignment::Center;

    default:
        return ParagraphAlignment::FillWidth;
    }
}

QPair<QVector<WhereDrawn>, WhereDrawn> PdfRenderer::drawTableCell(std::shared_ptr<MD::TableCell<MD::QStringTrait>> cell,
                                                    PdfAuxData &pdfData,
                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                    MD::Table<MD::QStringTrait>::Alignment align,
                                                    double scale)
{
    pdfData.m_startLine = cell->startLine();
    pdfData.m_startPos = cell->startColumn();
    pdfData.m_endLine = cell->endLine();
    pdfData.m_endPos = cell->endColumn();

    const auto wasRightToLeft = pdfData.m_layout.isRightToLeft();

    pdfData.m_layout.setRightToLeft(isRightToLeft(cell.get()));

    auto p = std::make_shared<MD::Paragraph<MD::QStringTrait>>();
    p->applyBlock(*cell.get());

    const auto ret = drawParagraph(pdfData, p.get(), doc, 0.0, false, CalcHeightOpt::Unknown, scale,
                                   Qt::black, false, nullptr, columnAlignmentToParagraphAlignment(align));

    pdfData.m_layout.setRightToLeft(wasRightToLeft);

    pdfData.m_firstOnPage = false;

    return ret;
}

void PdfRenderer::addFootnote(const QString &refId,
                              std::shared_ptr<MD::Footnote<MD::QStringTrait>> f,
                              PdfAuxData &pdfData,
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
                        this->addFootnote(ref->id(), fit->second, pdfData, doc);
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
        auto h = footnoteHeight(tmpData, doc, f.get(), &lineHeight);

        reserveSpaceForFootnote(pdfData, h, pdfData.m_layout.y(), pdfData.m_currentPageIdx, lineHeight);

        pdfData.m_footnotesAnchorsMap = tmpData.m_footnotesAnchorsMap;

        findFootnoteRefs(f.get(), pdfData);
    }
}

void PdfRenderer::drawRowBorder(PdfAuxData &pdfData,
                                int startPage,
                                QVector<WhereDrawn> &ret,
                                double offset,
                                double startY,
                                double endY,
                                double columnWidth,
                                int columnsCount)
{
    const auto autoOffset = pdfData.m_layout.addOffset(offset, !pdfData.m_layout.isRightToLeft());

    for (int i = startPage; i <= pdfData.currentPageIndex(); ++i) {
        pdfData.m_currentPainterIdx = i;

        pdfData.setColor(m_opts.m_borderColor);

        auto startX = (pdfData.m_layout.isRightToLeft() ? pdfData.m_layout.rightBorderXWithOffset() :
                                                          pdfData.m_layout.leftBorderXWithOffset());
        auto endX = startX;

        for (int c = 0; c < columnsCount; ++c) {
            endX += pdfData.m_layout.xIncrementDirection() * (columnWidth + s_tableMargin * 2.0);
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

            for (int c = 0; c < columnsCount; ++c) {
                x += pdfData.m_layout.xIncrementDirection() * (columnWidth + s_tableMargin * 2.0);

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

            for (int c = 0; c < columnsCount; ++c) {
                x += pdfData.m_layout.xIncrementDirection() * (columnWidth + s_tableMargin * 2.0);

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

            for (int c = 0; c < columnsCount; ++c) {
                x += pdfData.m_layout.xIncrementDirection() * (columnWidth + s_tableMargin * 2.0);

                pdfData.drawLine(x, sy, x, y);
            }

            pdfData.drawLine(startX, y, endX, y);

            ret.append({pdfData.currentPageIndex(), endY, sy - endY});
        }

        pdfData.restoreColor();
    }
}

double PdfRenderer::minNecessaryHeight(PdfAuxData &pdfData,
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
            drawParagraph(tmp, static_cast<MD::Paragraph<MD::QStringTrait> *>(item.get()), doc, offset, true, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::Code: {
        ret = drawCode(tmp, static_cast<MD::Code<MD::QStringTrait> *>(item.get()), doc, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::Blockquote: {
        ret = drawBlockquote(tmp, static_cast<MD::Blockquote<MD::QStringTrait> *>(item.get()), doc, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::List: {
        auto *list = static_cast<MD::List<MD::QStringTrait> *>(item.get());
        const auto bulletWidth = maxListNumberWidth(list);

        ret = drawList(tmp, list, m_doc, bulletWidth, offset, CalcHeightOpt::Minimum, scale).first;
    } break;

    case MD::ItemType::Table: {
        ret = drawTable(tmp, static_cast<MD::Table<MD::QStringTrait> *>(item.get()), doc, offset, CalcHeightOpt::Minimum, scale).first;
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
