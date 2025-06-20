/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QPlainTextEdit>
#include <QScopedPointer>

// md-editor include.
#include "colorsdlg.hpp"
#include "syntaxvisitor.hpp"

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/parser.h>
#include <md4qt/html.h>

namespace MdEditor
{

//
// Margins
//

struct Margins {
    bool m_enable = false;
    int m_length = 80;
}; // struct Margins

bool operator!=(const Margins &l, const Margins &r);

//
// Editor
//

struct EditorPrivate;
class SyntaxVisitor;
class Find;

//! Markdown text editor.
class Editor : public QPlainTextEdit
{
    Q_OBJECT

signals:
    void lineHovered(int lineNumber, const QPoint &pos);
    void lineNumberContextMenuRequested(int lineNumber, const QPoint &pos);
    void hoverLeaved();
    void ready();
    void misspelled(bool found);
    void doParsing(const QString &md, const QString &path, const QString &fileName, unsigned long long int counter,
                   SyntaxVisitor syntax);
    void doHighlight();

public:
    explicit Editor(QWidget *parent);
    ~Editor() override;

    void setDocName(const QString &name);
    const QString &docName() const;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool foundHighlighted() const;
    bool foundSelected() const;
    void applyColors(const Colors &colors);
    void enableSpellingCheck(bool on);
    std::shared_ptr<MD::Document<MD::QStringTrait>> currentDoc() const;
    void applyFont(const QFont &f);
    SyntaxVisitor &syntaxHighlighter() const;
    Margins &margins();
    void setFindWidget(Find *findWidget);
    const MD::details::IdsMap<MD::QStringTrait> &idsMap() const;
    bool isReady() const;

public slots:
    void showUnprintableCharacters(bool on);
    void showLineNumbers(bool on);
    void highlight(const QString &text, bool initCursor, QTextDocument::FindFlags findFlags);
    void clearExtraSelections();
    void replaceCurrent(const QString &with);
    void replaceAll(const QString &with);
    void highlightCurrent();
    void clearHighlighting();
    void goToLine(int l);
    void setText(const QString &t);
    void onNextMisspelled();
    void onWorkingDirectoryChange(const QString &wd, bool useWorkingDir);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void onFindNext();
    void onFindPrev();
    void onContentChanged();
    void onParsingDone(std::shared_ptr<MD::Document<MD::QStringTrait>> doc, unsigned long long int counter,
                       SyntaxVisitor syntax, MD::details::IdsMap<MD::QStringTrait> idsMap);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

protected:
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
    void lineHovered(int lineNumber, const QPoint &pos);
    void hoverLeaved();
    void lineNumberContextMenuRequested(int lineNumber, const QPoint &pos);

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
    void onHover(const QPoint &p);

private:
    Editor *m_codeEditor;
    int m_lineNumber = -1;
}; // class LineNumberArea

} /* namespace MdEditor */
