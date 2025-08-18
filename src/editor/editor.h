/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QPlainTextEdit>
#include <QScopedPointer>

// md-editor include.
#include "colorsdlg.h"
#include "syntaxvisitor.h"

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/html.h>
#include <md4qt/parser.h>

// shared include.
#include "plugins_page.h"

namespace MdEditor
{

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
                   const MdShared::PluginsCfg &pluginsCfg);

public:
    explicit Editor(QWidget *parent);
    ~Editor() override;

    //! Set document file name.
    void setDocName(const QString &name);
    //! \return Document file name.
    const QString &docName() const;

    //! Draw line number area.
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    //! \return Width of line number area.
    int lineNumberAreaWidth();
    //! \return Is any text in editor highlighted?
    bool foundHighlighted() const;
    //! \return Is any highlighted text in editor selected?
    bool foundSelected() const;
    //! Apply colors scheme.
    void applyColors(const Colors &colors);
    //! Enable/disable spelling check.
    void enableSpellingCheck(bool on);
    //! Current Mardown AST (parsed document).
    std::shared_ptr<MD::Document<MD::QStringTrait>> currentDoc() const;
    //! Apply font for editor.
    void applyFont(const QFont &f);
    //! \return Syntax highlighter.
    SyntaxVisitor &syntaxHighlighter() const;
    //! \return Setting for grayed area in editor.
    Margins &margins();
    //! Set "find" widget for editor.
    void setFindWidget(Find *findWidget);
    //! \return Map if IDs of the current parsed Markdown document.
    const MD::details::IdsMap<MD::QStringTrait> &idsMap() const;
    //! \return Is document ready?
    bool isReady() const;
    //! Set plugins configuration.
    void setPluginsCfg(const MdShared::PluginsCfg &cfg);
    //! \return Is auto-lists enabled?
    bool isAutoListsEnabled() const;
    //! Enable/disable auto-lists.
    void enableAutoLists(bool on = true);

    //! Indent mode.
    enum class IndentMode {
        //! Use tabs.
        Tabs = 0,
        //! Use spaces.
        Spaces = 1
    };

    //! Set indent mode.
    void setIndentMode(IndentMode mode);
    //! \return Indent mode.
    IndentMode indentMode() const;

    //! Set amount of spaces in indent.
    void setIndentSpacesCount(int s);
    //! \return Amount of spaces in indent.
    int indentSpacesCount() const;

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
    void clearUserStateOnAllBlocks();

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
    void onParsingDone(std::shared_ptr<MD::Document<MD::QStringTrait>> doc,
                       unsigned long long int counter,
                       SyntaxVisitor syntax,
                       MD::details::IdsMap<MD::QStringTrait> idsMap);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;

protected:
    //! \return Line number for a given point.
    int lineNumber(const QPoint &p);

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
    //! COntext menu requested on line number.
    void lineNumberContextMenuRequested(int lineNumber,
                                        const QPoint &pos);

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

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

    void enterEvent(QEnterEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    //! Actual processing of hover event on line numbers.
    void onHover(const QPoint &p);

private:
    Editor *m_codeEditor;
    int m_lineNumber = -1;
}; // class LineNumberArea

} /* namespace MdEditor */
