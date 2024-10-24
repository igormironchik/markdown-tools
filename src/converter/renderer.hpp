/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-pdf include.
#include "syntax.hpp"

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

#ifdef MD_PDF_TESTING
#include <QFile>
#include <QTextStream>
#endif // MD_PDF_TESTING

// C++ include.
#include <memory>
#include <string_view>

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
enum class ImageAlignment { Left, Center, Right }; // enum ImageAlignment

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
    //! Math font.
    QString m_mathFont;
    //! Math font size.
    int m_mathFontSize;
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
    //! Coordinates and margins.
    CoordsPageAttribs m_coords;
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
    //! DPI.
    quint16 m_dpi;
    //! Markdown document.
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_md;
    //! Syntax highlighter.
    std::shared_ptr<Syntax> m_syntax;
    //! Start line of procesing in the document.
    long long int m_startLine = 0;
    //! Start position in the start line.
    long long int m_startPos = 0;
    //! End line of procesing in the document.
    long long int m_endLine = 0;
    //! End position in the end line.
    long long int m_endPos = 0;
    //! Current file.
    QString m_currentFile;
    //! Footnotes map to map anchors.
    QMap<MD::Footnote<MD::QStringTrait> *, QPair<QString, int>> m_footnotesAnchorsMap;
    //! Resvg options.
    QSharedPointer<ResvgOptions> m_resvgOpts;
    //! Special blockquotes that should be highlighted.
    QMap<MD::Blockquote<MD::QStringTrait> *, QColor> m_highlightedBlockquotes;

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

    //! \return Image width.
    double imageWidth(const QByteArray &image);
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
    int m_pageIdx = -1;
    double m_y = 0.0;
    double m_height = 0.0;
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
                const RenderOpts &opts, bool testing = false) override;
    //! Terminate rendering.
    void terminate();

private slots:
    //! Real rendering.
    void renderImpl();
    //! Clean render.
    void clean() override;

protected:
    friend struct CellItem;
    friend struct CellData;
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
    void moveToNewLine(PdfAuxData &pdfData, double xOffset, double yOffset,
                       double yOffsetMultiplier, double yOffsetOnNewPage);
    //! Load image.
    QByteArray loadImage(MD::Image<MD::QStringTrait> *item, const ResvgOptions &opts,
                         double height = 1.0, bool scale = false, bool cache = true);
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

    //! Draw heading.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawHeading(PdfAuxData &pdfData,
                                                       const RenderOpts &renderOpts,
                                                       MD::Heading<MD::QStringTrait> *item,
                                                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                       double offset,
                                                       double nextItemMinHeight,
                                                       CalcHeightOpt heightCalcOpt,
                                                       double scale,
                                                       bool withNewLine = true);
    //! Draw paragraph.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawParagraph(PdfAuxData &pdfData,
                                                         const RenderOpts &renderOpts,
                                                         MD::Paragraph<MD::QStringTrait> *item,
                                                         std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                         double offset,
                                                         bool withNewLine,
                                                         CalcHeightOpt heightCalcOpt,
                                                         double scale,
                                                         const QColor &color = Qt::black,
                                                         bool scaleImagesToLineHeight = false);
    //! Draw block of code.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawCode(PdfAuxData &pdfData,
                                                    const RenderOpts &renderOpts,
                                                    MD::Code<MD::QStringTrait> *item,
                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                    double offset,
                                                    CalcHeightOpt heightCalcOpt,
                                                    double scale);
    //! Draw blockquote.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawBlockquote(PdfAuxData &pdfData,
                                                          const RenderOpts &renderOpts,
                                                          MD::Blockquote<MD::QStringTrait> *item,
                                                          std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                          double offset,
                                                          CalcHeightOpt heightCalcOpt,
                                                          double scale);
    //! Draw list.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawList(PdfAuxData &pdfData,
                                                    const RenderOpts &renderOpts,
                                                    MD::List<MD::QStringTrait> *item,
                                                    std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                    int bulletWidth,
                                                    double offset = 0.0,
                                                    CalcHeightOpt heightCalcOpt = CalcHeightOpt::Unknown,
                                                    double scale = 1.0,
                                                    bool nested = false);
    //! Draw table.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawTable(PdfAuxData &pdfData,
                                                     const RenderOpts &renderOpts,
                                                     MD::Table<MD::QStringTrait> *item,
                                                     std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                     double offset,
                                                     CalcHeightOpt heightCalcOpt,
                                                     double scale);

    //! \return Minimum necessary height to draw item, meant at least one line.
    double minNecessaryHeight(PdfAuxData &pdfData,
                              const RenderOpts &renderOpts,
                              std::shared_ptr<MD::Item<MD::QStringTrait>> item,
                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                              double offset,
                              double scale);
    //! \return Height of the footnote.
    QVector<WhereDrawn> drawFootnote(PdfAuxData &pdfData,
                                     const RenderOpts &renderOpts,
                                     std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                     const QString &footnoteRefId,
                                     MD::Footnote<MD::QStringTrait> *note,
                                     CalcHeightOpt heightCalcOpt,
                                     double *lineHeight = nullptr);
    //! \return Height of the footnote.
    QVector<WhereDrawn> footnoteHeight(PdfAuxData &pdfData,
                                       const RenderOpts &renderOpts,
                                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                       MD::Footnote<MD::QStringTrait> *note,
                                       double *lineHeight);
    //! Reserve space for footnote.
    void reserveSpaceForFootnote(PdfAuxData &pdfData,
                                 const RenderOpts &renderOpts,
                                 const QVector<WhereDrawn> &h,
                                 const double &currentY,
                                 int currentPage,
                                 double lineHeight,
                                 bool addExtraLine = false);
    //! Add footnote.
    void addFootnote(const QString &refId,
                     std::shared_ptr<MD::Footnote<MD::QStringTrait>> f,
                     PdfAuxData &pdfData,
                     const RenderOpts &renderOpts,
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
                                                        const RenderOpts &renderOpts,
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
                                                        bool firstItem);

    //! Auxiliary struct for calculation of spaces scales to shrink text to width.
    struct CustomWidth {
        //! Item on line.
        struct Width {
            double m_width = 0.0;
            double m_height = 0.0;
            bool m_isSpace = false;
            bool m_isNewLine = false;
            bool m_shrink = true;
            QString m_word;
        }; // struct Width

        //! Append new item.
        void append(const Width &w)
        {
            m_width.append(w);
        }
        //! \return Scale of space at line.
        double scale()
        {
            return m_scale.at(m_pos);
        }
        //! \return Height of the line.
        double height()
        {
            return m_height.at(m_pos);
        }
        //! Move to next line.
        void moveToNextLine()
        {
            ++m_pos;
        }
        //! Is drawing? This struct can be used to precalculate widthes and for actual drawing.
        bool isDrawing() const
        {
            return m_drawing;
        }
        //! Set drawing.
        void setDrawing(bool on = true)
        {
            m_drawing = on;
        }
        //! \return Is last element is new line?
        bool isNewLineAtEnd() const
        {
            return (m_width.isEmpty() ? false : m_width.back().m_isNewLine);
        }

        //! \return Begin iterator.
        QVector<Width>::ConstIterator cbegin() const
        {
            return m_width.cbegin();
        }
        //! \return End iterator.
        QVector<Width>::ConstIterator cend() const
        {
            return m_width.cend();
        }

        //! \return Height of first item.
        double firstItemHeight() const;
        //! Calculate scales.
        void calcScale(double lineWidth);

    private:
        //! Is drawing?
        bool m_drawing = false;
        //! Sizes of items.
        QVector<Width> m_width;
        //! Scales on lines.
        QVector<double> m_scale;
        //! Heights of lines.
        QVector<double> m_height;
        //! Position of current line.
        int m_pos = 0;
    }; // struct CustomWidth

    //! Draw text.
    QVector<QPair<QRectF, unsigned int>> drawText(PdfAuxData &pdfData,
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
                                                  const QColor &color = Qt::black);
    //! Draw inlined code.
    QVector<QPair<QRectF, unsigned int>> drawInlinedCode(PdfAuxData &pdfData,
                                                         const RenderOpts &renderOpts,
                                                         MD::Code<MD::QStringTrait> *item,
                                                         std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                         bool &newLine,
                                                         double offset,
                                                         bool firstInParagraph,
                                                         CustomWidth &cw,
                                                         double scale,
                                                         const QColor &color = Qt::black);
    //! Draw string.
    QVector<QPair<QRectF, unsigned int>> drawString(PdfAuxData &pdfData,
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
                                                    const QColor &color = Qt::black,
                                                    Font *regularSpaceFont = nullptr,
                                                    double regularSpaceFontSize = 0.0,
                                                    double regularSpaceFontScale = 0.0);
    //! Draw link.
    QVector<QPair<QRectF, unsigned int>> drawLink(PdfAuxData &pdfData,
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
                                                  double scale);
    //! Draw image.
    QPair<QRectF, unsigned int> drawImage(PdfAuxData &pdfData,
                                          const RenderOpts &renderOpts,
                                          MD::Image<MD::QStringTrait> *item,
                                          std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                          bool &newLine,
                                          double offset,
                                          bool firstInParagraph,
                                          CustomWidth &cw,
                                          double scale,
                                          ImageAlignment alignment,
                                          bool scaleImagesToLineHeight = false);
    //! Draw math expression.
    QPair<QRectF, unsigned int> drawMathExpr(PdfAuxData &pdfData,
                                             const RenderOpts &renderOpts,
                                             MD::Math<MD::QStringTrait> *item,
                                             std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                             bool &newLine,
                                             double offset,
                                             bool hasNext,
                                             bool firstInParagraph,
                                             CustomWidth &cw,
                                             double scale);

    //! Font in table.
    struct FontAttribs {
        QString m_family;
        bool m_bold;
        bool m_italic;
        bool m_strikethrough;
        int m_size;
    }; // struct FontAttribs

    friend bool operator!=(const PdfRenderer::FontAttribs &f1, const PdfRenderer::FontAttribs &f2);
    friend bool operator==(const PdfRenderer::FontAttribs &f1, const PdfRenderer::FontAttribs &f2);

    //! Item in the table's cell.
    struct CellItem {
        QString m_word;
        QByteArray m_image;
        QString m_url;
        QString m_footnote;
        QString m_footnoteRef;
        QColor m_color;
        QColor m_background;
        std::shared_ptr<MD::Footnote<MD::QStringTrait>> m_footnoteObj;
        FontAttribs m_font;

        //! \return Width of the item.
        double width(PdfAuxData &pdfData, PdfRenderer *render, double scale) const;
    }; // struct CellItem

    //! Cell in the table.
    struct CellData {
        double m_width = 0.0;
        double m_height = 0.0;
        MD::Table<MD::QStringTrait>::Alignment m_alignment;
        QVector<CellItem> m_items;

        void setWidth(double w)
        {
            m_width = w;
        }
        //! Calculate height for the given width.
        void heightToWidth(double lineHeight, double spaceWidth, double scale,
                           PdfAuxData &pdfData, PdfRenderer *render);
    }; //  struct CellData

    //! \return Height of the row.
    double rowHeight(const QVector<QVector<CellData>> &table, int row);
    //! Create auxiliary cell.
    void createAuxCell(const RenderOpts &renderOpts,
                       PdfAuxData &pdfData,
                       CellData &data,
                       MD::Item<MD::QStringTrait> *item,
                       std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                       const QString &url = {},
                       const QColor &color = {});
    //! Create auxiliary table for drawing.
    QVector<QVector<CellData>> createAuxTable(PdfAuxData &pdfData,
                                              const RenderOpts &renderOpts,
                                              MD::Table<MD::QStringTrait> *item,
                                              std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                              double scale);
    //! Calculate size of the cells in the table.
    void calculateCellsSize(PdfAuxData &pdfData, QVector<QVector<CellData>> &auxTable,
                            double spaceWidth, double offset, double lineHeight, double scale);
    //! Draw table's row.
    QPair<QVector<WhereDrawn>, WhereDrawn> drawTableRow(QVector<QVector<CellData>> &table,
                                                        int row,
                                                        PdfAuxData &pdfData,
                                                        double offset,
                                                        double lineHeight,
                                                        const RenderOpts &renderOpts,
                                                        std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                                                        QVector<QPair<QString, std::shared_ptr<MD::Footnote<MD::QStringTrait>>>> &footnotes,
                                                        double scale);
    //! Draw table border.
    void drawRowBorder(PdfAuxData &pdfData,
                       int startPage,
                       QVector<WhereDrawn> &ret,
                       const RenderOpts &renderOpts,
                       double offset,
                       const QVector<QVector<CellData>> &table,
                       double startY,
                       double endY);

    // Holder of single line in table.
    struct TextToDraw {
        double m_width = 0.0;
        double m_availableWidth = 0.0;
        double m_lineHeight = 0.0;
        MD::Table<MD::QStringTrait>::Alignment m_alignment;
        QVector<CellItem> m_text;

        void clear()
        {
            m_width = 0.0;
            m_text.clear();
        }
    }; // struct TextToDraw

    //! Draw text line in the cell.
    void drawTextLineInTable(const RenderOpts &renderOpts,
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
                             double scale);
    //! Create new page in table.
    void newPageInTable(PdfAuxData &pdfData, int &currentPage, int &endPage, double &endY);
    //! Make links in table clickable.
    void processLinksInTable(PdfAuxData &pdfData,
                             const QMap<QString, QVector<QPair<QRectF, unsigned int>>> &links,
                             std::shared_ptr<MD::Document<MD::QStringTrait>> doc);

    //! Draw horizontal line.
    void drawHorizontalLine(PdfAuxData &pdfData, const RenderOpts &renderOpts);

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
    //! Footnote counter.
    int m_footnoteNum;
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
