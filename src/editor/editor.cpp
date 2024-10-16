/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "editor.hpp"
#include "syntaxvisitor.hpp"

// Qt include.
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>
#include <QThread>
#include <QTextLayout>

// C++ include.
#include <functional>
#include <limits>
#include <utility>

namespace MdEditor
{

bool operator!=(const Margins &l, const Margins &r)
{
    return (l.m_enable != r.m_enable || l.m_length != r.m_length);
}

//
// DataParser
//

class DataParser : public QObject
{
    Q_OBJECT

signals:
    void newData();
    void done(std::shared_ptr<MD::Document<MD::QStringTrait>>, unsigned long long int);

public:
    DataParser()
    {
        connect(this, &DataParser::newData, this, &DataParser::onParse, Qt::QueuedConnection);
    }

    ~DataParser() override = default;

public slots:
    void onData(const QString &md, const QString &path, const QString &fileName, unsigned long long int counter)
    {
        m_data.clear();
        m_data.push_back(md);
        m_path = path;
        m_fileName = fileName;
        m_counter = counter;

        emit newData();
    }

private slots:
    void onParse()
    {
        if (!m_data.isEmpty()) {
            QTextStream stream(&m_data.back());

            const auto doc = m_parser.parse(stream, m_path, m_fileName);

            m_data.clear();

            emit done(doc, m_counter);
        }
    }

private:
    QStringList m_data;
    QString m_path;
    QString m_fileName;
    unsigned long long int m_counter;
    MD::Parser<MD::QStringTrait> m_parser;
};

//
// DocumentLayoutWithRightAlignment
//

class DocumentLayoutWithRightAlignment : public QPlainTextDocumentLayout
{
public:
    DocumentLayoutWithRightAlignment(QTextDocument *doc, QWidget *viewport)
        : QPlainTextDocumentLayout(doc)
        , m_viewport(viewport)
    {
    }
    ~DocumentLayoutWithRightAlignment() override = default;

    QRectF blockBoundingRect(const QTextBlock &block) const override
    {
        const auto r = QPlainTextDocumentLayout::blockBoundingRect(block);

        if (block.isValid()) {
            bool alignRight = false;

            switch (block.textDirection()) {
            case Qt::RightToLeft:
                alignRight = true;
                break;

            case Qt::LayoutDirectionAuto:
                alignRight = block.text().isRightToLeft();
                break;

            default:
                break;
            }

            if (alignRight) {
                auto tl = block.layout();
                auto option = document()->defaultTextOption();
                tl->setTextOption(option);
                auto margin = document()->documentMargin();
                int extraMargin = 0;
                if (option.flags() & QTextOption::AddSpaceForLineAndParagraphSeparators) {
                    QFontMetrics fm(block.charFormat().font());
                    extraMargin += fm.horizontalAdvance(QChar(0x21B5));
                }

                qreal availableWidth = m_viewport->width();
                if (availableWidth <= 0) {
                    availableWidth = qreal(INT_MAX);
                }
                availableWidth -= 2*margin + extraMargin;

                QFontMetrics fm(block.charFormat().font());

                for (int i = 0; i < tl->lineCount(); ++i) {
                    auto line = tl->lineAt(i);
                    line.setPosition({margin + availableWidth - line.naturalTextWidth(), line.position().y()});
                }
            }
        }

        return r;
    }

private:
    QWidget *m_viewport;
};

//
// EditorPrivate
//

struct EditorPrivate {
    explicit EditorPrivate(Editor *parent)
        : m_q(parent)
        , m_syntax(parent)
        , m_parsingThread(new QThread(m_q))
        , m_parser(new DataParser)
    {
        m_parser->moveToThread(m_parsingThread);

        QObject::connect(m_q, &Editor::doParsing, m_parser, &DataParser::onData, Qt::QueuedConnection);
        QObject::connect(m_parser, &DataParser::done, m_q, &Editor::onParsingDone, Qt::QueuedConnection);
    }

    ~EditorPrivate()
    {
        m_parsingThread->quit();
        m_parsingThread->wait();
    }

    void initUi()
    {
        m_lineNumberArea = new LineNumberArea(m_q);

        QObject::connect(m_q, &Editor::cursorPositionChanged, m_q, &Editor::highlightCurrentLine);
        QObject::connect(m_q, &QPlainTextEdit::textChanged, m_q, &Editor::onContentChanged);

        m_q->showLineNumbers(true);
        m_q->applyFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        m_q->updateLineNumberAreaWidth(0);
        m_q->highlightCurrentLine();
        m_q->showUnprintableCharacters(true);
        m_q->setFocusPolicy(Qt::ClickFocus);
        m_q->setCenterOnScroll(true);

        m_q->document()->setDocumentLayout(new DocumentLayoutWithRightAlignment(m_q->document(), m_q->viewport()));

        m_parsingThread->start();
    }

    void setExtraSelections()
    {
        QList<QTextEdit::ExtraSelection> tmp = m_syntaxHighlighting;
        tmp << m_extraSelections;
        tmp.prepend(m_currentLine);

        m_q->setExtraSelections(tmp);
    }

    Editor *m_q = nullptr;
    LineNumberArea *m_lineNumberArea = nullptr;
    QString m_docName;
    bool m_showLineNumberArea = true;
    QList<QTextEdit::ExtraSelection> m_extraSelections;
    QList<QTextEdit::ExtraSelection> m_syntaxHighlighting;
    QTextEdit::ExtraSelection m_currentLine;
    QString m_highlightedText;
    Colors m_colors;
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_currentDoc;
    SyntaxVisitor m_syntax;
    Margins m_margins;
    QThread *m_parsingThread = nullptr;
    DataParser *m_parser = nullptr;
    unsigned long long int m_currentParsingCounter = 0;
}; // struct EditorPrivate

//
// Editor
//

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_d(new EditorPrivate(this))
{
    m_d->initUi();
}

Editor::~Editor()
{
}

SyntaxVisitor &Editor::syntaxHighlighter() const
{
    return m_d->m_syntax;
}

Margins &Editor::margins()
{
    return m_d->m_margins;
}

bool Editor::foundHighlighted() const
{
    return !m_d->m_extraSelections.isEmpty();
}

bool Editor::foundSelected() const
{
    const auto c = textCursor();

    for (const auto &s : std::as_const(m_d->m_extraSelections)) {
        if (c.position() == s.cursor.position()) {
            if (c.hasSelection() && c.selectionStart() == s.cursor.selectionStart()
                && c.selectionEnd() == s.cursor.selectionEnd()) {
                return true;
            } else {
                return false;
            }
        }
    }

    return false;
}

void Editor::applyColors(const Colors &colors)
{
    m_d->m_colors = colors;

    if (m_d->m_colors.m_enabled) {
        onContentChanged();
    } else {
        m_d->m_syntax.clearHighlighting();
    }

    viewport()->update();
}

std::shared_ptr<MD::Document<MD::QStringTrait>> Editor::currentDoc() const
{
    return m_d->m_currentDoc;
}

void Editor::applyFont(const QFont &f)
{
    setFont(f);

    m_d->m_syntax.setFont(f);

    highlightSyntax(m_d->m_colors, m_d->m_currentDoc);
}

void Editor::setDocName(const QString &name)
{
    m_d->m_docName = name;
}

const QString &Editor::docName() const
{
    return m_d->m_docName;
}

int Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());

    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    if (digits < 2) {
        digits = 2;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void Editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    if (m_d->m_showLineNumberArea) {
        if (layoutDirection() == Qt::LeftToRight) {
            setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
        } else {
            setViewportMargins(0, 0, lineNumberAreaWidth(), 0);
        }
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

void Editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        m_d->m_lineNumberArea->scroll(0, dy);
    } else {
        m_d->m_lineNumberArea->update(0, rect.y(), m_d->m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void Editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();

    if (layoutDirection() == Qt::LeftToRight) {
        m_d->m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    } else {
        m_d->m_lineNumberArea->setGeometry(QRect(cr.left() + cr.width() - lineNumberAreaWidth(), cr.top(),
                                                 lineNumberAreaWidth(), cr.height()));
    }
}

void Editor::paintEvent(QPaintEvent *event)
{
    if (m_d->m_margins.m_enable) {
        QPainter painter(viewport());
        QRect r = viewport()->rect();
        QFontMetricsF fm(font());

        r.setX(document()->documentMargin() + qRound(fm.averageCharWidth() * m_d->m_margins.m_length));

        painter.setBrush(QColor(239, 239, 239));
        painter.setPen(Qt::NoPen);
        painter.drawRect(r);
    }

    QPlainTextEdit::paintEvent(event);
}

void Editor::highlightCurrentLine()
{
    static const QColor lineColor = QColor(255, 255, 0, 75);

    m_d->m_currentLine.format.setBackground(lineColor);
    m_d->m_currentLine.format.setProperty(QTextFormat::FullWidthSelection, true);
    m_d->m_currentLine.cursor = textCursor();
    m_d->m_currentLine.cursor.clearSelection();

    m_d->setExtraSelections();
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_d->m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_d->m_lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int Editor::lineNumber(const QPoint &p)
{
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= p.y()) {
        if (block.isVisible() && bottom >= p.y()) {
            return blockNumber;
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    return -1;
}

void LineNumberArea::enterEvent(QEnterEvent *event)
{
    onHover(event->position().toPoint());

    event->ignore();
}

void LineNumberArea::mouseMoveEvent(QMouseEvent *event)
{
    onHover(event->position().toPoint());

    event->ignore();
}

void LineNumberArea::leaveEvent(QEvent *event)
{
    m_lineNumber = -1;

    emit hoverLeaved();

    event->ignore();
}

void LineNumberArea::onHover(const QPoint &p)
{
    const auto ln = m_codeEditor->lineNumber(p);

    if (ln != m_lineNumber) {
        m_lineNumber = ln;

        emit lineHovered(m_lineNumber, mapToGlobal(QPoint(width(), p.y())));
    }
}

void Editor::showUnprintableCharacters(bool on)
{
    if (on) {
        QTextOption opt;
        opt.setFlags(QTextOption::ShowTabsAndSpaces);

        document()->setDefaultTextOption(opt);
    } else {
        document()->setDefaultTextOption({});
    }

    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);
}

void Editor::showLineNumbers(bool on)
{
    if (on) {
        connect(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
        connect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);
        connect(m_d->m_lineNumberArea, &LineNumberArea::lineHovered, this, &Editor::lineHovered);
        connect(m_d->m_lineNumberArea, &LineNumberArea::hoverLeaved, this, &Editor::hoverLeaved);

        m_d->m_lineNumberArea->show();
        m_d->m_showLineNumberArea = true;
    } else {
        disconnect(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
        disconnect(this, &Editor::updateRequest, this, &Editor::updateLineNumberArea);
        disconnect(m_d->m_lineNumberArea, &LineNumberArea::lineHovered, this, &Editor::lineHovered);
        disconnect(m_d->m_lineNumberArea, &LineNumberArea::hoverLeaved, this, &Editor::hoverLeaved);

        m_d->m_lineNumberArea->hide();
        m_d->m_showLineNumberArea = false;
    }

    updateLineNumberAreaWidth(0);
}

namespace /* anonymous */
{

template<class Iterator, class C = std::less<>>
bool markSelection(Iterator first, Iterator last, QTextCursor c, Editor *e, C cmp = C{})
{
    for (; first != last; ++first) {
        if (cmp(c.position(), first->cursor.position())) {
            c.setPosition(first->cursor.selectionStart());
            c.setPosition(first->cursor.selectionEnd(), QTextCursor::KeepAnchor);
            e->setTextCursor(c);

            return true;
        }
    }

    return false;
}

} /* namespace anonymous */

void Editor::highlight(const QString &text, bool initCursor)
{
    m_d->m_highlightedText = text;

    m_d->m_extraSelections.clear();

    if (!text.isEmpty()) {
        QTextCursor c(document());

        static const QColor color = QColor(Qt::yellow);

        while (!c.isNull()) {
            QTextEdit::ExtraSelection s;

            s.format.setBackground(color);
            s.cursor = document()->find(text, c, QTextDocument::FindCaseSensitively);

            if (!s.cursor.isNull())
                m_d->m_extraSelections.append(s);

            c = s.cursor;
        }
    }

    m_d->setExtraSelections();

    if (!m_d->m_extraSelections.isEmpty() && initCursor) {
        if (!markSelection(m_d->m_extraSelections.cbegin(), m_d->m_extraSelections.cend(),
                           QTextCursor(firstVisibleBlock()), this)) {
            markSelection(m_d->m_extraSelections.crbegin(), m_d->m_extraSelections.crend(),
                          QTextCursor(firstVisibleBlock()), this, std::greater<>{});
        }
    }
}

void Editor::onFindNext()
{
    if (!m_d->m_extraSelections.isEmpty()) {
        if (!markSelection(m_d->m_extraSelections.cbegin(), m_d->m_extraSelections.cend(), textCursor(), this)) {
            auto s = m_d->m_extraSelections.at(0).cursor;
            auto c = textCursor();
            c.setPosition(s.selectionStart());
            c.setPosition(s.selectionEnd(), QTextCursor::KeepAnchor);
            setTextCursor(c);
        }
    }
}

void Editor::onFindPrev()
{
    if (!m_d->m_extraSelections.isEmpty()) {
        auto c = textCursor();

        if (!markSelection(m_d->m_extraSelections.crbegin(), m_d->m_extraSelections.crend(),
                           textCursor(), this, std::greater<>{})) {
            auto s = m_d->m_extraSelections.at(m_d->m_extraSelections.size() - 1).cursor;
            auto c = textCursor();
            c.setPosition(s.selectionStart());
            c.setPosition(s.selectionEnd(), QTextCursor::KeepAnchor);
            setTextCursor(c);
        }
    }
}

void Editor::clearExtraSelections()
{
    m_d->m_highlightedText.clear();
    m_d->m_extraSelections.clear();

    m_d->setExtraSelections();
}

void Editor::replaceCurrent(const QString &with)
{
    if (foundSelected()) {
        auto c = textCursor();
        c.beginEditBlock();
        c.removeSelectedText();
        c.insertText(with);
        c.endEditBlock();
    }
}

void Editor::replaceAll(const QString &with)
{
    if (foundHighlighted()) {
        disconnect(this, &QPlainTextEdit::textChanged, this, &Editor::onContentChanged);

        QTextCursor editCursor(document());

        editCursor.beginEditBlock();

        QTextCursor found = editCursor;

        while (!found.isNull()) {
            found = document()->find(m_d->m_highlightedText, editCursor, QTextDocument::FindCaseSensitively);

            if (!found.isNull()) {
                editCursor.setPosition(found.selectionStart());
                editCursor.setPosition(found.selectionEnd(), QTextCursor::KeepAnchor);

                editCursor.removeSelectedText();
                editCursor.insertText(with);
            }
        }

        editCursor.endEditBlock();

        clearExtraSelections();
    }

    connect(this, &QPlainTextEdit::textChanged, this, &Editor::onContentChanged);
}

void Editor::onContentChanged()
{
    const auto md = toPlainText();
    QFileInfo info(m_d->m_docName);

    if (m_d->m_currentParsingCounter == std::numeric_limits<unsigned long long int>::max()) {
        m_d->m_currentParsingCounter = 0;
    }

    ++m_d->m_currentParsingCounter;

    emit doParsing(md, info.absolutePath(), info.fileName(), m_d->m_currentParsingCounter);
}

void Editor::onParsingDone(std::shared_ptr<MD::Document<MD::QStringTrait>> doc, unsigned long long int counter)
{
    if (m_d->m_currentParsingCounter == counter) {
        m_d->m_currentDoc = doc;

        highlightSyntax(m_d->m_colors, m_d->m_currentDoc);

        highlightCurrent();

        viewport()->update();

        emit ready();
    }
}

void Editor::highlightCurrent()
{
    highlight(m_d->m_highlightedText, false);
}

void Editor::clearHighlighting()
{
    clearExtraSelections();
}

void Editor::goToLine(int l)
{
    QTextBlock block = document()->begin();
    int blockNumber = block.blockNumber() + 1;

    while (block.isValid() && blockNumber < l) {
        block = block.next();
        ++blockNumber;
    }

    if (!block.isValid()) {
        block = document()->lastBlock();
    }

    auto cursor = textCursor();
    cursor.setPosition(block.position());
    setTextCursor(cursor);

    ensureCursorVisible();

    setFocus();
}

void Editor::setText(const QString &t)
{
    setPlainText(t);
}

void Editor::highlightSyntax(const Colors &colors, std::shared_ptr<MD::Document<MD::QStringTrait>> doc)
{
    m_d->m_syntax.highlight(doc, colors);
}

void Editor::keyPressEvent(QKeyEvent *event)
{
    auto c = textCursor();

    if (c.hasSelection()) {
        switch (event->key()) {
        case Qt::Key_Tab: {
            const auto ss = c.selectionStart();
            const auto se = c.selectionEnd();
            c.setPosition(ss);
            const auto start = c.blockNumber();
            c.setPosition(se, QTextCursor::KeepAnchor);
            const auto end = c.blockNumber();

            c.beginEditBlock();

            for (auto i = start; i <= end; ++i) {
                QTextCursor add(document()->findBlockByNumber(i));
                add.insertText(QStringLiteral("\t"));
            }

            c.endEditBlock();
        } break;

        case Qt::Key_Backtab: {
            const auto ss = c.selectionStart();
            const auto se = c.selectionEnd();
            c.setPosition(ss);
            const auto start = c.blockNumber();
            c.setPosition(se, QTextCursor::KeepAnchor);
            const auto end = c.blockNumber();

            c.beginEditBlock();

            for (auto i = start; i <= end; ++i) {
                QTextCursor del(document()->findBlockByNumber(i));

                if (del.block().text().startsWith(QStringLiteral("\t"))) {
                    del.deleteChar();
                }
            }

            c.endEditBlock();
        } break;

        default:
            QPlainTextEdit::keyPressEvent(event);
        }
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}

} /* namespace MdEditor */

#include "editor.moc"
