/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "src/converter/renderer.h"
#include "src/converter/syntax.h"
#include "src/converter/const.h"

#include "src/shared/utils.h"
#include "src/shared/plugins_page.h"

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/parser.h>

#include "test_const.h"

// MicroTex include.
#include <latex.h>

#include <QFontDatabase>
#include <QObject>
#include <QSignalSpy>
#include <QVector>
#include <QtTest/QtTest>

//! Prepare test data or do actual test?
static const bool s_printData = false;

//
// TestRender
//

class TestRender final : public QObject
{
    Q_OBJECT

private slots:
    //! Init tests.
    void initTestCase();
    //! Test footnotes rendering.
    void testFootnotes();
    //! Test table with images.
    void testTableWithImage();
    //! Test table with text.
    void testTableWithText();
    //! Test blockquotes.
    void testBlockquote();
    //! Complex test.
    void testComplex();
    //! Test image in text.
    void testImageInText();
    //! Footnote with table.
    void testFootnoteWithTable();
    //! Complex footnote.
    void testComplexFootnote();
    //! One more complex test.
    void testComplex3();

    //! Test footnotes rendering.
    void testFootnotesBigFont();
    //! Test table with images.
    void testTableWithImageBigFont();
    //! Test table with text.
    void testTableWithTextBigFont();
    //! Test blockquotes.
    void testBlockquoteBigFont();
    //! Complex test.
    void testComplexBigFont();
    //! Test image in text.
    void testImageInTextBigFont();
    //! Footnote with table.
    void testFootnoteWithTableBigFont();
    //! Complex footnote.
    void testComplexFootnoteBigFont();
    //! One more complex test.
    void testComplex3BigFont();
    //! Very long footnote.
    void testVeryLongFootnote();
    //! Very long footnote.
    void testVeryLongFootnoteBigFont();

    //! Test code.
    void testCode();
    //! Test complex 2.
    void testComplex2();

    //! Test task list.
    void testTaskList();
    //! Test task list.
    void testTaskListBigFont();

    //! Test link with inline content.
    void testLinks();
    //! Test link with inline content.
    void testLinksBigFont();

    //! Test math.
    void testMath();
    //! Test math.
    void testMathBigFont();

    //! Test placing of images.
    void testImagesPlacing();

    //! Test highlights of blockquotes.
    void testBlockquoteHighlighting();
    //! Test highlights of blockquotes.
    void testBlockquoteHighlightingBig();

    //! Test placing online/not online content in different cases.
    void testPlacing();

    //! Test descent calculation.
    void testDescent();

    //! Test super- and subscript.
    void testSuperSubScript();
    //! Test super- and subscript.
    void testSuperSubScriptBigFont();
}; // class TestRender

namespace MdPdf
{

namespace Render
{

namespace /* anonymous */
{

inline double mmInPt(double mm)
{
    return mm / s_mmInPt;
}

} /* namespace anonymous */

struct TestRendering {
    static void testRendering(const QString &fileName,
                              const QString &suffix,
                              const QVector<DrawPrimitive> &data,
                              double textFontSize,
                              double codeFontSize,
                              ImageAlignment align = ImageAlignment::Center,
                              bool withPlugins = false)
    {
        MD::Parser<MD::QStringTrait> parser;

        if (withPlugins) {
            MdShared::PluginsCfg pluginsCfg;
            pluginsCfg.m_sup.m_delimiter = QLatin1Char('^');
            pluginsCfg.m_sup.m_on = true;
            pluginsCfg.m_sub.m_delimiter = QLatin1Char('-');
            pluginsCfg.m_sub.m_on = true;
            pluginsCfg.m_mark.m_delimiter = QLatin1Char('=');
            pluginsCfg.m_mark.m_on = true;
            setPlugins(parser, pluginsCfg);
        }

        auto doc = parser.parse(s_folder + QStringLiteral("/../../manual/") + fileName, true);

        RenderOpts opts;
        opts.m_borderColor = QColor(81, 81, 81);
        opts.m_linkColor = QColor(33, 122, 255);
        opts.m_bottom = mmInPt(20.0);
        opts.m_syntax = std::make_shared<Syntax>();
        opts.m_syntax->setTheme(opts.m_syntax->themeForName(QStringLiteral("GitHub Light")));
        opts.m_codeFont = QStringLiteral("Space Mono");
        opts.m_codeFontSize = codeFontSize;
        opts.m_left = mmInPt(20.0);
        opts.m_linkColor = QColor(33, 122, 255);
        opts.m_right = mmInPt(20.0);
        opts.m_textFont = QStringLiteral("Droid Serif");
        opts.m_textFontSize = textFontSize;
        opts.m_top = mmInPt(20.0);
        opts.m_dpi = 150;
        opts.m_imageAlignment = align;

        opts.m_testData = data;
        opts.m_printDrawings = s_printData;
        opts.m_testDataFileName = s_folder + QStringLiteral("/") + fileName + suffix + QStringLiteral(".data");

        PdfRenderer render;

        render.render(QStringLiteral("./") + fileName + suffix + QStringLiteral(".pdf"), doc, opts);
        render.renderImpl();

        if (render.isError()) {
            QFAIL("Rendering of Markdown failed. Test aborted.");
        }
    }

    static void testDescent()
    {
        {
            PdfRenderer::CustomWidth cw;

            cw.append({10., 10., false, false, true, {}, 5.});
            cw.append({10., 20., false, false, true, {}, 10.});
            cw.append({10., 20., false, false, true, {}, 15.});
            cw.append({0., 0., false, true, true});

            cw.calcScale(100.);

            QVERIFY(qFuzzyCompare(cw.descent(), 15.));
            QVERIFY(qFuzzyCompare(cw.height(), 25.));
        }

        {
            PdfRenderer::CustomWidth cw;

            cw.append({10., 10., false, false, true, {}, 9.});
            cw.append({10., 20., false, false, true, {}, 1.});
            cw.append({10., 20., false, false, true, {}, 1.});
            cw.append({0., 0., false, true, true});

            cw.calcScale(100.);

            QVERIFY(qFuzzyCompare(cw.descent(), 9.));
            QVERIFY(qFuzzyCompare(cw.height(), 28.));
        }

        {
            PdfRenderer::CustomWidth cw;

            cw.append({10., 10., false, false, true, {}, 9.});
            cw.append({10., 10., false, false, true, {}, 10.});
            cw.append({0., 0., false, true, true});

            cw.calcScale(100.);

            QVERIFY(qFuzzyCompare(cw.descent(), 10.));
            QVERIFY(qFuzzyCompare(cw.height(), 11.));
        }

        {
            PdfRenderer::CustomWidth cw;

            cw.append({10., 10., false, false, true, {}, 0.});
            cw.append({10., 10., false, false, true, {}, 10.});
            cw.append({10., 10., false, false, true, {}, 0.});
            cw.append({0., 0., false, true, true});

            cw.calcScale(100.);

            QVERIFY(qFuzzyCompare(cw.descent(), 10.));
            QVERIFY(qFuzzyCompare(cw.height(), 20.));
        }

        {
            PdfRenderer::CustomWidth cw;

            cw.append({10., 20., false, false, true, {}, 1.});
            cw.append({10., 20., false, false, true, {}, 1.});
            cw.append({10., 10., false, false, true, {}, 9.});
            cw.append({0., 0., false, true, true});

            cw.calcScale(100.);

            QVERIFY(qFuzzyCompare(cw.descent(), 9.));
            QVERIFY(qFuzzyCompare(cw.height(), 28.));
        }
    }
};

DrawPrimitive::Type toType(const QString &t)
{
    if (t == QStringLiteral("Text")) {
        return DrawPrimitive::Type::Text;
    } else if (t == QStringLiteral("MultilineText")) {
        return DrawPrimitive::Type::MultilineText;
    } else if (t == QStringLiteral("Line")) {
        return DrawPrimitive::Type::Line;
    } else if (t == QStringLiteral("Rectangle")) {
        return DrawPrimitive::Type::Rectangle;
    } else if (t == QStringLiteral("Image")) {
        return DrawPrimitive::Type::Image;
    } else {
        return DrawPrimitive::Type::Unknown;
    }
}

QString readQuotedString(QTextStream &s, int length)
{
    QString ret;
    QChar c;

    while (c != QLatin1Char('"')) {
        s >> c;
    }

    for (int i = 0; i < length; ++i) {
        s >> c;
        ret.append(c);
    }

    s >> c;

    return ret;
}

QVector<DrawPrimitive> loadTestData(const QString &fileName, const QString &suffix)
{
    QVector<DrawPrimitive> data;

    QFile file(s_folder + QStringLiteral("/") + fileName + suffix + QStringLiteral(".data"));

    if (!file.open(QIODevice::ReadOnly)) {
        return data;
    }

    QTextStream stream(&file);

    while (!stream.atEnd()) {
        auto str = stream.readLine();

        if (str.isEmpty()) {
            break;
        }

        QTextStream s(&str);
        DrawPrimitive d;

        QString type;
        s >> type;
        d.m_type = toType(type);

        int length = 0;
        s >> length;

        d.m_text = readQuotedString(s, length);

        s >> d.m_x;
        s >> d.m_y;
        s >> d.m_x2;
        s >> d.m_y2;
        s >> d.m_width;
        s >> d.m_height;
        s >> d.m_xScale;
        s >> d.m_yScale;

        data.append(d);
    }

    file.close();

    return data;
}

} /* namespace Render */

} /* namespace MdPdf */

void doTest(const QString &fileName, const QString &suffix, double textFontSize, double codeFontSize,
            MdPdf::Render::ImageAlignment align = MdPdf::Render::ImageAlignment::Center,
            bool withPlugins = false)
{
    QVector<MdPdf::Render::DrawPrimitive> data;

    if (!s_printData) {
        data = MdPdf::Render::loadTestData(fileName, suffix);

        if (data.isEmpty()) {
            QFAIL("Failed to load test data.");
        }
    }

    MdPdf::Render::TestRendering::testRendering(fileName, suffix, data, textFontSize, codeFontSize, align, withPlugins);
}

void TestRender::initTestCase()
{
    QFontDatabase::addApplicationFont(s_font);
    QFontDatabase::addApplicationFont(s_italicFont);
    QFontDatabase::addApplicationFont(s_boldFont);
    QFontDatabase::addApplicationFont(s_boldItalicFont);
    QFontDatabase::addApplicationFont(s_monoFont);
    QFontDatabase::addApplicationFont(s_monoItalicFont);
    QFontDatabase::addApplicationFont(s_monoBoldFont);
    QFontDatabase::addApplicationFont(s_monoBoldItalicFont);

    tex::LaTeX::init(":/res");
}

void TestRender::testFootnotes()
{
    doTest(QStringLiteral("footnotes.md"), QString(), 8.0, 8.0);
}

void TestRender::testTableWithImage()
{
    doTest(QStringLiteral("table.md"), QString(), 8.0, 8.0);
}

void TestRender::testTableWithText()
{
    doTest(QStringLiteral("table2.md"), QString(), 8.0, 8.0);
}

void TestRender::testBlockquote()
{
    doTest(QStringLiteral("blockquote.md"), QString(), 8.0, 8.0);
}

void TestRender::testComplex()
{
    doTest(QStringLiteral("complex.md"), QString(), 8.0, 8.0);
}

void TestRender::testImageInText()
{
    doTest(QStringLiteral("image_in_text.md"), QString(), 8.0, 8.0);
}

void TestRender::testFootnoteWithTable()
{
    doTest(QStringLiteral("footnote2.md"), QString(), 8.0, 8.0);
}

void TestRender::testComplexFootnote()
{
    doTest(QStringLiteral("footnote3.md"), QString(), 8.0, 8.0);
}

void TestRender::testComplex3()
{
    doTest(QStringLiteral("complex3.md"), QString(), 8.0, 8.0);
}

void TestRender::testFootnotesBigFont()
{
    doTest(QStringLiteral("footnotes.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testTableWithImageBigFont()
{
    doTest(QStringLiteral("table.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testTableWithTextBigFont()
{
    doTest(QStringLiteral("table2.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testBlockquoteBigFont()
{
    doTest(QStringLiteral("blockquote.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testComplexBigFont()
{
    doTest(QStringLiteral("complex.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testImageInTextBigFont()
{
    doTest(QStringLiteral("image_in_text.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testFootnoteWithTableBigFont()
{
    doTest(QStringLiteral("footnote2.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testComplexFootnoteBigFont()
{
    doTest(QStringLiteral("footnote3.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testComplex3BigFont()
{
    doTest(QStringLiteral("complex3.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testCode()
{
    doTest(QStringLiteral("code.md"), QString(), 8.0, 8.0);
}

void TestRender::testComplex2()
{
    doTest(QStringLiteral("complex2.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testLinks()
{
    doTest(QStringLiteral("links.md"), QString(), 8.0, 8.0);
}

void TestRender::testLinksBigFont()
{
    doTest(QStringLiteral("links.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testTaskList()
{
    doTest(QStringLiteral("tasklist.md"), QString(), 8.0, 8.0);
}

void TestRender::testTaskListBigFont()
{
    doTest(QStringLiteral("tasklist.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testMath()
{
    doTest( QStringLiteral( "math.md" ), QString(), 8.0, 8.0 );
}

void TestRender::testMathBigFont()
{
    doTest( QStringLiteral( "math.md" ), QStringLiteral( "_big" ), 16.0, 14.0 );
}

void TestRender::testVeryLongFootnote()
{
    doTest(QStringLiteral("footnotes4.md"), QString(), 8.0, 8.0);
}

void TestRender::testVeryLongFootnoteBigFont()
{
    doTest(QStringLiteral("footnotes4.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testImagesPlacing()
{
    doTest(QStringLiteral("images_placing.md"), QString(), 8.0, 8.0);
}

void TestRender::testBlockquoteHighlighting()
{
    doTest(QStringLiteral("highlighted_quotes.md"), QString(), 8.0, 8.0);
}

void TestRender::testBlockquoteHighlightingBig()
{
    doTest(QStringLiteral("highlighted_quotes.md"), QStringLiteral("_big"), 16.0, 14.0);
}

void TestRender::testPlacing()
{
    doTest(QStringLiteral("different_placing_cases.md"), QString(), 8.0, 8.0, MdPdf::Render::ImageAlignment::Left);
}

void TestRender::testDescent()
{
    MdPdf::Render::TestRendering::testDescent();
}

void TestRender::testSuperSubScript()
{
    doTest(QStringLiteral("styles.md"), QString(), 8.0, 8.0, MdPdf::Render::ImageAlignment::Left, true);
}

void TestRender::testSuperSubScriptBigFont()
{
    doTest(QStringLiteral("styles.md"), QStringLiteral("_big"), 16.0, 14.0, MdPdf::Render::ImageAlignment::Left, true);
}

QTEST_MAIN(TestRender)

#include "main.moc"
