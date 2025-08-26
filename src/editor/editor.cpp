/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "editor.h"
#include "find.h"
#include "settings.h"

// Sonnet include.
#include <Sonnet/Settings>

// Qt include.
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QThread>

// C++ include.
#include <functional>
#include <limits>
#include <utility>

// md4qt include.
#include <md4qt/algo.h>
#include <md4qt/plugins.h>

// shared include.
#include "utils.h"

namespace MdEditor
{

bool operator!=(const Margins &l,
                const Margins &r)
{
    return (l.m_enable != r.m_enable || l.m_length != r.m_length);
}

//
// DataParser
//

//! Threaded parsing of Markdown content and syntax highlighting.
class DataParser : public QObject
{
    Q_OBJECT

signals:
    //! Signals about data available for parsing.
    void newData();
    //! Parsing is done.
    void done(std::shared_ptr<MD::Document<MD::QStringTrait>>,
              unsigned long long int,
              SyntaxVisitor syntax,
              MD::details::IdsMap<MD::QStringTrait> idsMap);

public:
    DataParser()
        : m_itemTypes({MD::ItemType::Paragraph,
                       MD::ItemType::Blockquote,
                       MD::ItemType::List,
                       MD::ItemType::Code,
                       MD::ItemType::Table,
                       MD::ItemType::Heading})
    {
        connect(this, &DataParser::newData, this, &DataParser::onParse, Qt::QueuedConnection);
    }

    ~DataParser() override = default;

public slots:
    //! New data arrived.
    void onData(const QString &md,
                const QString &path,
                const QString &fileName,
                unsigned long long int counter,
                SyntaxVisitor syntax,
                const MdShared::PluginsCfg &pluginsCfg)
    {
        m_data.clear();
        m_data.push_back(md);
        m_path = path;
        m_fileName = fileName;
        m_counter = counter;
        m_syntax = syntax;
        m_pluginsCfg = pluginsCfg;

        emit newData();
    }

private slots:
    //! Do parsing.
    void onParse()
    {
        if (!m_data.isEmpty()) {
            QTextStream stream(&m_data.back());
            MD::MdBlock<MD::QStringTrait>::Data data;

            MD::TextStream<MD::QStringTrait> tmp(stream);

            long long int i = 0;

            while (!tmp.atEnd()) {
                data.push_back(std::pair<MD::QStringTrait::InternalString, MD::MdLineData>(tmp.readLine(), {i}));
                ++i;
            }

            stream.seek(0);

            MD::StringListStream<MD::QStringTrait> linesStream(data);

            setPlugins(m_parser, m_pluginsCfg);

            const auto doc = m_parser.parse(stream, m_path, m_fileName, false);

            m_data.clear();

            m_syntax.highlight(&linesStream, doc, m_syntax.colors());

            MD::details::IdsMap<MD::QStringTrait> idsMap;

            m_id = 0;

            MD::forEach<MD::QStringTrait>(
                m_itemTypes,
                doc,
                [&idsMap, this](MD::Item<MD::QStringTrait> *item) {
                    const auto id = this->generateId(item);

                    if (!id.isEmpty()) {
                        idsMap.insert({item, id});
                    }
                },
                1);

            emit done(doc, m_counter, m_syntax, idsMap);
        }
    }

private:
    //! \return Generated ID for a given item.
    QString generateId(MD::Item<MD::QStringTrait> *item)
    {
        if (item->type() != MD::ItemType::Heading) {
            if (m_id == std::numeric_limits<unsigned long long int>::max()) {
                m_id = 0;
            }

            return QStringLiteral("md4qt-line-id-%1/%2/%3").arg(QString::number(++m_id), m_path, m_fileName);
        } else {
            const auto labelToId = [](const QString &label) -> QString {
                auto id = label;

                if (id.startsWith(QStringLiteral("#"))) {
                    id.remove(0, 1);
                }

                return id;
            };

            return labelToId(static_cast<MD::Heading<MD::QStringTrait> *>(item)->label());
        }
    }

private:
    //! Queue of Markdown content.
    QStringList m_data;
    //! Working directory.
    QString m_path;
    //! File name.
    QString m_fileName;
    //! ID of last requested parsing.
    unsigned long long int m_counter;
    //! Parser of Markdown.
    MD::Parser<MD::QStringTrait> m_parser;
    //! Syntax highlighter.
    SyntaxVisitor m_syntax;
    //! List of type of items that should get IDs.
    QVector<MD::ItemType> m_itemTypes;
    //! Internal counter for IDs generation.
    unsigned long long int m_id = 0;
    //! Plugins configuration.
    MdShared::PluginsCfg m_pluginsCfg;
};

//
// DocumentLayoutWithRightAlignment
//

//! Layout to support right alignment of text in plain text editor.
class DocumentLayoutWithRightAlignment : public QPlainTextDocumentLayout
{
public:
    DocumentLayoutWithRightAlignment(QTextDocument *doc,
                                     QWidget *viewport)
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
                availableWidth -= 2 * margin + extraMargin;

                for (int i = 0; i < tl->lineCount(); ++i) {
                    auto line = tl->lineAt(i);
                    line.setPosition({margin + availableWidth - line.naturalTextWidth(), line.position().y()});
                }
            }
        }

        return r;
    }

private:
    //! Editor's viewport.
    QWidget *m_viewport;
};

//
// EditorPrivate
//

struct EditorPrivate {
    explicit EditorPrivate(Editor *parent)
        : m_q(parent)
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
        QObject::connect(m_lineNumberArea,
                         &LineNumberArea::lineNumberContextMenuRequested,
                         m_q,
                         &Editor::lineNumberContextMenuRequested);

        m_q->showLineNumbers(true);
        m_q->applyFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        m_q->updateLineNumberAreaWidth(0);
        m_q->highlightCurrentLine();
        m_q->showUnprintableCharacters(true);
        m_q->setFocusPolicy(Qt::ClickFocus);
        m_q->setCenterOnScroll(true);

        m_q->document()->setDocumentLayout(new DocumentLayoutWithRightAlignment(m_q->document(), m_q->viewport()));
        m_q->doUpdate();

        m_parsingThread->start();
    }

    //! Set highlightings in editor.
    void setExtraSelections()
    {
        QList<QTextEdit::ExtraSelection> tmp = m_syntaxHighlighting;
        tmp << m_extraSelections;

        m_q->setExtraSelections(tmp);
    }

    //! \return New indent.
    QString makeIndent() const
    {
        return (m_settings.m_indentMode == Editor::IndentMode::Tabs
                    ? QStringLiteral("\t")
                    : QString(m_settings.m_indentSpacesCount, QLatin1Char(' ')));
    }

    //! Editor.
    Editor *m_q = nullptr;
    //! Line number area.
    LineNumberArea *m_lineNumberArea = nullptr;
    //! Document's name.
    QString m_docName;
    //! Is line number area shown?
    bool m_showLineNumberArea = true;
    //! Highlightings for "find" mode.
    QList<QTextEdit::ExtraSelection> m_extraSelections;
    //! Syntax highlightings, including spelling checks.
    QList<QTextEdit::ExtraSelection> m_syntaxHighlighting;

    //! Current line format.
    struct CurrentLineFormat {
        //! X.
        int m_x;
        //! Color.
        QColor m_color;
    }; // struct CurrentLineFormat

    //! Current line's highlighting.
    CurrentLineFormat m_currentLine;
    //! Currently highlighted text in "find" mode.
    QString m_highlightedText;
    //! Current parsed Markdown document.
    std::shared_ptr<MD::Document<MD::QStringTrait>> m_currentDoc;
    //! Map of current itemss IDs.
    MD::details::IdsMap<MD::QStringTrait> m_idsMap;
    //! Syntax highlighter.
    SyntaxVisitor m_syntax;
    //! Tread for parsing.
    QThread *m_parsingThread = nullptr;
    //! Data parser.
    DataParser *m_parser = nullptr;
    //! "Find/replace" widget.
    Find *m_find = nullptr;
    //! IDs of a last parsing request.
    unsigned long long int m_currentParsingCounter = 0;
    //! Working directory.
    QString m_workingDirectory;
    //! Should working directory be used?
    bool m_useWorkingDir = false;
    //! Is editor ready?
    bool m_isReady = true;
    //! Settings.
    Settings m_settings;
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

void Editor::setFindWidget(Find *findWidget)
{
    m_d->m_find = findWidget;
}

const MD::details::IdsMap<MD::QStringTrait> &Editor::idsMap() const
{
    return m_d->m_idsMap;
}

bool Editor::isReady() const
{
    return m_d->m_isReady;
}

void Editor::setPluginsCfg(const MdShared::PluginsCfg &cfg)
{
    m_d->m_settings.m_pluginsCfg = cfg;

    onContentChanged();
}

void Editor::enableAutoLists(bool on)
{
    m_d->m_settings.m_isAutoListsEnabled = on;
}

void Editor::enableGithubBehaviour(bool on)
{
    m_d->m_settings.m_githubBehaviour = on;
}

void Editor::enableAutoListInCodeBlock(bool on)
{
    m_d->m_settings.m_dontUseAutoListInCodeBlock = !on;
}

void Editor::enableAutoCodeBlocks(bool on)
{
    m_d->m_settings.m_isAutoCodeBlocksEnabled = on;
}

void Editor::setIndentMode(IndentMode mode)
{
    m_d->m_settings.m_indentMode = mode;
}

void Editor::setIndentSpacesCount(int s)
{
    m_d->m_settings.m_indentSpacesCount = s;
}

const Settings &Editor::settings() const
{
    return m_d->m_settings;
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
            if (c.hasSelection()
                && c.selectionStart() == s.cursor.selectionStart()
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
    m_d->m_settings.m_colors = colors;

    m_d->m_syntax.setColors(colors);

    highlightCurrentLine();
}

void Editor::applyMargins(const Margins &m)
{
    m_d->m_settings.m_margins = m;

    viewport()->update();
}

void Editor::enableSpellingCheck(bool on)
{
    m_d->m_settings.m_enableSpelling = on;

    syntaxHighlighter().spellingSettingsChanged(on);
}

std::shared_ptr<MD::Document<MD::QStringTrait>> Editor::currentDoc() const
{
    return m_d->m_currentDoc;
}

void Editor::applyFont(const QFont &f)
{
    m_d->m_settings.m_font = f;

    setFont(f);

    m_d->m_syntax.setFont(f);
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

void Editor::updateLineNumberArea(const QRect &rect,
                                  int dy)
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
        m_d->m_lineNumberArea->setGeometry(
            QRect(cr.left() + cr.width() - lineNumberAreaWidth(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

struct CodeBlockBackgroundData : public QTextBlockUserData {
    explicit CodeBlockBackgroundData(qsizetype x)
        : m_x(x)
    {
    }

    ~CodeBlockBackgroundData() override = default;

    qsizetype m_x;
}; // struct CodeBlockBackgroundData

void Editor::drawCodeBlocksBackground(QPainter &p)
{
    p.setBrush(
        m_d->m_syntax.codeBlockSyntaxHighlighter()->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding));
    p.setPen(Qt::NoPen);

    auto visibleBlock = firstVisibleBlock();
    const qsizetype bottom = viewport()->rect().bottom();

    if (isReady()) {
        qsizetype top = viewport()->rect().y();
        qsizetype x = -1;
        qsizetype h = 0;

        for (const auto &rect : std::as_const(m_d->m_syntax.highlightedCodeRects())) {
            if (rect.m_endLine >= visibleBlock.blockNumber()) {
                const auto startY = qRound(blockBoundingGeometry(document()->findBlockByNumber(rect.m_startLine))
                                               .translated(contentOffset())
                                               .top());

                if (startY > bottom) {
                    break;
                }

                bool first = true;

                for (auto line = rect.m_startLine; line <= rect.m_endLine; ++line) {
                    const auto block = document()->findBlockByNumber(line);
                    auto geometry = blockBoundingGeometry(block).translated(contentOffset());

                    if (!block.next().isValid()) {
                        geometry.adjust(0., 0., 0., -document()->documentMargin());
                    }

                    if (qRound(geometry.bottom()) > top && first) {
                        top = qRound(geometry.top());

                        first = false;
                    }

                    if (block.lineCount() > 1) {
                        x = 0;
                    }

                    if (line == rect.m_startColumnLine && x != 0) {
                        x = fontMetrics().horizontalAdvance(block.text().sliced(0, rect.m_startColumn),
                                                            block.layout()->textOption())
                            - fontMetrics().horizontalAdvance(QLatin1Char(' ')) * rect.m_spacesBefore
                            + qRound(document()->documentMargin());
                    }

                    if (qRound(geometry.bottom()) > top) {
                        h += qRound(geometry.height());
                    }
                }

                for (auto line = rect.m_startLine; line <= rect.m_endLine; ++line) {
                    document()->findBlockByNumber(line).setUserData(new CodeBlockBackgroundData(x));
                }

                p.drawRect(x, top, viewport()->rect().width() - x - qRound(document()->documentMargin()), h);

                x = -1;
                h = 0;
            }
        }

        highlightCurrentLine();
    } else {
        while (visibleBlock.isValid()) {
            auto geometry = blockBoundingGeometry(visibleBlock).translated(contentOffset());

            if (!visibleBlock.next().isValid()) {
                geometry.adjust(0., 0., 0., -document()->documentMargin());
            }

            if (bottom < qRound(geometry.top())) {
                break;
            }

            if (visibleBlock.userData()) {
                const auto x = static_cast<CodeBlockBackgroundData *>(visibleBlock.userData())->m_x;

                p.drawRect(x,
                           qRound(geometry.top()),
                           viewport()->rect().width() - x - qRound(document()->documentMargin()),
                           qRound(geometry.height()));
            }

            visibleBlock = visibleBlock.next();
        }
    }
}

void Editor::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());

    if (m_d->m_settings.m_colors.m_codeThemeEnabled && m_d->m_settings.m_colors.m_drawCodeBackground) {
        drawCodeBlocksBackground(painter);
    }

    if (m_d->m_settings.m_margins.m_enable) {
        QRect r = viewport()->rect();
        QFontMetricsF fm(font());

        if (layoutDirection() == Qt::LeftToRight) {
            r.setX(document()->documentMargin() + qRound(fm.averageCharWidth() * m_d->m_settings.m_margins.m_length));
        } else {
            r.setWidth(r.width()
                       - qRound(fm.averageCharWidth() * m_d->m_settings.m_margins.m_length)
                       - document()->documentMargin());
        }

        painter.setBrush(QColor(239, 239, 239, 200));
        painter.setPen(Qt::NoPen);
        painter.drawRect(r);
    }

    painter.setBrush(m_d->m_currentLine.m_color);
    painter.setPen(Qt::NoPen);
    const auto cr = cursorRect();
    painter.drawRect(m_d->m_currentLine.m_x,
                     cr.top(),
                     viewport()->rect().width() - qRound(document()->documentMargin()) - m_d->m_currentLine.m_x,
                     cr.height());

    QPlainTextEdit::paintEvent(event);
}

void Editor::contextMenuEvent(QContextMenuEvent *event)
{
    auto menu = createStandardContextMenu(event->pos());

    auto c = cursorForPosition(event->pos());
    const auto line = c.block().blockNumber();
    const auto pos = c.position() - c.block().position();

    QPair<long long int, long long int> wordPos;
    QMap<QAction *, QString> suggested;

    if (syntaxHighlighter().isMisspelled(line, pos, wordPos)) {
        c.setPosition(c.block().position() + wordPos.first);
        c.setPosition(c.block().position() + wordPos.second + 1, QTextCursor::KeepAnchor);
        const auto word = c.selectedText();

        const auto suggestions = syntaxHighlighter().spellSuggestions(word);

        if (!suggestions.isEmpty()) {
            menu->addSeparator();
            auto suggestionsMenu = new QMenu(tr("Spelling"), menu);
            menu->addMenu(suggestionsMenu);

            for (const auto &w : std::as_const(suggestions)) {
                suggested.insert(suggestionsMenu->addAction(w), w);
            }
        }

        if (suggestions.isEmpty()) {
            menu->addSeparator();
        }

        menu->addAction(tr("Skip Word"), [word, this]() {
            Sonnet::Settings sonnet;
            auto ignored = sonnet.currentIgnoreList();
            ignored.append(word);
            sonnet.setCurrentIgnoreList(ignored);
            this->enableSpellingCheck(this->syntaxHighlighter().isSpellingEnabled());
            this->doUpdate();
            sonnet.save();
        });
    }

    auto action = menu->exec(event->globalPos());

    if (suggested.contains(action)) {
        c.beginEditBlock();
        c.removeSelectedText();
        c.insertText(suggested[action]);
        c.endEditBlock();
    }

    event->accept();

    menu->deleteLater();
}

bool Editor::canInsertFromMimeData(const QMimeData *source) const
{
    return source->hasText();
}

void Editor::insertFromMimeData(const QMimeData *source)
{
    insertPlainText(source->text());
}

void Editor::highlightCurrentLine()
{
    static const QColor lineColor = QColor(255, 255, 0, 75);

    if (textCursor().block().userData()
        && m_d->m_settings.m_colors.m_codeThemeEnabled
        && m_d->m_settings.m_colors.m_drawCodeBackground) {
        m_d->m_currentLine.m_color = QColor(
            m_d->m_syntax.codeBlockSyntaxHighlighter()->theme().editorColor(KSyntaxHighlighting::Theme::CurrentLine));
        m_d->m_currentLine.m_x = static_cast<CodeBlockBackgroundData *>(textCursor().block().userData())->m_x;
    } else {
        m_d->m_currentLine.m_color = lineColor;
        m_d->m_currentLine.m_x = document()->documentMargin();
    }

    viewport()->update();
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

void LineNumberArea::contextMenuEvent(QContextMenuEvent *event)
{
    emit lineNumberContextMenuRequested(m_lineNumber, event->globalPos());

    event->accept();
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

template<class Iterator,
         class C = std::less<>>
bool markSelection(Iterator first,
                   Iterator last,
                   QTextCursor c,
                   Editor *e,
                   C cmp = C{})
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

void Editor::highlight(const QString &text,
                       bool initCursor,
                       QTextDocument::FindFlags findFlags)
{
    m_d->m_highlightedText = text;

    m_d->m_extraSelections.clear();

    if (!text.isEmpty()) {
        QTextCursor c(document());

        static const QColor color = QColor(Qt::yellow);

        while (!c.isNull()) {
            QTextEdit::ExtraSelection s;

            s.format.setBackground(color);
            s.cursor = document()->find(text, c, findFlags);

            if (!s.cursor.isNull()) {
                m_d->m_extraSelections.append(s);
            }

            c = s.cursor;
        }
    }

    m_d->setExtraSelections();

    if (!m_d->m_extraSelections.isEmpty() && initCursor) {
        if (!markSelection(m_d->m_extraSelections.cbegin(),
                           m_d->m_extraSelections.cend(),
                           QTextCursor(firstVisibleBlock()),
                           this)) {
            markSelection(m_d->m_extraSelections.crbegin(),
                          m_d->m_extraSelections.crend(),
                          QTextCursor(firstVisibleBlock()),
                          this,
                          std::greater<>{});
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
        if (!markSelection(m_d->m_extraSelections.crbegin(),
                           m_d->m_extraSelections.crend(),
                           textCursor(),
                           this,
                           std::greater<>{})) {
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

    onContentChanged();
}

void Editor::onContentChanged()
{
    const auto md = toPlainText();
    QFileInfo info(m_d->m_docName);

    if (m_d->m_currentParsingCounter == std::numeric_limits<unsigned long long int>::max()) {
        m_d->m_currentParsingCounter = 0;
    }

    ++m_d->m_currentParsingCounter;

    m_d->m_isReady = false;

    emit doParsing(md,
                   (m_d->m_useWorkingDir ? m_d->m_workingDirectory : info.absolutePath()),
                   info.fileName(),
                   m_d->m_currentParsingCounter,
                   m_d->m_syntax,
                   m_d->m_settings.m_pluginsCfg);
}

void Editor::onParsingDone(std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                           unsigned long long int counter,
                           SyntaxVisitor syntax,
                           MD::details::IdsMap<MD::QStringTrait> idsMap)
{
    if (m_d->m_currentParsingCounter == counter) {
        m_d->m_currentDoc = doc;
        m_d->m_idsMap = idsMap;
        m_d->m_syntax = syntax;

        m_d->m_syntax.applyFormats(document());

        highlightCurrent();

        viewport()->update();

        m_d->m_isReady = true;

        auto block = document()->firstBlock();

        while (block.isValid()) {
            block.setUserData(nullptr);
            block = block.next();
        }

        emit misspelled(syntaxHighlighter().hasMisspelled());
        emit ready();
    }
}

void Editor::highlightCurrent()
{
    highlight(m_d->m_highlightedText, false, m_d->m_find->findFlags());
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

void Editor::onNextMisspelled()
{
    syntaxHighlighter().highlightNextMisspelled(this);
}

void Editor::onWorkingDirectoryChange(const QString &wd,
                                      bool useWorkingDir)
{
    m_d->m_workingDirectory = wd;
    m_d->m_useWorkingDir = useWorkingDir;

    onContentChanged();
}

void Editor::doUpdate()
{
    onContentChanged();

    viewport()->update();
}

static const int s_unknownUserState = -1;

void Editor::clearUserStateOnAllBlocks()
{
    auto block = document()->firstBlock();

    while (block.isValid()) {
        block.setUserState(s_unknownUserState);
        block = block.next();
    }
}

static const int s_autoAddedListItem = 1;

bool Editor::handleReturnKeyForCode(QKeyEvent *event,
                                    const MD::PosCache<MD::QStringTrait>::Items &items,
                                    bool inList)
{
    if (m_d->m_settings.m_isAutoCodeBlocksEnabled && !items.isEmpty() && items.back()->type() == MD::ItemType::Code) {
        if (inList && !m_d->m_settings.m_dontUseAutoListInCodeBlock) {
            return false;
        }

        auto code = static_cast<MD::Code<MD::QStringTrait> *>(items.back());

        if (!code->isInline()
            && (code->endLine() >= textCursor().block().blockNumber()
                || (code->isFensedCode() && code->endDelim().startLine() == -1))) {
            QPlainTextEdit::keyPressEvent(event);

            if (code->isFensedCode()) {
                const auto block = document()->findBlockByNumber(code->startDelim().startLine());
                const auto first = block.text().sliced(0, code->startDelim().startColumn());
                textCursor().insertText(first);

                if (!code->syntax().isEmpty()
                    && !(inList
                         && code->endDelim().startLine() == -1
                         && textCursor().block().blockNumber() - 1 == code->endLine())) {
                    textCursor().block().setUserData(
                        new CodeBlockBackgroundData(fontMetrics().horizontalAdvance(first, block.layout()->textOption())
                                                    + document()->documentMargin()));
                }
            } else {
                const auto block = document()->findBlockByNumber(code->startLine());
                const auto first = block.text().sliced(0, code->startColumn());
                textCursor().insertText(first);
            }

            return true;
        }
    }

    return false;
}

void Editor::keyPressEvent(QKeyEvent *event)
{
    auto c = textCursor();

    if (event->key() == Qt::Key_Return) {
        event->accept();

        const auto lineNumber = c.block().blockNumber();
        const auto lineLength = c.block().length();

        const auto items = syntaxHighlighter().findFirstInCache({0, lineNumber, lineLength, lineNumber});

        if (m_d->m_settings.m_isAutoListsEnabled) {
            if (!items.isEmpty()) {
                for (auto it = items.crbegin(), last = items.crend(); it != last; ++it) {
                    if ((*it)->type() == MD::ItemType::ListItem) {
                        auto l = static_cast<MD::ListItem<MD::QStringTrait> *>(*it);

                        if (handleReturnKeyForCode(event, items, true)) {
                            return;
                        }

                        if (!m_d->m_settings.m_githubBehaviour || l->items().size() <= 1) {
                            c.setPosition(document()->findBlockByNumber(l->startLine()).position());

                            if (l->items().isEmpty() && c.block().userState() == s_autoAddedListItem) {
                                textCursor().beginEditBlock();
                                c.setPosition(c.position() + lineLength - 1, QTextCursor::KeepAnchor);
                                c.deleteChar();
                                c.block().setUserState(s_unknownUserState);
                                textCursor().endEditBlock();
                            } else {
                                textCursor().beginEditBlock();

                                QPlainTextEdit::keyPressEvent(event);

                                if (l->delim().startColumn()) {
                                    c.setPosition(c.block().position() + l->delim().startColumn(),
                                                  QTextCursor::KeepAnchor);
                                    textCursor().insertText(c.selectedText());
                                }

                                c.setPosition(c.block().position() + l->delim().startColumn());
                                c.setPosition(c.block().position() + l->delim().endColumn() + 1,
                                              QTextCursor::KeepAnchor);

                                if (l->listType() == MD::ListItem<MD::QStringTrait>::Unordered) {
                                    textCursor().insertText(c.selectedText());
                                } else {
                                    const auto delim = c.selectedText();
                                    const auto number = delim.sliced(0, delim.length() - 1).toInt();
                                    textCursor().insertText(QString::number(number + 1));
                                    textCursor().insertText(delim.back());
                                }

                                textCursor().insertText(QString(1, QLatin1Char(' ')));

                                if (l->isTaskList()) {
                                    textCursor().insertText(QStringLiteral("[ ] "));
                                }

                                textCursor().block().setUserState(s_autoAddedListItem);

                                textCursor().endEditBlock();
                            }

                            return;
                        } else {
                            break;
                        }
                    }
                }
            }
        }

        if (!handleReturnKeyForCode(event, items, false)) {
            QPlainTextEdit::keyPressEvent(event);
        }

        return;
    }

    if (c.hasSelection()) {
        switch (event->key()) {
        case Qt::Key_Tab: {
            const auto ss = c.selectionStart();
            const auto se = c.selectionEnd();
            c.setPosition(ss);
            const auto start = c.blockNumber();
            c.setPosition(se, QTextCursor::KeepAnchor);
            const auto end = c.blockNumber();

            const auto indent = m_d->makeIndent();

            c.beginEditBlock();

            for (auto i = start; i <= end; ++i) {
                QTextCursor add(document()->findBlockByNumber(i));
                add.insertText(indent);
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

            const auto indent = m_d->makeIndent();

            c.beginEditBlock();

            for (auto i = start; i <= end; ++i) {
                QTextCursor del(document()->findBlockByNumber(i));

                if (del.block().text().startsWith(indent)) {
                    if (m_d->m_settings.m_indentMode == Editor::IndentMode::Spaces) {
                        del.setPosition(del.position() + m_d->m_settings.m_indentSpacesCount, QTextCursor::KeepAnchor);
                    }

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
