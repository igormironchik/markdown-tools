/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Skia include.
#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkImage.h>
#include <include/core/SkPictureRecorder.h>
#include <modules/skshaper/include/SkShaper.h>
#include <modules/skshaper/include/SkShaper_harfbuzz.h>
#include <modules/skshaper/include/SkShaper_skunicode.h>
#include <modules/skunicode/include/SkUnicode_icu.h>

#ifdef Q_OS_LINUX
#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>
#endif

#include <modules/svg/include/SkSVGDOM.h>

// md4qt include.
#include <md4qt/src/doc.h>

// Qt include.
#include <QByteArray>
#include <QColor>
#include <QImage>
#include <QMutex>
#include <QNetworkReply>
#include <QObject>
#include <QSharedPointer>
#include <QStack>
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

// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
QByteArray qt_inflateSvgzDataFrom(QIODevice *device,
                                  bool doCheckContent);

namespace MdPdf
{

namespace Render
{

//! Footnote scale.
static const double s_footnoteScale = 0.75;
//! Default margin - 20 mm.
static const double s_margin = 72.0 / 25.4 * 20.0;
//! Base offset for blockquotes.
static const double s_blockquoteBaseOffset = 10.0;
//! Width of blockquote mark.
static const double s_blockquoteMarkWidth = 3.0;
//! Margin in table.
static const double s_tableMargin = 2.0;

//! \return A$ size.
SkSize a4Size();

class PdfRenderer;

using Font = SkFont;
using Painter = SkCanvas;
using String = Utf8String;
using Image = SkImage;

//! Rectangle with base point at bottom left corner.
//! Width is directed to the right, height is directed to the top.
struct RectF {
    RectF() = default;
    RectF(qreal leftX,
          qreal bottomY,
          qreal width,
          qreal height);

    qreal x() const;
    qreal bottomY() const;
    qreal width() const;
    qreal height() const;

    void setWidth(qreal w);

    qreal m_leftX = 0.0;
    qreal m_bottomY = 0.0;
    qreal m_width = 0.0;
    qreal m_height = 0.0;
}; // struct RectF

//! Page descriptors.
struct Page {
    std::shared_ptr<SkPictureRecorder> m_recorder;
    Painter *m_canvas = nullptr;
}; // struct Page

//! Image alignment.
enum class ImageAlignment {
    Unknown,
    Left,
    Center,
    Right
}; // enum ImageAlignment

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

#ifdef MD_PDF_TESTING
struct DrawPrimitive {
    enum class Type {
        Text = 0,
        Line,
        Rectangle,
        Image,
        MultilineText,
        Blob,
        Unknown
    };

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
    //! Mark color.
    QColor m_markColor;
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
    QSharedPointer<MdShared::Syntax> m_syntax;
    //! Image alignment.
    ImageAlignment m_imageAlignment;

#ifdef MD_PDF_TESTING
    bool m_printDrawings = false;
    QVector<DrawPrimitive> m_testData;
    QString m_testDataFileName;
#endif // MD_PDF_TESTING
}; // struct RenderOpts

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
                                    double descent);

    static const double s_scale;
    static const double s_baselineScale;

    double nextLineHeight(double lineHeight) const;
    double nextBaselineDelta(bool up) const;
    double currentBaselineDelta() const;
    double nextScale() const;
    double currentScale() const;
    double currentDescent() const;
    double nextDescent(double descent) const;
    bool isMarkColorEnabled() const;
    // pair.first - line height, pair.second - lower part, below descent.
    std::pair<double,
              double>
    fullLineHeight() const;

    //! Stack.
    std::vector<PrevBaselineState> m_stack;
    //! Is mark style applied?
    long long int m_mark = 0;
}; // struct PrevBaselineStateStack

//! Margins.
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

//! Layout direction handler.
struct LayoutDirectionHandler {
    double x() const;
    double y() const;
    void setRightToLeft(bool on);
    bool isRightToLeft() const;
    void setX(double value);
    void addX(double value);
    void moveXToBegin();
    void setY(double value);
    void addY(double value,
              double direction = 1.0);
    double leftBorderXWithOffset() const;
    double rightBorderXWithOffset() const;
    bool isFit(double width) const;
    double topY() const;
    const PageMargins &margins() const;
    PageMargins &margins();
    double pageWidth() const;
    double pageHeight() const;
    double borderStartX() const;
    double xIncrementDirection() const;
    RectF currentRect(double width,
                      double height,
                      double baseline = 0.0) const;
    double startX(double width) const;
    double availableWidth() const;

    struct Offset {
        Offset(std::vector<Offset *> &offsets,
               double value,
               bool left);
        ~Offset();

        Offset(const Offset &) = delete;
        Offset &operator=(const Offset &) = delete;

        double m_value = 0.0;
        bool m_left = true;

    private:
        std::vector<Offset *> &m_offsets;
    };

    Offset addOffset(double value,
                     bool left);

    //! Coordinates and margins.
    CoordsPageAttribs m_coords;

private:
    bool m_isRightToLeft = false;
    std::vector<Offset *> m_offset;
}; // struct LayoutDirectionHandler

/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/**
 * Helper for shaping text directly into a SkTextBlob.
 */
class TextBlobBuilderRunHandler final : public SkShaper::RunHandler
{
public:
    TextBlobBuilderRunHandler(const char *utf8Text,
                              SkPoint offset);

    sk_sp<SkTextBlob> makeBlob();
    SkPoint endPoint();
    void beginLine() override;
    void runInfo(const RunInfo &info) override;
    void commitRunInfo() override;
    Buffer runBuffer(const RunInfo &info) override;
    void commitRunBuffer(const RunInfo &info) override;
    void commitLine() override;
    SkScalar horizontalAdvance() const;

private:
    SkTextBlobBuilder fBuilder;
    char const *const fUtf8Text;
    uint32_t *fClusters;
    int fClusterOffset;
    int fGlyphCount;
    SkScalar fMaxRunAscent;
    SkScalar fMaxRunDescent;
    SkScalar fMaxRunLeading;
    SkPoint fCurrentPosition;
    SkPoint fOffset;
}; // TextBlobBuilderRunHandler

//! Auxiliary struct for rendering.
struct PdfAuxData {
    //! Painters.
    std::vector<Page> *m_pages = nullptr;
    //! Layout direction handler.
    LayoutDirectionHandler m_layout;
    //! Current line height.
    double m_lineHeight = 0.0;
    //! Extra space before footnotes.
    double m_extraInFootnote = 0.0;
    //! Start line of procesing in the document.
    long long int m_startLine = 0;
    //! Start position in the start line.
    long long int m_startPos = 0;
    //! End line of procesing in the document.
    long long int m_endLine = 0;
    //! End position in the end line.
    long long int m_endPos = 0;
    //! Anchors in document.
    QStringList m_anchors;
    //! Reserved spaces on the pages for footnotes.
    QMap<unsigned int, double> m_reserved;
    //! Colors stack.
    QStack<QColor> m_colorsStack;
    //! Markdown document.
    QSharedPointer<MD::Document> m_md;
    //! Current file.
    QString m_currentFile;
    //! Footnotes map to map anchors.
    QMap<MD::Footnote *, QPair<QString, int>> m_footnotesAnchorsMap;
    //! Map of footnotes references to its counter (uses for back links from footnote).
    QMap<MD::Footnote *, int> m_footnoteRefCount;
    //! Special blockquotes that should be highlighted.
    QMap<MD::Blockquote *, QColor> m_highlightedBlockquotes;
    //! Cache of fonts.
    QMap<QString, QSharedPointer<QTemporaryFile>> m_fontsCache;
    //! Stack of painters used on table drawing.
    QMap<int, char> m_cachedPainters;
    //! SkFontMgr.
    sk_sp<SkFontMgr> m_fontMgr;
    //! SkUnicode.
    sk_sp<SkUnicode> m_unicode;
    //! SkShaper.
    std::shared_ptr<SkShaper> m_shaper;
    //! Cache of typesets.
    QHash<QString, sk_sp<SkTypeface>> m_typefaceCache;
    //! Current SkPaint.
    SkPaint m_currentPaint;
    //! Index of the current page.
    int m_currentPageIdx = -1;
    //! Current page index for drawing footnotes.
    int m_footnotePageIdx = -1;
    //! Current painter index.
    int m_currentPainterIdx = -1;
    //! Current index of the footnote (for drawing number in the PDF).
    int m_currentFootnote = 1;
    //! Footnote counter.
    int m_footnoteNum = 1;
    //! Drawing footnotes or the document?
    bool m_drawFootnotes = false;
    //! Is this first item on the page?
    bool m_firstOnPage = true;
    //! Continue drawing of paragraph?
    bool m_continueParagraph = false;
    //! Flag when drawing table.
    bool m_tableDrawing = false;

#ifdef MD_PDF_TESTING
    QMap<QString, QString> m_fonts;
    QSharedPointer<QFile> m_drawingsFile;
    QSharedPointer<QTextStream> m_drawingsStream;
    QVector<DrawPrimitive> m_testData;
    PdfRenderer *m_self = nullptr;
    int m_testPos = 0;
    bool m_printDrawings = false;
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

    //! Draw blob.
    void drawBlob(double x,
                  const sk_sp<SkTextBlob> &blob);
    //! Draw text
    void drawText(double x,
                  double y,
                  const Utf8String &text,
                  const Font &font,
                  double scaleX,
                  bool strikeout);
    //! Draw image.
    void drawImage(double x,
                   double y,
                   const Image *img,
                   double xScale,
                   double yScale);
    //! Draw image.
    void drawImage(double x,
                   double y,
                   const SkSVGDOM *img,
                   double xScale,
                   double yScale);
    //! Draw line.
    void drawLine(double x1,
                  double y1,
                  double x2,
                  double y2);
    //! Save document.
    void save(const QString &fileName);
    //! Draw rectangle.
    void drawRectangle(double x,
                       double y,
                       double width,
                       double height,
                       SkPaint::Style m);

    //! Set color.
    void setColor(const QColor &c);
    //! Restore color.
    void restoreColor();
    //! Repeat color (needed after new page creation).
    void repeatColor();

    //! Shape string.
    //!
    //! \return True on success.
    bool shape(TextBlobBuilderRunHandler &handler,
               const Font &font,
               const String &s,
               bool leftToRight) const;

    //! \return String width.
    double stringWidth(const Font &font,
                       const String &s,
                       bool leftToRight) const;
    //! \return Line spacing.
    double lineSpacing(const Font &font) const;
    //! \return Font ascent.
    double fontAscent(const Font &font) const;
    //! \return Font bounding box scale of total line height.
    double fontBackgroundBoxScale(const Font &font) const;
    //! \return Font descent.
    double fontDescent(const Font &font) const;
}; // struct PdfAuxData;

//! Where was the item drawn?
struct WhereDrawn {
    //! Page painter index.
    int m_pageIdx = -1;
    //! Y of line's bottom.
    double m_y = 0.0;
    //! Height of the item (is directed to the top).
    double m_height = 0.0;
    //! Extra height that can be skipped (usually extra line before new paragraph or heading).
    double m_extraHeight = 0.0;
}; // struct WhereDrawn

//! Flag for RTL languages support.
struct RTLFlag {
    RTLFlag();

    bool isCheck() const;
    bool isRightToLeft() const;

    bool m_isOn = false;
    bool m_check = true;
};

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
    void append(const Width &w);
    //! \return Scale of space at line.
    double scale() const;
    //! \return Height of the line.
    double height() const;
    //! \return Descent of the line.
    double descent() const;
    //! \return Width of the line.
    double width() const;
    //! Move to next line.
    void moveToNextLine();
    //! Is drawing? This struct can be used to precalculate widthes and for actual drawing.
    bool isDrawing() const;
    //! Set drawing.
    void setDrawing(bool on = true);
    //! \return Is last element is new line?
    bool isNewLineAtEnd() const;
    //! \return Begin iterator.
    QVector<double>::ConstIterator cbegin() const;
    //! \return End iterator.
    QVector<double>::ConstIterator cend() const;
    //! \return Height of first item.
    double firstLineHeight() const;
    //! Calculate scales.
    void calcScale(double lineWidth);
    //! \return Paragraph alignment.
    ParagraphAlignment alignment() const;
    //! Set paragraph alignment.
    void setAlignment(ParagraphAlignment alignment);
    //! \return Whether error occured?
    bool isError() const;

private:
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
    //! Alignments of lines.
    QVector<ParagraphAlignment> m_alignment;
    //! Position of current line.
    int m_pos = 0;
    //! Is drawing?
    bool m_drawing = false;
    //! Is error occured?
    mutable bool m_isError = false;
}; // struct CustomWidth

//! Auxiliary struct to automatically init/deinit previous base line calculations.
struct AutoSubSupScriptInit {
    AutoSubSupScriptInit(PdfRenderer *render,
                         MD::ItemWithOpts *item,
                         PrevBaselineStateStack &stack,
                         double lineHeight,
                         double descent);
    ~AutoSubSupScriptInit();

    bool wasAdded() const;

    PdfRenderer *m_render;
    MD::ItemWithOpts *m_item;
    PrevBaselineStateStack &m_stack;
    std::size_t m_count;
};

//
// LoadImageFromNetwork
//

//! Loader of image from network.
class LoadImageFromNetwork final : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void start();

public:
    LoadImageFromNetwork(const QUrl &url,
                         QThread *thread,
                         double height,
                         bool scale);
    ~LoadImageFromNetwork() override = default;

    const QImage &image() const;
    void load();
    bool isSvg() const;
    const QByteArray &svgData() const;

private Q_SLOTS:
    void loadImpl();
    void loadFinished();
    void loadError(QNetworkReply::NetworkError);

private:
    Q_DISABLE_COPY(LoadImageFromNetwork)

    QThread *m_thread;
    QImage m_img;
    QByteArray m_svgData;
    QNetworkReply *m_reply;
    QUrl m_url;
    double m_height;
    bool m_scale;
}; // class LoadImageFromNetwork

} /* namespace Render */

} /* namespace MdPdf */
