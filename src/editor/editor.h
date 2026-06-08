/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QPlainTextEdit>
#include <QScopedPointer>

// md-editor include.
#include "syntaxvisitor.h"

// md-utils include.
#include "diff.h"

// md4qt include.
#include <md4qt/src/html.h>
#include <md4qt/src/parser.h>

// shared include.
#include "plugins_page.h"

QT_BEGIN_NAMESPACE
class QStringListModel;
QT_END_NAMESPACE

namespace MdEditor
{

class LineNumberArea;

//! \return Whether block is folded.
bool isFolded(const QTextBlock &block);

struct Settings;

//
// Margins
//

//! Settings for grayed space in editor - showing area of a given length in characters.
struct Margins {
    //! Is enabled?
    bool m_enable = false;
    //! Length in characters.
    int m_length = 80;
}; // struct Margins

//! Not-equality operator for settings of grayed space (margin).
bool operator!=(const Margins &l,
                const Margins &r);

//
// Editor
//

struct EditorPrivate;
class SyntaxVisitor;
class Find;
class MainWindow;

//! Markdown text editor. Actual text editor where user can type...
class Editor : public QPlainTextEdit
{
    Q_OBJECT

signals:
    //! Line number hovered.
    void lineHovered(int lineNumber,
                     const QPoint &pos);
    //! Context menu on line number was requested.
    void lineNumberContextMenuRequested(int lineNumber,
                                        const QPoint &pos);
    //! Hover leaved from line number.
    void hoverLeaved();
    //! Editor is ready, there are no pending threaded parsings of content.
    void ready();
    //! After parsing was found at least one misspelled word.
    void misspelled(bool found);
    //! Request for parsing of Markdon on thread.
    void doParsing(const QString &md,
                   const QString &path,
                   const QString &fileName,
                   unsigned long long int counter,
                   SyntaxVisitor syntax,
                   const MdShared::PluginsCfg &pluginsCfg,
                   QSharedPointer<MdUtils::BlockLines> blocks);
    //! Link clicked.
    void linkClicked(const QString &url);

public:
    Editor(QWidget *parent,
           MainWindow *mainWindow,
           QStringListModel *tocModel);
    ~Editor() override;

    //! Set document file name.
    void setDocName(const QString &name);
    //! \return Document file name.
    const QString &docName() const;

    //! Type of the map of items be theirs IDs.
    using ItemsMap = QHash<QString, MD::Item *>;

    //! Draw line number area.
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    //! \return Width of collapsing blocks handles.
    int collapsingBlockHandleWidth() const;
    //! \return Width of line number area.
    int lineNumberAreaWidth() const;
    //! \return Is any text in editor highlighted?
    bool foundHighlighted() const;
    //! \return Is any highlighted text in editor selected?
    bool foundSelected() const;
    //! Apply colors scheme.
    void applyColors(const Colors &colors);
    //! Apply margin.
    void applyMargins(const Margins &m);
    //! Enable/disable spelling check.
    void enableSpellingCheck(bool on);
    //! Current Mardown AST (parsed document).
    QSharedPointer<MD::Document> currentDoc() const;
    //! Apply font for editor.
    void applyFont(const QFont &f);
    //! \return Syntax highlighter.
    SyntaxVisitor &syntaxHighlighter() const;
    //! Set "find" widget for editor.
    void setFindWidget(Find *findWidget);
    //! \return Map if IDs of the current parsed Markdown document.
    const MD::details::IdsMap &idsMap() const;
    //! \return Maop of items be theirs IDs.
    const ItemsMap &itemsMap() const;
    //! \return Is document ready?
    bool isReady() const;
    //! Set plugins configuration.
    void setPluginsCfg(const MdShared::PluginsCfg &cfg);
    //! Enable/disable auto-lists.
    void enableAutoLists(bool on = true);
    //! Enable auto add list after non-first block of list item (like on GitHub).
    void enableGithubBehaviour(bool on = true);
    //! Enable auto add list in code block in list item.
    void enableAutoListInCodeBlock(bool on = true);
    //! Enable auto continue of code blocks.
    void enableAutoCodeBlocks(bool on = true);
    //! Enable auto-completion of internal links.
    void enableAutoCompletionOfLinks(bool on = true);
    //! Enable auto-completion of Emojies.
    void enableAutoCompletionOfEmojies(bool on = true);
    //! Collapse/uncollapse block.
    void collapse(qsizetype line,
                  bool on);

    //! Indent mode.
    enum class IndentMode {
        //! Use tabs.
        Tabs = 0,
        //! Use spaces.
        Spaces = 1
    };

    //! Set indent mode.
    void setIndentMode(IndentMode mode);
    //! Set amount of spaces in indent.
    void setIndentSpacesCount(int s);
    //! Enable preview to follow editor.
    void setPreviewFollowEditor(bool on = true);
    //! \return Settings.
    const Settings &settings() const;
    //! \return Lines of blocks.
    const MdUtils::BlockLines &blockLines() const;
    //! \return Line number area widget.
    LineNumberArea *lineNumberArea() const;

public slots:
    //! Enable/disable showing of unprintable characters.
    void showUnprintableCharacters(bool on);
    //! Enable/disable showing of line number area.
    void showLineNumbers(bool on);
    //! Highlight a given text in editor.
    void highlight(const QString &text,
                   bool initCursor,
                   QTextDocument::FindFlags findFlags);
    //! Clear highlighting of any text in editor.
    void clearExtraSelections();
    //! Replace current selected text with a given text.
    void replaceCurrent(const QString &with);
    //! Replace all highlighted text with the given text.
    void replaceAll(const QString &with);
    //! Highlight text that should be highlighted. Useful after content's update.
    void highlightCurrent();
    //! Just a synonym for clearExtraSelections().
    void clearHighlighting();
    //! Go to a given line.
    void goToLine(int l);
    //! Set a content of the editor.
    void setText(const QString &t);
    //! Highlight next misspelled word.
    void onNextMisspelled();
    //! Set working directory.
    void onWorkingDirectoryChange(const QString &wd,
                                  bool useWorkingDir);
    //! Update editor.
    void doUpdate();
    //! Clear user state on all blocks.
    void clearAutoListStateOnAllBlocks();
    //! Try to navigate to reference.
    void tryToNavigate(const QString &ref);
    //! Notify that file was saved.
    void fileWasSaved();
    //! Enable/disable auto saving of content.
    void enableAutoSave(bool on = true);
    //! Unfold the line.
    void unfoldLine(const QTextCursor &cursor);

private slots:
    //! Calculate and change line number area width.
    void updateLineNumberAreaWidth(int newBlockCount);
    //! Highlight current line (line on which cursor is positioned).
    void highlightCurrentLine();
    //! Update line number area (repaint).
    void updateLineNumberArea(const QRect &rect,
                              int dy);
    //! Highlight next.
    void onFindNext();
    //! Highlight previous.
    void onFindPrev();
    //! Process content's change.
    void onContentChanged();
    //! Process finish of Markdown content parsing on a thread.
    void onParsingDone(QSharedPointer<MD::Document> doc,
                       unsigned long long int counter,
                       SyntaxVisitor syntax,
                       MD::details::IdsMap idsMap,
                       Editor::ItemsMap itemsMap,
                       QSharedPointer<MdUtils::BlockLines> blockLines,
                       MdUtils::BlockLinesDiff diff);
    //! Link clicked.
    void onLinkClicked(const QString &url);
    //! Check for URL auto-completion.
    void checkUrlAutocompletion();
    //! Check for Emoji autocompletion.
    void checkEmojiAutocompletion();
    //! Completion activated.
    void onCompletionActivated(const QString &text);
    //! Emoji completion activated.
    void onEmojiCompletionActivated(const QString &emoji);

private:
    //! Navigate to reference.
    bool navigate(const QString &place);
    //! Show/hide lines.
    void setBlockVisible(qsizetype line,
                         bool on);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched,
                     QEvent *event) override;

protected:
    //! \return Line number for a given point.
    int lineNumber(const QPoint &p);
    //! Draw code blocks background.
    void drawCodeBlocksBackground(QPainter &p);
    //! Handle "Return: key for code.
    bool handleReturnKeyForCode(QKeyEvent *event,
                                const MD::PosCache::Items &items,
                                bool inList);

private:
    friend struct EditorPrivate;
    friend class LineNumberArea;
    friend class FindPrivate;

    Q_DISABLE_COPY(Editor)

    QScopedPointer<EditorPrivate> m_d;
}; // class Editor

//
// LineNumberArea
//

//! Line number area.
class LineNumberArea : public QWidget
{
    Q_OBJECT

signals:
    //! Line number hovered.
    void lineHovered(int lineNumber,
                     const QPoint &pos);
    //! Hover leaved a line number.
    void hoverLeaved();
    //! Context menu requested on line number.
    void lineNumberContextMenuRequested(int lineNumber,
                                        const QPoint &pos);
    //! Shading area changed.
    void shadingAreaChanged();

public:
    LineNumberArea(Editor *editor)
        : QWidget(editor)
        , m_codeEditor(editor)
    {
        setMouseTracking(true);
    }

    QSize sizeHint() const override
    {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

    //! \return Whether folding handle is on the given line.
    bool isFoldingHandleHere(qsizetype line) const;
    //! \return Folding line for the given line.
    qsizetype foldedLineNumber(qsizetype line) const;
    //! \return Neares folding marker for the given line.
    qsizetype nearestFoldingLineNumber(qsizetype line) const;
    //! \return Currently highlighed block.
    const MdUtils::BlockLines &highlightedBlock() const;
    //! Set shading area.
    void setShadingArea(int top,
                        int bottom);
    //! \return Shading area.
    const QPair<int,
                int> &
    shadingArea() const;
    //! \return Whether area is hovered.
    bool isHovered() const;

public slots:
    //! Update hover.
    void updateHover();
    //! Clear any hovering state.
    void clearHovering();

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

    void enterEvent(QEnterEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    //! Actual processing of hover event on line numbers.
    void onHover(const QPoint &p);

private:
    Editor *m_codeEditor;
    int m_lineNumber = -1;
    bool m_leftBtnPressed = false;
    MdUtils::BlockLines m_highlightedBlock;
    QPair<int, int> m_shadingArea = {-1, -1};
    bool m_isHovered = false;
}; // class LineNumberArea

} /* namespace MdEditor */
