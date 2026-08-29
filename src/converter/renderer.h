/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// shared include.
#include "emoji_parser.h"
#include "syntax.h"
#include "utils.h"

// converter include.
#include "renderer_aux.h"

// Qt include.
#include <QObject>

namespace MdPdf
{

namespace Render
{

//
// Renderer
//

//! Abstract renderer.
class Renderer : public QObject
{
    Q_OBJECT

Q_SIGNALS:
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

    virtual void render(const QString &fileName,
                        QSharedPointer<MD::Document> doc,
                        const RenderOpts &opts) = 0;
}; // class Renderer

//
// PdfRenderer
//

//! Renderer to PDF.
class PdfRenderer : public Renderer
{
    Q_OBJECT

Q_SIGNALS:
    //! Internal signal for start rendering.
    void start();

public:
    PdfRenderer();
    ~PdfRenderer() override = default;

public Q_SLOTS:
    //! Render document.
    //! Renderer will delete himself on job finish.
    void render(const QString &fileName,
                QSharedPointer<MD::Document> doc,
                const MdPdf::Render::RenderOpts &opts) override;
    //! Terminate rendering.
    void terminate();

private Q_SLOTS:
    //! Real rendering.
    void renderImpl();

private:
    friend class AutoSubSupScriptInit;

#ifdef MD_PDF_TESTING
    friend struct TestRendering;
    bool isError() const;
#endif

    //! Create font.
    Font createFont(const QString &name,
                    bool bold,
                    bool italic,
                    double size,
                    PdfAuxData &pdfData);

    //! Create new page.
    void createPage(PdfAuxData &pdfData);

    //! Draw empty line.
    void moveToNewLine(
        //! Auxiliary PDF data.
        PdfAuxData &pdfData,
        //! Offset for Y coordinate.
        double yOffset,
        //! Multiplier for Y coordinate. Real offset will be yOffset * yOffsetMultiplier.
        double yOffsetMultiplier,
        //! Y offset on new page.
        double yOffsetOnNewPage);
    //! Load image.
    QByteArray loadImage(
        //! Image.
        MD::Image *item,
        //! Height to scale image to.
        double height = 1.0,
        //! Should image be scaled?
        bool scale = false,
        //! Store in cache loaded image data?
        bool cache = true,
        //! Is this an SVG?
        bool *isSvg = nullptr);
    //! Max width of numbered list bullet.
    int maxListNumberWidth(MD::List *list) const;

    //! What calculation of height to do?
    enum class CalcHeightOpt {
        //! Don't calculate, do drawing.
        Unknown = 0,
        //! Calculate minimum requred height (at least one line).
        Minimum = 1,
        //! Calculate full height.
        Full = 2
    }; // enum class CalcHeightOpt

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
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawHeading(PdfAuxData &pdfData,
                MD::Heading *item,
                QSharedPointer<MD::Document> doc,
                double offset,
                double nextItemMinHeight,
                CalcHeightOpt heightCalcOpt,
                double scale,
                bool withNewLine = true,
                RTLFlag *rtl = nullptr);
    //! Draw paragraph.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawParagraph(PdfAuxData &pdfData,
                  MD::Paragraph *item,
                  QSharedPointer<MD::Document> doc,
                  double offset,
                  bool withNewLine,
                  CalcHeightOpt heightCalcOpt,
                  double scale,
                  const QColor &color = Qt::black,
                  bool scaleImagesToLineHeight = false,
                  RTLFlag *rtl = nullptr,
                  ParagraphAlignment align = ParagraphAlignment::FillWidth);
    //! Draw block of code.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawCode(PdfAuxData &pdfData,
             MD::Code *item,
             QSharedPointer<MD::Document> doc,
             double offset,
             CalcHeightOpt heightCalcOpt,
             double scale);
    //! Draw blockquote.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawBlockquote(PdfAuxData &pdfData,
                   MD::Blockquote *item,
                   QSharedPointer<MD::Document> doc,
                   double offset,
                   CalcHeightOpt heightCalcOpt,
                   double scale,
                   RTLFlag *rtl = nullptr);
    //! Draw list.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawList(PdfAuxData &pdfData,
             MD::List *item,
             QSharedPointer<MD::Document> doc,
             int bulletWidth,
             double offset = 0.0,
             CalcHeightOpt heightCalcOpt = CalcHeightOpt::Unknown,
             double scale = 1.0,
             bool nested = false,
             RTLFlag *rtl = nullptr);
    //! Draw table.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawTable(PdfAuxData &pdfData,
              MD::Table *item,
              QSharedPointer<MD::Document> doc,
              double offset,
              CalcHeightOpt heightCalcOpt,
              double scale);

    //! \return Minimum necessary height to draw item, meant at least one line.
    double minNecessaryHeight(PdfAuxData &pdfData,
                              QSharedPointer<MD::Item> item,
                              QSharedPointer<MD::Document> doc,
                              double offset,
                              double scale);
    //! \return Height of the footnote.
    QVector<WhereDrawn> drawFootnote(PdfAuxData &pdfData,
                                     QSharedPointer<MD::Document> doc,
                                     const QString &footnoteRefId,
                                     MD::Footnote *note,
                                     CalcHeightOpt heightCalcOpt,
                                     double *lineHeight = nullptr,
                                     RTLFlag *rtl = nullptr);
    //! \return Height of the footnote.
    QVector<WhereDrawn> footnoteHeight(PdfAuxData &pdfData,
                                       QSharedPointer<MD::Document> doc,
                                       MD::Footnote *note,
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
                     QSharedPointer<MD::Footnote> f,
                     PdfAuxData &pdfData,
                     QSharedPointer<MD::Document> doc);

    //! List item type.
    enum class ListItemType {
        Unknown,
        //! Ordered.
        Ordered,
        //! Unordered.
        Unordered
    }; // enum class ListItemType

    //! Draw list item.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawListItem(PdfAuxData &pdfData,
                 MD::ListItem *item,
                 QSharedPointer<MD::Document> doc,
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

    //! Align line.
    void alignLine(PdfAuxData &pdfData,
                   const CustomWidth &cw);

    //! Initialize baseline with the given item.
    void initSubSupScript(MD::ItemWithOpts *item,
                          PrevBaselineStateStack &state,
                          double lineHeight,
                          double descent);

    //! Deinit baseline with the given item.
    void deinitSubSupScript(MD::ItemWithOpts *item,
                            PrevBaselineStateStack &state);

    //! Draw text.
    QVector<QPair<RectF,
                  unsigned int>>
    drawText(PdfAuxData &pdfData,
             MD::Text *item,
             QSharedPointer<MD::Document> doc,
             bool &newLine,
             const Font *footnoteFont,
             MD::Item *nextItem,
             int footnoteNum,
             double offset,
             bool firstInParagraph,
             CustomWidth &cw,
             double scale,
             PrevBaselineStateStack &previousBaseline,
             const QColor &color = Qt::black,
             RTLFlag *rtl = nullptr);
    //! Draw emoji.
    QVector<QPair<RectF,
                  unsigned int>>
    drawEmoji(PdfAuxData &pdfData,
              MdShared::EmojiItem *item,
              QSharedPointer<MD::Document> doc,
              bool &newLine,
              const Font *footnoteFont,
              MD::Item *nextItem,
              int footnoteNum,
              double offset,
              bool firstInParagraph,
              CustomWidth &cw,
              double scale,
              PrevBaselineStateStack &previousBaseline,
              const QColor &color = Qt::black,
              RTLFlag *rtl = nullptr);
    //! Draw inlined code.
    QVector<QPair<RectF,
                  unsigned int>>
    drawInlinedCode(PdfAuxData &pdfData,
                    MD::Code *item,
                    QSharedPointer<MD::Document> doc,
                    bool &newLine,
                    double offset,
                    bool firstInParagraph,
                    CustomWidth &cw,
                    double scale,
                    PrevBaselineStateStack &previousBaseline,
                    RTLFlag *rtl = nullptr,
                    bool inLink = false);

    bool nextOnOneLineIsFit(PdfAuxData &pdfData,
                            const QVector<Word> &words,
                            qsizetype idx,
                            qsizetype last,
                            double width);

    //! Draw a word.
    qsizetype drawWord(PdfAuxData &pdfData,
                       const QVector<Word> &words,
                       qsizetype idx,
                       qsizetype last,
                       bool &newLine,
                       bool draw,
                       const QColor &background,
                       CustomWidth &cw,
                       const PrevBaselineStateStack &currentBaseline,
                       double lineHeight,
                       bool footnoteAtEnd,
                       double footnoteWidth,
                       QVector<QPair<RectF,
                                     unsigned int>> &ret,
                       bool strikeout,
                       double fullWidth,
                       double offset,
                       double &h,
                       const QColor &color,
                       int footnoteNum,
                       bool *wasMovedToNewLine = nullptr);

    void drawSpace(PdfAuxData &pdfData,
                   bool useRegularSpace,
                   bool &firstSpaceDrawn,
                   const Font &spaceFont,
                   const Font &font,
                   CustomWidth &cw,
                   bool draw,
                   const Font *regularSpaceFont,
                   double spaceWidth,
                   bool &newLine,
                   QVector<QPair<RectF,
                                 unsigned int>> &ret,
                   double lineHeight,
                   PrevBaselineStateStack &currentBaseline,
                   const QColor &color,
                   const QColor &background,
                   bool strikeout);

    //! Draw string.
    QVector<QPair<RectF,
                  unsigned int>>
    drawString(PdfAuxData &pdfData,
               const QString &str,
               const Font &spaceFont,
               const Font &font,
               double lineHeight,
               QSharedPointer<MD::Document> doc,
               bool &newLine,
               const Font *footnoteFont,
               MD::Item *nextItem,
               int footnoteNum,
               double offset,
               bool firstInParagraph,
               CustomWidth &cw,
               QColor background,
               bool strikeout,
               long long int startLine,
               long long int startPos,
               long long int endLine,
               long long int endPos,
               PrevBaselineStateStack &currentBaseline,
               const QColor &color = Qt::black,
               const Font *regularSpaceFont = nullptr,
               RTLFlag *rtl = nullptr);
    //! Draw blob or simple text.
    void drawTextBlobOrText(PdfAuxData &pdfData,
                            const Font &font,
                            const Utf8String &str,
                            double descent,
                            double baselineDelta,
                            bool rtl,
                            double length,
                            bool strikeout);
    //! Draw link.
    QVector<QPair<RectF,
                  unsigned int>>
    drawLink(PdfAuxData &pdfData,
             MD::Link *item,
             QSharedPointer<MD::Document> doc,
             bool &newLine,
             const Font &footnoteFont,
             MD::Item *prevItem,
             MD::Item *nextItem,
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
             PrevBaselineStateStack &previousBaseline,
             RTLFlag *rtl = nullptr);
    //! \return Is \par it a space?
    template<class Iterator>
    inline bool isSpace(Iterator it)
    {
        if ((*it)->type() == MD::ItemType::Text) {
            auto t = static_cast<MD::Text *>(it->get());

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
    inline bool isNothingAfter(MD::Block::Items::const_iterator it,
                               MD::Block::Items::const_iterator last);
    //! Skip backward til \par func returns true.
    template<class Iterator,
             class Func>
    inline Iterator skipBackwardWithFunc(Iterator it,
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
    MD::Item *getPrevItem(MD::Block::Items::const_iterator it,
                          MD::Block::Items::const_iterator begin,
                          MD::Block::Items::const_iterator last);
    //! Skip raw HTML and spaces backward.
    inline MD::Block::Items::const_iterator skipRawHtmlAndSpacesBackward(MD::Block::Items::const_iterator it,
                                                                         MD::Block::Items::const_iterator begin,
                                                                         MD::Block::Items::const_iterator last);
    //! Skip raw HTML and spaces.
    template<class Iterator>
    inline Iterator skipRawHtmlAndSpaces(Iterator it,
                                         Iterator last)
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
                                          MD::Item *item,
                                          double offset,
                                          double lineHeight,
                                          bool scaleImagesToLineHeight);
    //! \return Is after \par it a text item or online content?
    bool isTextOrOnlineAfter(MD::Block::Items::const_iterator it,
                             MD::Block::Items::const_iterator last,
                             PdfAuxData &pdfData,
                             double offset,
                             double lineHeight,
                             bool scaleImagesToLineHeight);
    //! \return Is before \par it a text item or online content?
    bool isTextOrOnlineBefore(MD::Block::Items::const_iterator it,
                              MD::Block::Items::const_iterator begin,
                              MD::Block::Items::const_iterator last,
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
            auto m = static_cast<MD::Math *>(it->get());

            return m->isInline();
        } break;

        case MD::ItemType::Image: {
            return isOnlineImage(pdfData,
                                 static_cast<MD::Image *>(it->get()),
                                 offset,
                                 lineHeight,
                                 scaleImagesToLineHeight);
        } break;

        case MD::ItemType::Link: {
            auto l = static_cast<MD::Link *>(it->get());

            if (!l->p()->isEmpty()) {
                if (reverse) {
                    return isTextOrOnline(l->p()->items().crbegin(),
                                          l->p()->items().crend(),
                                          reverse,
                                          pdfData,
                                          offset,
                                          lineHeight,
                                          scaleImagesToLineHeight);
                } else {
                    return isTextOrOnline(l->p()->items().cbegin(),
                                          l->p()->items().cend(),
                                          reverse,
                                          pdfData,
                                          offset,
                                          lineHeight,
                                          scaleImagesToLineHeight);
                }
            } else if (l->img()->isEmpty()) {
                return true;
            } else {
                return isOnlineImage(pdfData, l->img().get(), offset, lineHeight, scaleImagesToLineHeight);
            }
        } break;

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
                       MD::Image *item,
                       double offset,
                       double lineHeight,
                       bool scaleImagesToLineHeight);
    //! Draw image.
    QPair<RectF,
          unsigned int>
    drawImage(PdfAuxData &pdfData,
              MD::Image *item,
              QSharedPointer<MD::Document> doc,
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
              PrevBaselineStateStack &previousBaseline,
              MD::Item *prevItem,
              ImageAlignment alignment = ImageAlignment::Unknown,
              bool scaleImagesToLineHeight = false);

    //! Draw math expression.
    QPair<RectF,
          unsigned int>
    drawMathExpr(PdfAuxData &pdfData,
                 MD::Math *item,
                 QSharedPointer<MD::Document> doc,
                 MD::Item *prevItem,
                 bool &newLine,
                 double offset,
                 bool isNextText,
                 bool firstInParagraph,
                 CustomWidth &cw,
                 double scale,
                 PrevBaselineStateStack &previousBaseline);

    //! \return Height of the table's row.
    double rowHeight(PdfAuxData &pdfData,
                     QSharedPointer<MD::TableRow> row,
                     double width,
                     QSharedPointer<MD::Document> doc,
                     double scale);

    //! Draw table's row.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawTableRow(QSharedPointer<MD::TableRow> row,
                 PdfAuxData &pdfData,
                 QSharedPointer<MD::Document> doc,
                 MD::Table *table,
                 double offset,
                 double scale,
                 double columnWidth,
                 bool rightToLeftTable,
                 int columnsCount);

    //! Draw table's cell.
    QPair<QVector<WhereDrawn>,
          WhereDrawn>
    drawTableCell(QSharedPointer<MD::TableCell> cell,
                  PdfAuxData &pdfData,
                  QSharedPointer<MD::Document> doc,
                  MD::Table::Alignment align,
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
    void handleException(PdfAuxData &pdfData,
                         const QString &msg);

private:
    //! Name of the output file.
    QString m_fileName;
    //! Markdown document.
    QSharedPointer<MD::Document> m_doc;
    //! Render options.
    RenderOpts m_opts;
    //! Mutex.
    QMutex m_mutex;
    //! Termination flag.
    bool m_terminate;
    //! Cache of images.
    QMap<QString, QPair<QByteArray, bool>> m_imageCache;
    //! Footnotes to draw.
    QVector<QPair<QString, QSharedPointer<MD::Footnote>>> m_footnotes;
#ifdef MD_PDF_TESTING
    bool m_isError;
#endif
}; // class Renderer

} /* namespace Render */

} /* namespace MdPdf */
