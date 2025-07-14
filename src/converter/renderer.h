/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-pdf include.
#include "syntax.h"

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/doc.h>
#include <md4qt/traits.h>

// Qt include.
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QNetworkReply>
#include <QObject>
#include <QStack>
#include <QSharedPointer>
#include <QTemporaryFile>

#ifdef MD_PDF_TESTING
#include <QFile>
#include <QTextStream>
#endif // MD_PDF_TESTING

// C++ include.
#include <functional>
#include <memory>
#include <string_view>
#include <vector>

// podofo include.
#include <podofo/podofo.h>

// resvg include.
#include <ResvgQt.h>

namespace MdPdf
{

namespace Render
{

struct Utf8String {
    QByteArray data;

    Utf8String(const QByteArray &a)
        : data(a)
    {
    }

    Utf8String(const char *s)
        : data(s)
    {
    }

    operator const char *() const
    {
        return data.data();
    }

    operator std::string_view() const
    {
        return data.data();
    }
}; // struct Utf8String

#define MD_PDF_USE_PODOFO
#ifdef MD_PDF_USE_PODOFO

using Font = PoDoFo::PdfFont;
using Document = PoDoFo::PdfMemDocument;
using Page = PoDoFo::PdfPage;
using Painter = PoDoFo::PdfPainter;
using Image = PoDoFo::PdfImage;
using Destination = PoDoFo::PdfDestination;
using Color = PoDoFo::PdfColor;
using Rect = PoDoFo::Rect;
using String = Utf8String;

#endif // MD_PDF_USE_PODOFO

//! Footnote scale.
static const double s_footnoteScale = 0.75;

#ifdef MD_PDF_TESTING
struct DrawPrimitive {
    enum class Type { Text = 0, Line, Rectangle, Image, MultilineText, Unknown };

    Type m_type;
    QString m_text;
    double m_x;
    double m_y;
    double m_x2;
    double m_y2;
    double m_width;
    double m_height;
    double m_xScale;
    double m_yScale;
};
#endif // MD_PDF_TESTING

//! Image alignment.
enum class ImageAlignment { Unknown, Left, Center, Right }; // enum ImageAlignment

//! Paragraph alignment.
enum class ParagraphAlignment {
    //! Unknown,
    Unknown,
    //! Left.
    Left,
    //! Center.
    Center,
    //! Right.
    Right,
    //! FillWidth
    FillWidth
}; // enum ParagraphAlignment

//
// RenderOpts
//

//! Options for rendering.
struct RenderOpts {
    //! Text font.
    QString m_textFont;
    //! Text font size.
    int m_textFontSize;
    //! Code font.
    QString m_codeFont;
    //! Code font size.
    int m_codeFontSize;
    //! Links color.
    QColor m_linkColor;
    //! Borders color.
    QColor m_borderColor;
    //! Left margin.
    double m_left;
    //! Right margin.
    double m_right;
    //! Top margin.
    double m_top;
    //! Bottom margin.
    double m_bottom;
    //! DPI.
    quint16 m_dpi;
    //! Syntax highlighter.
    std::shared_ptr<Syntax> m_syntax;
    //! Image alignment.
    ImageAlignment m_imageAlignment;

#ifdef MD_PDF_TESTING
    bool m_printDrawings = false;
    QVector<DrawPrimitive> m_testData;
    QString m_testDataFileName;
#endif // MD_PDF_TESTING
}; // struct RenderOpts

//
// Renderer
//

//! Abstract renderer.
class Renderer : public QObject
{
    Q_OBJECT

signals:
    //! Progress of rendering.
    void progress(int percent);
    //! Error.
    void error(const QString &msg);
    //! Rendering is done.
    void done(bool terminated);
    //! Status message.
    void status(const QString &msg);

public:
    Renderer() = default;
    ~Renderer() override = default;

    virtual void render(const QString &fileName, std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                        const RenderOpts &opts, bool testing = false) = 0;
    virtual void clean() = 0;
}; // class Renderer

static const double s_margin = 72.0 / 25.4 * 20.0;
static const double s_beforeHeading = 15.0;
static const double s_blockquoteBaseOffset = 10.0;
static const double s_blockquoteMarkWidth = 3.0;
static const double s_tableMargin = 2.0;

//! Mrgins.
struct PageMargins {
    double m_left = s_margin;
    double m_right = s_margin;
    double m_top = s_margin;
    double m_bottom = s_margin;
}; // struct PageMargins

//! Page current coordinates and etc...
struct CoordsPageAttribs {
    PageMargins m_margins;
    double m_pageWidth = 0.0;
    double m_pageHeight = 0.0;
    double m_x = 0.0;
    double m_y = 0.0;
}; // struct CoordsPageAttribs

class PdfRenderer;

//! Layout direction handler.
struct LayoutDirectionHandler {
    double x() const { return m_coords.m_x; }
    double y() const { return m_coords.m_y; }
    void setRightToLeft(bool on) { m_isRightToLeft = on; }
    bool isRightToLeft() const { return m_isRightToLeft; }
    void setX(double value) { m_coords.m_x = value; }
    void addX(double value) { m_coords.m_x += xIncrementDirection() * value; }
    void moveXToBegin() { setX((isRightToLeft() ? rightBorderXWithOffset() : leftBorderXWithOffset())); }
    void setY(double value) { m_coords.m_y = value; }
    void addY(double value, double direction = 1.0) { m_coords.m_y -= direction * value; }
    double leftBorderXWithOffset() const { return (m_coords.m_margins.m_left +
                                                   (!m_offset.empty() && m_offset.back()->m_left ?
                                                        m_offset.back()->m_value : 0.0)); }
    double rightBorderXWithOffset() const { return (m_coords.m_pageWidth - m_coords.m_margins.m_right -
                                                    (!m_offset.empty() && !m_offset.back()->m_left ?
                                                         m_offset.back()->m_value : 0.0)); }

    bool isFit(double width) const
    {
        return (isRightToLeft() ? (x() - width >= leftBorderXWithOffset() ||
                                   qAbs(leftBorderXWithOffset() - x() + width) < 0.01) :
                                  (x() + width <= rightBorderXWithOffset() ||
                                   qAbs(x() + width - rightBorderXWithOffset()) < 0.01));
    }

    double topY() const { return m_coords.m_pageHeight - m_coords.m_margins.m_top; }
    const PageMargins & margins() const { return m_coords.m_margins; }
    PageMargins & margins() { return m_coords.m_margins; }
    double pageWidth() const { return m_coords.m_pageWidth; }
    double pageHeight() const { return m_coords.m_pageHeight; }
    double borderStartX() const { return (isRightToLeft() ? m_coords.m_pageWidth - m_coords.m_margins.m_right :
                                                           m_coords.m_margins.m_left); }
    double xIncrementDirection() const { return (isRightToLeft() ? -1.0 : 1.0); }
    QRectF currentRect(double width, double height) const { return QRectF(startX(width), y(), width, height); }
    double startX(double width) const { return (isRightToLeft() ? x() - width : x()); }
    double availableWidth() const { return (isRightToLeft() ? x() - leftBorderXWithOffset() :
                                                              rightBorderXWithOffset() - x()); }

    struct Offset {
        Offset(std::vector<Offset*> &offsets, double value, bool left)
            : m_value(value)
            , m_left(left)
            , m_offsets(offsets)
        {
            m_offsets.push_back(this);
        }

        ~Offset()
        {
            m_offsets.pop_back();
        }

        Offset(const Offset &) = delete;
        Offset & operator=(const Offset &) = delete;

        double m_value = 0.0;
        bool m_left = true;

    private:
        std::vector<Offset*> &m_offsets;
    };

    Offset addOffset(double value, bool left) { return Offset(m_offset, value, left); }

    //! Coordinates and margins.
    CoordsPageAttribs m_coords;

private:
    bool m_isRightToLeft = false;
    std::vector<Offset*> m_offset;
}; // struct LayoutDirectionHandler

//! Auxiliary struct for rendering.
struct PdfAuxData {
    //! Document.
    Document *m_doc = nullptr;
    //! Painters.
    std::vector<std::shared_ptr<Painter>> *m_painters = nullptr;
    //! Page.
    Page *m_page = nullptr;
    //! Index of the current page.
    int m_currentPageIdx = -1;
    //! Layout direction handler.
    LayoutDirectionHandler m_layout;
    //! Anchors in document.
    QStringList m_anchors;
    //! Reserved spaces on the pages for footnotes.
    QMap<unsigned int, double> m_reserved;
    //! Drawing footnotes or the document?
    bool m_drawFootnotes = false;
    //! Current page index for drawing footnotes.
    int m_footnotePageIdx = -1;
    //! Current painter index.
    int m_currentPainterIdx = -1;
    //! Current index of the footnote (for drawing number in the PDF).
    int m_currentFootnote = 1;
    //! Is this first item on the page?
    bool m_firstOnPage = true;
    //! Continue drawing of paragraph?
    bool m_continueParagraph = false;
    //! Current line height.
    double m_lineHeight = 0.0;
    //! Extra space before footnotes.
    double m_extraInFootnote = 0.0;
    //! Colors stack.
    QStack<QColor> m_colorsStack;
    //! Markdown document.
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_md;
    //! Start line of procesing in the document.
    long long int m_startLine = 0;
    //! Start position in the start line.
    long long int m_startPos = 0;
    //! End line of procesing in the document.
    long long int m_endLine = 0;
    //! End position in the end line.
    long long int m_endPos = 0;
    //! Footnote counter.
    int m_footnoteNum = 1;
    //! Current file.
    QString m_currentFile;
    //! Footnotes map to map anchors.
    QMap<MD::Footnote<MD::QStringTrait> *, QPair<QString, int>> m_footnotesAnchorsMap;
    //! Resvg options.
    QSharedPointer<ResvgOptions> m_resvgOpts;
    //! Special blockquotes that should be highlighted.
    QMap<MD::Blockquote<MD::QStringTrait> *, QColor> m_highlightedBlockquotes;
    //! Cache of fonts.
    QMap<QString, QSharedPointer<QTemporaryFile>> m_fontsCache;
    //! Stack of painters used on table drawing.
    QMap<int, char> m_cachedPainters;
    //! Flag when drawing table.
    bool m_tableDrawing = false;

#ifdef MD_PDF_TESTING
    QMap<QString, QString> m_fonts;
    QSharedPointer<QFile> m_drawingsFile;
    QSharedPointer<QTextStream> m_drawingsStream;
    bool m_printDrawings = false;
    QVector<DrawPrimitive> m_testData;
    int m_testPos = 0;
    PdfRenderer *m_self = nullptr;
#endif // MD_PDF_TESTING

    //! \return Top Y coordinate on the page.
    double topY(int page) const;
    //! \return Current page index.
    int currentPageIndex() const;
    //! \return Top footnote Y coordinate on the page.
    double topFootnoteY(int page) const;
    //! \return Minimum allowe Y coordinate on the current page.
    double currentPageAllowedY() const;
    //! \return Minimum allowe Y coordinate on the page.
    double allowedY(int page) const;
    //! Reserve space for drawing, i.e. move footnotes on the next page.
    void freeSpaceOn(int page);

    //! Draw text
    void drawText(double x, double y, const char *text, Font *font, double size, double scale, bool strikeout);
    //! Draw image.
    void drawImage(double x, double y, Image *img, double xScale, double yScale);
    //! Draw line.
    void drawLine(double x1, double y1, double x2, double y2);
    //! Save document.
    void save(const QString &fileName);
    //! Draw rectangle.
    void drawRectangle(double x, double y, double width, double height, PoDoFo::PdfPathDrawMode m);

    //! Set color.
    void setColor(const QColor &c);
    //! Restore color.
    void restoreColor();
    //! Repeat color (needed after new page creation).
    void repeatColor();

    //! \return String width.
    double stringWidth(Font *font, double size, double scale, const String &s) const;
    //! \return Line spacing.
    double lineSpacing(Font *font, double size, double scale) const;
    //! \return Font ascent.
    double fontAscent(Font *font, double size, double scale) const;
    //! \return Font descent.
    double fontDescent(Font *font, double size, double scale) const;
}; // struct PdfAuxData;

//! Where was the item drawn?
struct WhereDrawn {
    //! Page painter index.
    int m_pageIdx = -1;
    //! Y of line's bottom.
    double m_y = 0.0;
    //! Height of the item.
    double m_height = 0.0;
    //! Extra height that can be skipped (usually extra line before new paragraph or heading).
    double m_extraHeight = 0.0;
}; // struct WhereDrawn

//
// PdfRenderer
//

//! Renderer to PDF.
class PdfRenderer : public Renderer
{
    Q_OBJECT

signals:
    //! Internal signal for start rendering.
    void start();

public:
    PdfRenderer();
    ~PdfRenderer() override = default;

    //! \return Is font can be created?
    static bool isFontCreatable(const QString &font, bool monospace);

    //! Convert QString to UTF-8.
    static Utf8String createUtf8String(const QString &text);
    //! Convert UTF-8 to QString.
    static QString createQString(const char *str);

#ifdef MD_PDF_TESTING
    bool isError() const;
#endif

public slots:
    //! Render document. \note Document can be changed during rendering.
    //! Don't reuse the same document twice.
    //! Renderer will delete himself on job finish.
    void render(const QString &fileName, std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                const MdPdf::Render::RenderOpts &opts, bool testing = false) override;
    //! Terminate rendering.
    void terminate();

private slots:
    //! Real rendering.
    void renderImpl();
    //! Clean render.
    void clean() override;

protected:
#ifdef MD_PDF_TESTING
    friend struct TestRendering;
#endif

    //! Create font.
    Font *createFont(const QString &name, bool bold, bool italic, double size,
                     Document *doc, double scale, const PdfAuxData &pdfData);

private:
    //! Create new page.
    void createPage(PdfAuxData &pdfData);

    //! Draw empty line.
    void moveToNewLine(
            //! Auxiliary PDF data.
            PdfAuxData &pdfData,
            //! Not used now.
            double xOffset,
            //! Offset for Y coordinate.
            double yOffset,
            //! Multiplier for Y coordinate. Real offset will be yOffset * yOffsetMultiplier.
            double yOffsetMultiplier,
            //! Y offset on new page.
            double yOffsetOnNewPage);
    //! Load image.
    QByteArray loadImage(
            //! Image.
            MD::Image<MD::QStringTrait> *item,
            //! Options for SVG rendering.
            const ResvgOptions &opts,
            //! Height to scale image to.
            double height = 1.0,
            //! Should image be scaled?
            bool scale = false,
            //! Store in cache loaded image data?
            bool cache = true);
    //! Make all links clickable.
    void resolveLinks(PdfAuxData &pdfData);
    //! Max width of numbered list bullet.
    int maxListNumberWidth(MD::List<MD::QStringTrait> *list) const;

    //! What calculation of height to do?
    enum class CalcHeightOpt {
        //! Don't calculate, do drawing.
        Unknown = 0,
        //! Calculate minimum requred height (at least one line).
        Minimum = 1,
        //! Calculate full height.
        Full = 2
    }; // enum class CalcHeightOpt

    //! Finish pages.
    void finishPages(PdfAuxData &pdfData);

    //! Flag for RTL languages support.
    struct RTLFlag {
        RTLFlag()
            : m_isOn(false)
            , m_check(true)
        {
        }

        bool isCheck() const { return m_check; }
        bool isRightToLeft() const { return m_isOn; }

        bool m_isOn = false;
        bool m_check = true;
    };

    void setRTLFlagToFalseIfCheck(RTLFlag *rtl)
    {
        if (rtl && rtl->isCheck()) {
            rtl->m_check = false;
            rtl->m_isOn = false;
        }
    }

    void resetRTLFlagToDefaults(RTLFlag *rtl)
    {
        if (rtl) {
            *rtl = {};
        }
    }

    //! Draw heading.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawHeading(PdfAuxData &pdfData,
                                                       MD::Heading<MD::QStringTrait> *item,
                                                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                       double offset,
                                                       double nextItemMinHeight,
                                                       CalcHeightOpt heightCalcOpt,
                                                       double scale,
                                                       bool withNewLine = true,
                                                       RTLFlag *rtl = nullptr);
    //! Draw paragraph.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawParagraph(PdfAuxData &pdfData,
                                                         MD::Paragraph<MD::QStringTrait> *item,
                                                         std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                         double offset,
                                                         bool withNewLine,
                                                         CalcHeightOpt heightCalcOpt,
                                                         double scale,
                                                         const QColor &color = Qt::black,
                                                         bool scaleImagesToLineHeight = false,
                                                         RTLFlag *rtl = nullptr,
                                                         ParagraphAlignment align = ParagraphAlignment::FillWidth);
    //! Draw block of code.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawCode(PdfAuxData &pdfData,
                                                    MD::Code<MD::QStringTrait> *item,
                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                    double offset,
                                                    CalcHeightOpt heightCalcOpt,
                                                    double scale);
    //! Draw blockquote.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawBlockquote(PdfAuxData &pdfData,
                                                          MD::Blockquote<MD::QStringTrait> *item,
                                                          std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                          double offset,
                                                          CalcHeightOpt heightCalcOpt,
                                                          double scale,
                                                          RTLFlag *rtl = nullptr);
    //! Draw list.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawList(PdfAuxData &pdfData,
                                                    MD::List<MD::QStringTrait> *item,
                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                    int bulletWidth,
                                                    double offset = 0.0,
                                                    CalcHeightOpt heightCalcOpt = CalcHeightOpt::Unknown,
                                                    double scale = 1.0,
                                                    bool nested = false,
                                                    RTLFlag *rtl = nullptr);
    //! Draw table.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawTable(PdfAuxData &pdfData,
                                                     MD::Table<MD::QStringTrait> *item,
                                                     std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                     double offset,
                                                     CalcHeightOpt heightCalcOpt,
                                                     double scale);

    //! \return Minimum necessary height to draw item, meant at least one line.
    double minNecessaryHeight(PdfAuxData &pdfData,
                              std::shared_ptr<MD::Item<MD::QStringTrait>> item,
                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                              double offset,
                              double scale);
    //! \return Height of the footnote.
    QVector<WhereDrawn> drawFootnote(PdfAuxData &pdfData,
                                     std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                     const QString &footnoteRefId,
                                     MD::Footnote<MD::QStringTrait> *note,
                                     CalcHeightOpt heightCalcOpt,
                                     double *lineHeight = nullptr,
                                     RTLFlag *rtl = nullptr);
    //! \return Height of the footnote.
    QVector<WhereDrawn> footnoteHeight(PdfAuxData &pdfData,
                                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                       MD::Footnote<MD::QStringTrait> *note,
                                       double *lineHeight);
    //! Reserve space for footnote.
    void reserveSpaceForFootnote(PdfAuxData &pdfData,
                                 const QVector<WhereDrawn> &h,
                                 const double &currentY,
                                 int currentPage,
                                 double lineHeight,
                                 bool addExtraLine = false);
    //! Add footnote.
    void addFootnote(const QString &refId,
                     std::shared_ptr<MD::Footnote<MD::QStringTrait>> f,
                     PdfAuxData &pdfData,
                     std::shared_ptr<MD::Document<MD::QStringTrait>> doc);

    //! List item type.
    enum class ListItemType {
        Unknown,
        //! Ordered.
        Ordered,
        //! Unordered.
        Unordered
    }; // enum class ListItemType

    //! Draw list item.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawListItem(PdfAuxData &pdfData,
                                                        MD::ListItem<MD::QStringTrait> *item,
                                                        std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                        int &idx,
                                                        ListItemType &prevListItemType,
                                                        int bulletWidth,
                                                        double offset,
                                                        CalcHeightOpt heightCalcOpt,
                                                        double scale,
                                                        //! A very first item in list, even not nested first item in nested list.
                                                        bool firstInList,
                                                        //! Just first item in list, possibly in nested list.
                                                        bool firstItem,
                                                        RTLFlag *rtl = nullptr);

    //! Auxiliary struct for calculation of spaces scales to shrink text to width.
    struct CustomWidth {
        //! Item on line.
        struct Width {
            double m_width = 0.0;
            double m_height = 0.0;
            bool m_isSpace = false;
            bool m_isNewLine = false;
            bool m_shrink = true;
            QString m_word = {};
            double m_descent = 0.0;
            ParagraphAlignment m_alignment = ParagraphAlignment::Unknown;
        }; // struct Width

        //! Append new item.
        void append(const Width &w) { m_width.append(w); }
        //! \return Scale of space at line.
        double scale() const { return m_scale.at(m_pos); }
        //! \return Height of the line.
        double height() const { return m_height.at(m_pos); }
        //! \return Descent of the line.
        double descent() const { return m_descent.at(m_pos); }
        //! \return Width of the line.
        double width() const { return m_lineWidth.at(m_pos); }
        //! Move to next line.
        void moveToNextLine() { ++m_pos; }
        //! Is drawing? This struct can be used to precalculate widthes and for actual drawing.
        bool isDrawing() const { return m_drawing; }
        //! Set drawing.
        void setDrawing(bool on = true) { m_drawing = on; }
        //! \return Is last element is new line?
        bool isNewLineAtEnd() const { return (m_width.isEmpty() ? false : m_width.back().m_isNewLine); }
        //! \return Begin iterator.
        QVector<double>::ConstIterator cbegin() const { return m_height.cbegin(); }
        //! \return End iterator.
        QVector<double>::ConstIterator cend() const { return m_height.cend(); }
        //! \return Height of first item.
        double firstLineHeight() const;
        //! Calculate scales.
        void calcScale(double lineWidth);
        //! \return Paragraph alignment.
        ParagraphAlignment alignment() const { return m_alignment.at(m_pos); }
        //! Set paragraph alignment.
        void setAlignment(ParagraphAlignment alignment) { std::for_each(m_alignment.begin(), m_alignment.end(),
            [alignment](auto &a){ if (a == ParagraphAlignment::Unknown) { a = alignment; } }); }

    private:
        //! Is drawing?
        bool m_drawing = false;
        //! Sizes of items.
        QVector<Width> m_width;
        //! Scales on lines.
        QVector<double> m_scale;
        //! Heights of lines.
        QVector<double> m_height;
        //! Widthes of lines.
        QVector<double> m_lineWidth;
        //! Descents.
        QVector<double> m_descent;
        //! Position of current line.
        int m_pos = 0;
        //! Alignments of lines.
        QVector<ParagraphAlignment> m_alignment;
    }; // struct CustomWidth

    //! Align line.
    void alignLine(PdfAuxData &pdfData, const CustomWidth &cw);

    //! Baseline delta and scale of previous item.
    //! Used for calculating superscript and subscript.
    struct PrevBaselineState {
        //! Baseline delta.
        double m_baselineDelta = 0.0;
        //! Scale.
        double m_scale = 1.0;
        //! Line height.
        double m_lineHeight = 0.0;
        //! Descent.
        double m_descent = 0.0;
    }; // struct PrevBaselineState

    //! Baseline delta and scale of previous item.
    //! Used for calculating superscript and subscript.
    struct PrevBaselineStateStack {
        explicit PrevBaselineStateStack(double lineHeight,
                                        double descent)
        {
            m_stack.push_back({0.0, 1.0, lineHeight, descent});
        }

        static const double s_scale;
        static const double s_baselineScale;

        double nextLineHeight() const
        {
            return m_stack.back().m_lineHeight / s_scale;
        }

        double nextBaselineDelta() const
        {
            return (m_stack.back().m_lineHeight - currentDescent()) * s_baselineScale;
        }

        double nextScale() const
        {
            return m_stack.back().m_scale / s_scale;
        }

        double currentDescent() const
        {
            return m_stack.back().m_descent;
        }

        double nextDescent() const
        {
            return currentDescent() / s_scale;
        }

        // pair.first - line height, pair.second - lower part, below descent.
        std::pair<double,
                  double>
        fullLineHeight() const
        {
            const auto firstHeight = m_stack.front().m_lineHeight;
            const auto firstDescent = m_stack.front().m_descent;
            double upper = 0.0;
            double lower = 0.0;

            for (auto it = std::next(m_stack.cbegin()), last = m_stack.cend(); it != last; ++it) {
                if (it->m_baselineDelta > 0.0) {
                    if (it->m_lineHeight - it->m_descent + it->m_baselineDelta > firstHeight) {
                        const double tmp = it->m_lineHeight - it->m_descent + it->m_baselineDelta - firstHeight;

                        if (tmp > upper) {
                            upper = tmp;
                        }
                    }
                } else {
                    const double tmp = it->m_lineHeight - firstDescent;

                    if (tmp > lower) {
                        lower = tmp;
                    }
                }
            }

            return {firstHeight + upper + lower, lower};
        }

        //! Stack.
        std::vector<PrevBaselineState> m_stack;
        //! Is mark style applied?
        long long int m_mark = 0;
    }; // struct PrevBaselineStateStack

    //! Initialize baseline with the given item.
    void initSubSupScript(MD::ItemWithOpts<MD::QStringTrait> *item,
                          PrevBaselineStateStack &state);

    //! Deinit baseline with the given item.
    void deinitSubSupScript(MD::ItemWithOpts<MD::QStringTrait> *item, PrevBaselineStateStack &state);

    struct AutoSubSupScriptInit {
        AutoSubSupScriptInit(PdfRenderer *render,
                             MD::ItemWithOpts<MD::QStringTrait> *item,
                             PrevBaselineStateStack &stack)
            : m_render(render)
            , m_item(item)
            , m_stack(stack)
        {
            m_render->initSubSupScript(m_item, m_stack);
        }

        ~AutoSubSupScriptInit()
        {
            m_render->deinitSubSupScript(m_item, m_stack);
        }

        PdfRenderer *m_render;
        MD::ItemWithOpts<MD::QStringTrait> *m_item;
        PrevBaselineStateStack &m_stack;
    };

    //! Draw text.
    QPair<QVector<QPair<QRectF,
                        unsigned int>>,
          PrevBaselineStateStack>
    drawText(PdfAuxData &pdfData,
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
             const QColor &color = Qt::black,
             RTLFlag *rtl = nullptr);
    //! Draw inlined code.
    QPair<QVector<QPair<QRectF,
                        unsigned int>>,
          PrevBaselineStateStack>
    drawInlinedCode(PdfAuxData &pdfData,
                    MD::Code<MD::QStringTrait> *item,
                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                    bool &newLine,
                    double offset,
                    bool firstInParagraph,
                    CustomWidth &cw,
                    double scale,
                    const PrevBaselineStateStack &previousBaseline,
                    RTLFlag *rtl = nullptr,
                    bool inLink = false);

    //! Draw string.
    QPair<QVector<QPair<QRectF,
                        unsigned int>>,
          PrevBaselineStateStack>
    drawString(PdfAuxData &pdfData,
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
               const QColor &color = Qt::black,
               Font *regularSpaceFont = nullptr,
               double regularSpaceFontSize = 0.0,
               double regularSpaceFontScale = 0.0,
               RTLFlag *rtl = nullptr);
    //! Draw link.
    QPair<QVector<QPair<QRectF,
                        unsigned int>>,
          PrevBaselineStateStack>
    drawLink(PdfAuxData &pdfData,
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
             RTLFlag *rtl = nullptr);
    //! \return Is \par it a space?
    template<class Iterator>
    inline bool isSpace(Iterator it)
    {
        if ((*it)->type() == MD::ItemType::Text) {
            auto t = static_cast<MD::Text<MD::QStringTrait>*>(it->get());

            if (t->text().simplified().isEmpty()) {
                return true;
            }
        }

        return false;
    }
    //! \return Is \par it not HTML nor space?
    template<class Iterator>
    inline bool isNotHtmlNorSpace(Iterator it)
    {
        if ((*it)->type() != MD::ItemType::RawHtml) {
            if (isSpace(it)) {
                return false;
            } else {
                return true;
            }
        }

        return false;
    }
    //! \return Is \par it not HTML?
    template<class Iterator>
    inline bool isNotHtml(Iterator it)
    {
        return ((*it)->type() != MD::ItemType::RawHtml);
    }
    //! \return Is after \par it nothing except HTML, spaces.
    inline bool isNothingAfter(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                            MD::Block<MD::QStringTrait>::Items::const_iterator last);
    //! Skip backward til \par func returns true.
    template<class Iterator, class Func>
    inline Iterator
    skipBackwardWithFunc(Iterator it,
                         Iterator begin,
                         Iterator last,
                         Func func)
    {
        for (; it != begin; --it) {
            if (std::invoke(func, this, it)) {
                break;
            }
        }

        if (it == begin && begin != last) {
            if (std::invoke(func, this, it)) {
                return it;
            } else {
                return last;
            }
        }

        return it;
    }
    //! \return Previous not HTML item.
    MD::Item<MD::QStringTrait> *getPrevItem(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                                         MD::Block<MD::QStringTrait>::Items::const_iterator begin,
                                         MD::Block<MD::QStringTrait>::Items::const_iterator last);
    //! Skip raw HTML and spaces backward.
    inline MD::Block<MD::QStringTrait>::Items::const_iterator
    skipRawHtmlAndSpacesBackward(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                MD::Block<MD::QStringTrait>::Items::const_iterator begin,
                MD::Block<MD::QStringTrait>::Items::const_iterator last);
    //! Skip raw HTML and spaces.
    template<class Iterator>
    inline Iterator
    skipRawHtmlAndSpaces(Iterator it, Iterator last)
    {
        for (; it != last; ++it) {
            if (isNotHtmlNorSpace(it)) {
                break;
            }
        }

        return it;
    }
    //! \return Is item a online image, or link with last online image?
    bool isOnlineImageOrOnlineImageInLink(PdfAuxData &pdfData,
                                          MD::Item<MD::QStringTrait> *item,
                                          double offset,
                                          double lineHeight,
                                          bool scaleImagesToLineHeight);
    //! \return Is after \par it a text item or online content?
    bool isTextOrOnlineAfter(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                             MD::Block<MD::QStringTrait>::Items::const_iterator last,
                             PdfAuxData &pdfData,
                             double offset,
                             double lineHeight,
                             bool scaleImagesToLineHeight);
    //! \return Is before \par it a text item or online content?
    bool isTextOrOnlineBefore(MD::Block<MD::QStringTrait>::Items::const_iterator it,
                              MD::Block<MD::QStringTrait>::Items::const_iterator begin,
                              MD::Block<MD::QStringTrait>::Items::const_iterator last,
                              PdfAuxData &pdfData,
                              double offset,
                              double lineHeight,
                              bool scaleImagesToLineHeight);
    //! \return Is \par it a text or online content?
    template<class Iterator>
    bool isTextOrOnline(Iterator it,
                        Iterator last,
                        bool reverse,
                        PdfAuxData &pdfData,
                        double offset,
                        double lineHeight,
                        bool scaleImagesToLineHeight)
    {
        it = skipRawHtmlAndSpaces(it, last);

        if (it != last) {
            return isTextOrOnline(it, reverse, pdfData, offset, lineHeight, scaleImagesToLineHeight);
        } else {
            return false;
        }
    }
    //! \return Is \par it a text or online content?
    template<class Iterator>
    bool isTextOrOnline(Iterator it,
                        bool reverse,
                        PdfAuxData &pdfData,
                        double offset,
                        double lineHeight,
                        bool scaleImagesToLineHeight)
    {
        switch ((*it)->type()) {
        case MD::ItemType::Text:
        case MD::ItemType::Code:
            return true;

        case MD::ItemType::Math: {
            auto m = static_cast<MD::Math<MD::QStringTrait>*>(it->get());

            return m->isInline();
        }
            break;

        case MD::ItemType::Image: {
            return isOnlineImage(pdfData, static_cast<MD::Image<MD::QStringTrait>*>(it->get()),
                                 offset, lineHeight, scaleImagesToLineHeight);
        }
            break;

        case MD::ItemType::Link: {
            auto l = static_cast<MD::Link<MD::QStringTrait>*>(it->get());

            if (!l->p()->isEmpty()) {
                if (reverse) {
                    return isTextOrOnline(l->p()->items().crbegin(), l->p()->items().crend(), reverse,
                                          pdfData, offset, lineHeight, scaleImagesToLineHeight);
                } else {
                    return isTextOrOnline(l->p()->items().cbegin(), l->p()->items().cend(), reverse,
                                          pdfData, offset, lineHeight, scaleImagesToLineHeight);
                }
            } else if (l->img()->isEmpty()) {
                return true;
            } else {
                return isOnlineImage(pdfData, l->img().get(), offset, lineHeight, scaleImagesToLineHeight);
            }
        }
            break;

        default:
            return false;
        }

        return false;
    }
    //! \return Is image online?
    bool isOnlineImage(double totalAvailableWidth,
                       double iWidth,
                       double iHeight,
                       double lineHeight);
    //! \return Is image online?
    bool isOnlineImage(PdfAuxData &pdfData,
                       MD::Image<MD::QStringTrait> *item,
                       double offset,
                       double lineHeight,
                       bool scaleImagesToLineHeight);
    //! Draw image.
    QPair<QPair<QRectF,
                unsigned int>,
          PrevBaselineStateStack>
    drawImage(PdfAuxData &pdfData,
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
              ImageAlignment alignment = ImageAlignment::Unknown,
              bool scaleImagesToLineHeight = false);

    //! Draw math expression.
    QPair<QPair<QRectF,
                unsigned int>,
          PrevBaselineStateStack>
    drawMathExpr(PdfAuxData &pdfData,
                 MD::Math<MD::QStringTrait> *item,
                 std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                 MD::Item<MD::QStringTrait> *prevItem,
                 bool &newLine,
                 double offset,
                 bool isNextText,
                 bool firstInParagraph,
                 CustomWidth &cw,
                 double scale,
                 const PrevBaselineStateStack &previousBaseline);

    //! \return Height of the table's row.
    double rowHeight(PdfAuxData &pdfData,
                     std::shared_ptr<MD::TableRow<MD::QStringTrait>> row,
                     double width,
                     std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                     double scale);

    //! Draw table's row.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawTableRow(std::shared_ptr<MD::TableRow<MD::QStringTrait>> row,
                                                        PdfAuxData &pdfData,
                                                        std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                        MD::Table<MD::QStringTrait> *table,
                                                        double offset,
                                                        double scale,
                                                        double columnWidth,
                                                        bool rightToLeftTable,
                                                        int columnsCount);

    //! Draw table's cell.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawTableCell(std::shared_ptr<MD::TableCell<MD::QStringTrait>> cell,
                                                         PdfAuxData &pdfData,
                                                         std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                         MD::Table<MD::QStringTrait>::Alignment align,
                                                         double scale);

    //! Draw table border.
    void drawRowBorder(PdfAuxData &pdfData,
                       int startPage,
                       QVector<WhereDrawn> &ret,
                       double offset,
                       double startY,
                       double endY,
                       double columnWidth,
                       int columnsCount);

    //! Draw horizontal line.
    void drawHorizontalLine(PdfAuxData &pdfData);

    //! Handle rendering exception.
    void handleException(PdfAuxData &pdfData, const QString &msg);

private:
    //! Name of the output file.
    QString m_fileName;
    //! Markdown document.
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_doc;
    //! Render options.
    RenderOpts m_opts;
    //! Mutex.
    QMutex m_mutex;
    //! Termination flag.
    bool m_terminate;
    //! All destinations in the document.
    QMap<QString, std::shared_ptr<Destination>> m_dests;
    //! Links that not yet clickable.
    QMultiMap<QString, QVector<QPair<QRectF, unsigned int>>> m_unresolvedLinks;
    //! Footnotes links.
    QMultiMap<QString, QPair<QRectF, unsigned int>> m_unresolvedFootnotesLinks;
    //! Cache of images.
    QMap<QString, QByteArray> m_imageCache;
    //! Footnotes to draw.
    QVector<QPair<QString, std::shared_ptr<MD::Footnote<MD::QStringTrait>>>> m_footnotes;
#ifdef MD_PDF_TESTING
    bool m_isError;
#endif
}; // class Renderer

//
// LoadImageFromNetwork
//

//! Loader of image from network.
class LoadImageFromNetwork final : public QObject
{
    Q_OBJECT

signals:
    void start();

public:
    LoadImageFromNetwork(const QUrl &url, QThread *thread, const ResvgOptions &opts, double height, bool scale);
    ~LoadImageFromNetwork() override = default;

    const QImage &image() const;
    void load();

private slots:
    void loadImpl();
    void loadFinished();
    void loadError(QNetworkReply::NetworkError);

private:
    Q_DISABLE_COPY(LoadImageFromNetwork)

    QThread *m_thread;
    QImage m_img;
    QNetworkReply *m_reply;
    QUrl m_url;
    const ResvgOptions &m_opts;
    double m_height;
    bool m_scale;
}; // class LoadImageFromNetwork

} /* namespace Render */

} /* namespace MdPdf */
