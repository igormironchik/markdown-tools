/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "editor.h"
#include "find.h"
#include "mainwindow.h"
#include "settings.h"

// Sonnet include.
#include <Sonnet/Settings>

// Qt include.
#include <QApplication>
#include <QCompleter>
#include <QCursor>
#include <QDesktopServices>
#include <QFileInfo>
#include <QListView>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QScrollBar>
#include <QStringListModel>
#include <QStyle>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QThread>

// C++ include.
#include <functional>
#include <limits>
#include <utility>

// md4qt include.
#include <md4qt/src/algo.h>

// shared include.
#include "emoji.h"
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
    void done(QSharedPointer<MD::Document>,
              unsigned long long int,
              SyntaxVisitor syntax,
              MD::details::IdsMap idsMap,
              Editor::ItemsMap itemsMap);

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
            MD::TextStream tmp(stream);
            QStringList lines;

            while (!tmp.atEnd()) {
                lines.append(tmp.readLine().slicedCopy(0));
            }

            stream.seek(0);

            setPlugins(m_parser, m_pluginsCfg, true);

            const auto doc = m_parser.parse(stream, m_path, m_fileName);

            m_data.clear();

            m_syntax.highlight(&lines, doc, m_syntax.colors());

            MD::details::IdsMap idsMap;
            Editor::ItemsMap itemsMap;

            m_id = 0;

            MD::forEach(
                m_itemTypes,
                doc,
                [&idsMap, &itemsMap, this](MD::Item *item) {
                    const auto id = this->generateId(item);

                    if (!id.isEmpty()) {
                        idsMap.insert(item, id);
                        itemsMap.insert(id, item);
                    }
                },
                1);

            emit done(doc, m_counter, m_syntax, idsMap, itemsMap);
        }
    }

private:
    //! \return Generated ID for a given item.
    QString generateId(MD::Item *item)
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

            return labelToId(static_cast<MD::Heading *>(item)->label());
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
    MD::Parser m_parser;
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
    explicit EditorPrivate(Editor *parent,
                           MainWindow *mainWindow,
                           QStringListModel *tocModel)
        : m_q(parent)
        , m_mainWindow(mainWindow)
        , m_tocModel(tocModel)
        , m_emojiModel(new QStringListModel(s_emojiKeys,
                                            m_q))
        , m_completer(new QCompleter(m_tocModel,
                                     m_q))
        , m_emojiCompleter(new QCompleter(m_emojiModel,
                                          m_q))
        , m_parsingThread(new QThread(m_q))
        , m_parser(new DataParser)
    {
        m_parser->moveToThread(m_parsingThread);
        m_q->setMouseTracking(true);

        m_completer->setCaseSensitivity(Qt::CaseInsensitive);
        m_completer->setCompletionMode(QCompleter::PopupCompletion);
        m_completer->setWidget(m_q->viewport());

        {
            auto completerView = new QListView(m_q);
            completerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            completerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            completerView->setSelectionBehavior(QAbstractItemView::SelectRows);
            completerView->setSelectionMode(QAbstractItemView::SingleSelection);
            completerView->setModelColumn(m_completer->completionColumn());
            completerView->setTextElideMode(Qt::ElideNone);
            m_completer->setPopup(completerView);
            completerView->installEventFilter(m_q);
        }

        QObject::connect(m_completer,
                         QOverload<const QString &>::of(&QCompleter::activated),
                         m_q,
                         &Editor::onCompletionActivated);

        m_emojiCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        m_emojiCompleter->setCompletionMode(QCompleter::PopupCompletion);
        m_emojiCompleter->setWidget(m_q->viewport());

        {
            auto completerView = new QListView(m_q);
            completerView->setEditTriggers(QAbstractItemView::NoEditTriggers);
            completerView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            completerView->setSelectionBehavior(QAbstractItemView::SelectRows);
            completerView->setSelectionMode(QAbstractItemView::SingleSelection);
            completerView->setModelColumn(m_emojiCompleter->completionColumn());
            completerView->setTextElideMode(Qt::ElideNone);
            m_emojiCompleter->setPopup(completerView);
            completerView->installEventFilter(m_q);
        }

        QObject::connect(m_emojiCompleter,
                         QOverload<const QString &>::of(&QCompleter::activated),
                         m_q,
                         &Editor::onEmojiCompletionActivated);

        QObject::connect(m_q, &Editor::doParsing, m_parser, &DataParser::onData, Qt::QueuedConnection);
        QObject::connect(m_parser, &DataParser::done, m_q, &Editor::onParsingDone, Qt::QueuedConnection);
        QObject::connect(m_q, &Editor::linkClicked, m_q, &Editor::onLinkClicked);
    }

    ~EditorPrivate()
    {
        m_parser->deleteLater();
        m_parsingThread->quit();
        m_parsingThread->wait();
    }

    void initUi()
    {
        m_lineNumberArea = new LineNumberArea(m_q);

        QObject::connect(m_q, &Editor::cursorPositionChanged, m_q->viewport(), qOverload<>(&QWidget::update));
        QObject::connect(m_q, &QPlainTextEdit::textChanged, m_q, &Editor::onContentChanged);
        QObject::connect(m_q, &Editor::ready, m_q, &Editor::checkUrlAutocompletion);
        QObject::connect(m_q, &Editor::ready, m_q, &Editor::checkEmojiAutocompletion);
        QObject::connect(m_lineNumberArea,
                         &LineNumberArea::lineNumberContextMenuRequested,
                         m_q,
                         &Editor::lineNumberContextMenuRequested);

        m_q->showLineNumbers(true);
        m_q->applyFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        m_q->updateLineNumberAreaWidth(0);
        m_q->showUnprintableCharacters(true);
        m_q->setFocusPolicy(Qt::ClickFocus);
        m_q->setCenterOnScroll(true);

        m_q->document()->setDocumentLayout(new DocumentLayoutWithRightAlignment(m_q->document(), m_q->viewport()));
        m_q->doUpdate();
        m_q->setAcceptDrops(false);

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

    //! \return Link if it's there...
    MD::Link *isLink(const MD::PosCache::Items &items)
    {
        if (!items.isEmpty()) {
            for (const auto &i : std::as_const(items)) {
                if (i->type() == MD::ItemType::Link) {
                    return static_cast<MD::Link *>(i);
                }
            }
        }

        return nullptr;
    }

    //! Restore cursor.
    void restoreCursor()
    {
        if (m_cursorOverriden) {
            QApplication::restoreOverrideCursor();
            m_cursorOverriden = false;
        }
    }

    //! Check for link hovering.
    void checkForLinkHovering(bool ctrlModifier,
                              const QPoint &pos)
    {
        const auto restore = [this]() {
            if (this->m_underlinedLink.isValid()) {
                this->restoreCursor();

                for (auto i = this->m_underlinedLink.m_startLine; i <= this->m_underlinedLink.m_endLine; ++i) {
                    const auto block = m_q->document()->findBlockByNumber(i);

                    auto formats = block.layout()->formats();

                    if (!formats.isEmpty()) {
                        formats.pop_back();
                        block.layout()->setFormats(formats);
                    }
                }

                this->m_underlinedLink = {};

                this->m_q->viewport()->update();
            }
        };

        const auto underline =
            [this](long long int startLine, long long int startColumn, long long int endLine, long long int endColumn) {
                for (auto i = startLine; i <= endLine; ++i) {
                    QTextCharFormat fmt;
                    fmt.setFontUnderline(true);

                    const auto block = m_q->document()->findBlockByNumber(i);

                    QTextLayout::FormatRange r;
                    r.format = fmt;
                    r.start = (i == startLine ? startColumn : 0);
                    r.length =
                        (i == startLine ? (i == endLine ? endColumn - startColumn + 1 : block.length() - startColumn)
                                        : (i == endLine ? endColumn + 1 : block.length()));

                    auto formats = block.layout()->formats();
                    formats.push_back(r);
                    block.layout()->setFormats(formats);
                }
            };

        if (ctrlModifier) {
            const auto cursor = m_q->cursorForPosition(pos);

            const auto lineNumber = cursor.block().blockNumber();
            const auto pos = cursor.position() - cursor.block().position();

            const auto link = isLink(m_syntax.findFirstInCache({pos, lineNumber, pos, lineNumber}));

            if (link) {
                UnderlinedLink u = {link->startLine(), link->startColumn(), link->endLine(), link->endColumn()};

                if (m_underlinedLink != u) {
                    restore();

                    m_underlinedLink = u;

                    QApplication::setOverrideCursor(Qt::PointingHandCursor);

                    m_cursorOverriden = true;

                    underline(link->startLine(), link->startColumn(), link->endLine(), link->endColumn());

                    m_q->viewport()->update();
                }
            } else {
                restore();
            }
        } else {
            restore();
        }
    }

    //! Editor.
    Editor *m_q = nullptr;
    //! Main window.
    MainWindow *m_mainWindow = nullptr;
    //! ToC model.
    QStringListModel *m_tocModel = nullptr;
    //! Emoji model.
    QStringListModel *m_emojiModel = nullptr;
    //! ID completer.
    QCompleter *m_completer = nullptr;
    //! Emoji completer.
    QCompleter *m_emojiCompleter = nullptr;
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
    QSharedPointer<MD::Document> m_currentDoc;
    //! Map of current items IDs.
    MD::details::IdsMap m_idsMap;
    //! Map of items be theirs IDs.
    Editor::ItemsMap m_itemsMap;
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
    //! Is key was pressed? In case of Ctrl+Z will be false.
    bool m_keyPressed = false;
    //! Is content was changed by keyboard?
    bool m_isContentChangedByKey = false;
    //! Is left mouse button pressed?
    bool m_leftMouseBtnPressed = false;
    //! Settings.
    Settings m_settings;
    //! Current link.
    MD::Link *m_link = nullptr;

    //! Underlined link.
    struct UnderlinedLink {
        long long int m_startLine = -1;
        long long int m_startColumn = -1;
        long long int m_endLine = -1;
        long long int m_endColumn = -1;

        bool operator!=(const UnderlinedLink &other)
        {
            return (m_startLine != other.m_startLine
                    || m_startColumn != other.m_startColumn
                    || m_endLine != other.m_endLine
                    || m_endColumn != other.m_endColumn);
        }

        bool isValid() const
        {
            return (m_startLine != -1 && m_startColumn != -1 && m_endLine != -1 && m_endColumn != -1);
        }
    }; // struct UnderlinedLink

    //! Underlined link coordinates.
    UnderlinedLink m_underlinedLink;
    //! Is cursor overriden?
    bool m_cursorOverriden = false;
    //! Start emoji pos.
    qsizetype m_startEmoji = -1;
    //! End emoji pos.
    qsizetype m_endEmoji = -1;
}; // struct EditorPrivate

//
// Editor
//

Editor::Editor(QWidget *parent,
               MainWindow *mainWindow,
               QStringListModel *tocModel)
    : QPlainTextEdit(parent)
    , m_d(new EditorPrivate(this,
                            mainWindow,
                            tocModel))
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

const MD::details::IdsMap &Editor::idsMap() const
{
    return m_d->m_idsMap;
}

const Editor::ItemsMap &Editor::itemsMap() const
{
    return m_d->m_itemsMap;
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

void Editor::enableAutoCompletionOfLinks(bool on)
{
    m_d->m_settings.m_isLinksAutoCompletionEnabled = on;
}

void Editor::enableAutoCompletionOfEmojies(bool on)
{
    m_d->m_settings.m_isEmojiAutoCompletionEnabled = on;
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

    viewport()->update();
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

QSharedPointer<MD::Document> Editor::currentDoc() const
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
    } else {
        int x = -1;
        int y = -1;
        int h = 0;

        while (visibleBlock.isValid()) {
            auto geometry = blockBoundingGeometry(visibleBlock).translated(contentOffset());

            if (!visibleBlock.next().isValid()) {
                geometry.adjust(0., 0., 0., -document()->documentMargin());
            }

            if (bottom < qRound(geometry.top())) {
                break;
            }

            if (visibleBlock.userData()) {
                const auto currentX = static_cast<CodeBlockBackgroundData *>(visibleBlock.userData())->m_x;

                if (x == -1) {
                    x = currentX;
                    y = qRound(geometry.top());
                    h = qRound(geometry.height());
                } else if (x != currentX) {
                    p.drawRect(x, y, viewport()->rect().width() - x - qRound(document()->documentMargin()), h);

                    x = currentX;
                    y = qRound(geometry.top());
                    h = qRound(geometry.height());
                } else {
                    h += qRound(geometry.height());
                }
            } else if (x != -1) {
                p.drawRect(x, y, viewport()->rect().width() - x - qRound(document()->documentMargin()), h);

                x = -1;
            }

            visibleBlock = visibleBlock.next();
        }

        if (x != -1) {
            p.drawRect(x, y, viewport()->rect().width() - x - qRound(document()->documentMargin()), h);
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

    highlightCurrentLine();

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

void Editor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_d->m_leftMouseBtnPressed = true;
    }

    QPlainTextEdit::mousePressEvent(event);
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_d->m_underlinedLink.isValid() && m_d->m_leftMouseBtnPressed) {
            const auto link = m_d->isLink(m_d->m_syntax.findFirstInCache({m_d->m_underlinedLink.m_startColumn,
                                                                          m_d->m_underlinedLink.m_startLine,
                                                                          m_d->m_underlinedLink.m_startColumn,
                                                                          m_d->m_underlinedLink.m_startLine}));

            if (link) {
                emit linkClicked(link->url());
            }
        }

        m_d->m_leftMouseBtnPressed = false;
    }

    QPlainTextEdit::mouseReleaseEvent(event);
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
    m_d->checkForLinkHovering(event->modifiers().testFlag(Qt::ControlModifier), event->pos());

    QPlainTextEdit::mouseMoveEvent(event);
}

void Editor::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        m_d->checkForLinkHovering(event->modifiers().testFlag(Qt::ControlModifier), mapFromGlobal(QCursor::pos()));
    }

    QPlainTextEdit::keyReleaseEvent(event);
}

bool Editor::eventFilter(QObject *watched,
                         QEvent *event)
{
    if (watched == m_d->m_completer->popup() || watched == m_d->m_emojiCompleter->popup()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if (!keyEvent->matches(QKeySequence::MoveToEndOfBlock)
                && !keyEvent->matches(QKeySequence::MoveToEndOfDocument)
                && !keyEvent->matches(QKeySequence::MoveToEndOfLine)
                && !keyEvent->matches(QKeySequence::MoveToNextChar)
                && !keyEvent->matches(QKeySequence::MoveToNextLine)
                && !keyEvent->matches(QKeySequence::MoveToNextPage)
                && !keyEvent->matches(QKeySequence::MoveToNextWord)
                && !keyEvent->matches(QKeySequence::MoveToPreviousChar)
                && !keyEvent->matches(QKeySequence::MoveToPreviousLine)
                && !keyEvent->matches(QKeySequence::MoveToPreviousPage)
                && !keyEvent->matches(QKeySequence::MoveToPreviousWord)
                && !keyEvent->matches(QKeySequence::MoveToStartOfBlock)
                && !keyEvent->matches(QKeySequence::MoveToStartOfDocument)
                && !keyEvent->matches(QKeySequence::MoveToStartOfLine)
                && !keyEvent->matches(QKeySequence::InsertParagraphSeparator)) {
                keyPressEvent(keyEvent);
            }
        }
    }

    return QPlainTextEdit::eventFilter(watched, event);
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
    QTextOption opts = document()->defaultTextOption();

    if (on) {
        opts.setFlags(opts.flags() | QTextOption::ShowTabsAndSpaces);
    } else {
        opts.setFlags(opts.flags() & ~QTextOption::ShowTabsAndSpaces);
    }

    document()->setDefaultTextOption(opts);

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
        disconnect(this, &QPlainTextEdit::textChanged, this, 0);

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

        connect(this, &QPlainTextEdit::textChanged, this, &Editor::onContentChanged);
    }

    onContentChanged();
}

void Editor::checkUrlAutocompletion()
{
    if (m_d->m_settings.m_isLinksAutoCompletionEnabled) {
        if (m_d->m_isContentChangedByKey) {
            const auto lineNumber = textCursor().block().blockNumber();
            const auto pos = textCursor().positionInBlock() - 1;

            const auto items = syntaxHighlighter().findFirstInCache({pos, lineNumber, pos, lineNumber});

            if (!items.isEmpty() && items.back()->type() == MD::ItemType::Link) {
                auto link = static_cast<MD::Link *>(items.back());

                if (pos >= link->urlPos().startColumn() && pos <= link->urlPos().endColumn()) {
                    const auto url = textCursor().block().text().sliced(
                        link->urlPos().startColumn(),
                        link->urlPos().endColumn() - link->urlPos().startColumn() + 1);

                    if (url.startsWith(QLatin1Char('#'))) {
                        m_d->m_link = link;
                        m_d->m_completer->setCompletionPrefix(url);
                        auto tc = textCursor();
                        tc.setPosition(tc.block().position() + link->urlPos().startColumn());
                        m_d->m_completer->complete(
                            QRect(cursorRect(tc).bottomLeft(),
                                  QSize(m_d->m_completer->popup()->sizeHintForColumn(0)
                                            + (m_d->m_completer->completionCount() > m_d->m_completer->maxVisibleItems()
                                                   ? m_d->m_completer->popup()->verticalScrollBar()->sizeHint().width()
                                                   : 0)
                                            + QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin)
                                            + 1,
                                        1)));
                    }
                }
            }
        }
    }
}

static const QChar s_colon = QLatin1Char(':');
static const QChar s_space = QLatin1Char(' ');
static const QChar s_backslash = QLatin1Char('\\');

MD::Text *findTextInLink(MD::Link *link,
                         qsizetype pos)
{
    for (auto it = link->p()->items().cbegin(), last = link->p()->items().cend(); it != last; ++it) {
        if ((*it)->startColumn() <= pos && (*it)->endColumn() >= pos && (*it)->type() == MD::ItemType::Text) {
            return static_cast<MD::Text *>(it->get());
        }
    }

    return nullptr;
}

void Editor::checkEmojiAutocompletion()
{
    if (m_d->m_settings.m_isEmojiAutoCompletionEnabled) {
        if (m_d->m_isContentChangedByKey) {
            auto tc = textCursor();
            const auto lineNumber = tc.block().blockNumber();
            const auto pos = textCursor().positionInBlock() - 1;

            const auto items = syntaxHighlighter().findFirstInCache({pos, lineNumber, pos, lineNumber});

            if (!items.isEmpty()
                && (items.back()->type() == MD::ItemType::Link || items.back()->type() == MD::ItemType::Text)) {
                MD::Text *textInLink = nullptr;

                if (items.back()->type() == MD::ItemType::Link) {
                    if (pos > static_cast<MD::Link *>(items.back())->textPos().endColumn()) {
                        m_d->m_emojiCompleter->popup()->hide();
                        return;
                    }

                    textInLink = findTextInLink(static_cast<MD::Link *>(items.back()), pos);

                    if (!textInLink) {
                        m_d->m_emojiCompleter->popup()->hide();
                        return;
                    }
                }

                qsizetype startPos = -1;
                qsizetype firstCharPos = tc.block().position()
                    + (items.back()->type() == MD::ItemType::Link ? textInLink->startColumn()
                                                                  : items.back()->startColumn());

                for (qsizetype i = tc.position() - 1; i >= firstCharPos; --i) {
                    const auto ch = document()->characterAt(i);

                    if (ch == s_space) {
                        return;
                    } else if (ch == s_colon) {
                        if (i == firstCharPos || document()->characterAt(i - 1) != s_backslash) {
                            startPos = i - tc.block().position() + 1;
                            firstCharPos = i;
                            m_d->m_startEmoji = i;
                            break;
                        } else {
                            m_d->m_emojiCompleter->popup()->hide();
                            return;
                        }
                    }
                }

                if (startPos != -1) {
                    QString emoji;
                    qsizetype lastCharPos = tc.block().position()
                        + (items.back()->type() == MD::ItemType::Link ? textInLink->endColumn()
                                                                      : items.back()->endColumn());

                    if (tc.position() <= lastCharPos) {
                        for (qsizetype i = tc.position(); i <= lastCharPos; ++i) {
                            if (document()->characterAt(i) == s_space) {
                                emoji = tc.block().text().sliced(startPos, i - tc.block().position() - startPos);
                                m_d->m_endEmoji = i - 1;
                                break;
                            } else if (i == lastCharPos) {
                                m_d->m_endEmoji = lastCharPos;

                                if (lastCharPos >= firstCharPos) {
                                    emoji = tc.block().text().sliced(startPos, lastCharPos - firstCharPos);
                                }
                            }
                        }
                    } else {
                        m_d->m_endEmoji = lastCharPos;

                        if (lastCharPos >= firstCharPos) {
                            emoji = tc.block().text().sliced(startPos, lastCharPos - firstCharPos);
                        }
                    }

                    if (!emoji.isEmpty()) {
                        m_d->m_emojiCompleter->setCompletionPrefix(emoji);
                        auto tc = textCursor();
                        tc.setPosition(tc.block().position() + startPos - 1);
                        m_d->m_emojiCompleter->complete(QRect(
                            cursorRect(tc).bottomLeft(),
                            QSize(m_d->m_emojiCompleter->popup()->sizeHintForColumn(0)
                                      + (m_d->m_emojiCompleter->completionCount()
                                                 > m_d->m_emojiCompleter->maxVisibleItems()
                                             ? m_d->m_emojiCompleter->popup()->verticalScrollBar()->sizeHint().width()
                                             : 0)
                                      + QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin)
                                      + 1,
                                  1)));
                    }
                }
            }
        }
    }
}

void Editor::onCompletionActivated(const QString &text)
{
    if (m_d->m_link) {
        auto tc = QTextCursor(document()->findBlockByNumber(m_d->m_link->urlPos().startLine()));
        const auto startPos = tc.position();
        tc.setPosition(startPos + m_d->m_link->urlPos().startColumn());
        tc.setPosition(startPos + m_d->m_link->urlPos().endColumn() + 1, QTextCursor::KeepAnchor);
        tc.insertText(text);
        setTextCursor(tc);
    }
}

void Editor::onEmojiCompletionActivated(const QString &emoji)
{
    auto tc = textCursor();
    tc.setPosition(m_d->m_startEmoji);
    tc.setPosition(m_d->m_endEmoji + 1, QTextCursor::KeepAnchor);
    tc.insertText(s_colon + emoji + s_colon);
    setTextCursor(tc);
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
    m_d->m_isContentChangedByKey = m_d->m_keyPressed;
    m_d->m_keyPressed = false;

    emit doParsing(md,
                   (m_d->m_useWorkingDir ? m_d->m_workingDirectory : info.absolutePath()),
                   info.fileName(),
                   m_d->m_currentParsingCounter,
                   m_d->m_syntax,
                   m_d->m_settings.m_pluginsCfg);
}

void Editor::onParsingDone(QSharedPointer<MD::Document> doc,
                           unsigned long long int counter,
                           SyntaxVisitor syntax,
                           MD::details::IdsMap idsMap,
                           Editor::ItemsMap itemsMap)
{
    if (m_d->m_currentParsingCounter == counter) {
        m_d->m_currentDoc = doc;
        m_d->m_idsMap = idsMap;
        m_d->m_itemsMap = itemsMap;
        m_d->m_syntax = syntax;
        m_d->m_isReady = true;

        m_d->m_underlinedLink = {};
        m_d->restoreCursor();
        m_d->m_syntax.applyFormats(document());

        highlightCurrent();

        auto block = document()->firstBlock();

        while (block.isValid()) {
            block.setUserData(nullptr);
            block = block.next();
        }

        emit ready();
        emit misspelled(syntaxHighlighter().hasMisspelled());

        viewport()->repaint();
    }
}

void Editor::tryToNavigate(const QString &ref)
{
    auto place = ref;

    if (ref.startsWith(QLatin1Char('#'))) {
        if (!place.endsWith(m_d->m_docName)) {
            place.append(QStringLiteral("/"));
            place.append(m_d->m_docName);
        }
    }

    navigate(place);
}

bool Editor::navigate(const QString &place)
{
    auto hit = m_d->m_currentDoc->labeledHeadings().find(place);

    if (hit != m_d->m_currentDoc->labeledHeadings().cend()) {
        auto c = textCursor();
        c.setPosition(document()->findBlockByNumber(hit.value()->startLine()).position());

        setTextCursor(c);

        ensureCursorVisible();

        return true;
    }

    return false;
}

void Editor::onLinkClicked(const QString &url)
{
    auto place = url;

    auto lit = m_d->m_currentDoc->labeledLinks().find(url);

    if (lit != m_d->m_currentDoc->labeledLinks().cend()) {
        place = lit.value()->url();
    }

    if (place.startsWith(QLatin1Char('#'))) {
        if (navigate(place)) {
            return;
        }
    }

    if (!m_d->m_mainWindow->tryToNavigate(place)) {
        QDesktopServices::openUrl(QUrl(place));
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
    m_d->m_underlinedLink = {};
    m_d->restoreCursor();

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

void replaceListDelims(QString &text,
                       const MD::PosCache::Items &items,
                       bool fenced)
{
    const auto lineNumber =
        (fenced ? static_cast<MD::Code *>(items.back())->startDelim().startLine() : items.back()->startLine());

    auto it = items.crbegin();
    ++it;

    for (auto last = items.crend(); it != last; ++it) {
        if ((*it)->startLine() == lineNumber) {
            if ((*it)->type() == MD::ItemType::ListItem) {
                auto li = static_cast<MD::ListItem *>(*it);

                const auto count = li->delim().endColumn() - li->delim().startColumn() + 1;

                text.replace(li->delim().startColumn(), count, QString(count, QLatin1Char(' ')));
            }
        }
    }
}

static const int s_autoAddedListItem = 1;

bool Editor::handleReturnKeyForCode(QKeyEvent *event,
                                    const MD::PosCache::Items &items,
                                    bool inList)
{
    if (m_d->m_settings.m_isAutoCodeBlocksEnabled && !items.isEmpty() && items.back()->type() == MD::ItemType::Code) {
        if (inList && !m_d->m_settings.m_dontUseAutoListInCodeBlock) {
            return false;
        }

        auto code = static_cast<MD::Code *>(items.back());

        if (!code->isInline()
            && (code->endLine() >= textCursor().block().blockNumber()
                || (code->isFensedCode() && code->endDelim().startLine() == -1))) {
            textCursor().beginEditBlock();
            textCursor().insertBlock();
            textCursor().endEditBlock();

            if (code->isFensedCode()) {
                const auto block = document()->findBlockByNumber(code->startDelim().startLine());
                auto first = block.text().sliced(0, code->startDelim().startColumn());

                if (inList) {
                    replaceListDelims(first, items, true);
                }

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
                auto first = block.text().sliced(0, code->startColumn());

                if (inList) {
                    replaceListDelims(first, items, false);
                }

                textCursor().insertText(first);
            }

            return true;
        }
    }

    return false;
}

inline bool isEmptyParagraphOnly(const MD::Block::Items &items)
{
    return (items.size() == 1
            && items.at(0)->type() == MD::ItemType::Paragraph
            && static_cast<MD::Paragraph *>(items.at(0).get())->isEmpty());
}

void Editor::keyPressEvent(QKeyEvent *event)
{
    m_d->m_link = nullptr;
    m_d->m_keyPressed = !event->text().isEmpty() && !event->matches(QKeySequence::Undo);

    auto c = textCursor();

    if (event == QKeySequence::InsertParagraphSeparator) {
        event->accept();

        const auto lineNumber = c.block().blockNumber();
        const auto lineLength = c.block().length();
        const auto pos = c.position() - c.block().position();

        const auto items = syntaxHighlighter().findFirstInCache({0, lineNumber, lineLength, lineNumber});

        if (m_d->m_settings.m_isAutoListsEnabled) {
            if (!items.isEmpty()) {
                for (auto it = items.crbegin(), last = items.crend(); it != last; ++it) {
                    if ((*it)->type() == MD::ItemType::ListItem) {
                        auto l = static_cast<MD::ListItem *>(*it);

                        if (handleReturnKeyForCode(event, items, true)) {
                            return;
                        }

                        if (!m_d->m_settings.m_githubBehaviour || l->items().size() <= 1) {
                            c.setPosition(document()->findBlockByNumber(l->startLine()).position());

                            if ((l->items().isEmpty() || isEmptyParagraphOnly(l->items()))
                                && c.block().userState() == s_autoAddedListItem) {
                                textCursor().beginEditBlock();
                                c.setPosition(c.position() + lineLength - 1, QTextCursor::KeepAnchor);
                                c.deleteChar();
                                c.block().setUserState(s_unknownUserState);
                                textCursor().endEditBlock();
                            } else {
                                textCursor().beginEditBlock();
                                textCursor().insertBlock();

                                if (pos <= l->delim().endColumn()) {
                                    textCursor().endEditBlock();

                                    return;
                                }

                                if (l->delim().startColumn()) {
                                    c.setPosition(c.block().position() + l->delim().startColumn(),
                                                  QTextCursor::KeepAnchor);
                                    textCursor().insertText(c.selectedText());
                                }

                                c.setPosition(c.block().position() + l->delim().startColumn());
                                c.setPosition(c.block().position() + l->delim().endColumn() + 1,
                                              QTextCursor::KeepAnchor);

                                if (l->listType() == MD::ListItem::Unordered) {
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
            textCursor().beginEditBlock();
            textCursor().insertBlock();
            textCursor().endEditBlock();
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
