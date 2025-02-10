/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "find.hpp"
#include "editor.hpp"
#include "mainwindow.hpp"
#include "ui_find.h"

// Qt include.
#include <QPalette>

namespace MdEditor
{

//
// FindPrivate
//

struct FindPrivate {
    FindPrivate(MainWindow *w, Editor *e, Find *parent)
        : m_q(parent)
        , m_editor(e)
        , m_window(w)
    {
    }

    void initUi()
    {
        m_ui.setupUi(m_q);

        QObject::connect(m_ui.findEdit, &QLineEdit::textChanged, m_q, &Find::onFindTextChanged);
        QObject::connect(m_editor, &Editor::ready, m_q, &Find::onEditorReady);

        auto findPrevAction = new QAction(Find::tr("Find Previous"), m_q);
        findPrevAction->setShortcutContext(Qt::ApplicationShortcut);
        findPrevAction->setShortcut(Find::tr("Shift+F3"));
        findPrevAction->setToolTip(Find::tr("Find Previous <small>Shift+F3</small>"));
        m_ui.findPrevBtn->setDefaultAction(findPrevAction);
        m_ui.findPrevBtn->setEnabled(false);

        auto findNextAction = new QAction(Find::tr("Find Next"), m_q);
        findNextAction->setShortcutContext(Qt::ApplicationShortcut);
        findNextAction->setShortcut(Find::tr("F3"));
        findNextAction->setToolTip(Find::tr("Find Next <small>F3</small>"));
        m_ui.findNextBtn->setDefaultAction(findNextAction);
        m_ui.findNextBtn->setEnabled(false);

        auto replaceAction = new QAction(Find::tr("Replace"), m_q);
        replaceAction->setToolTip(Find::tr("Replace Selected"));
        m_ui.replaceBtn->setDefaultAction(replaceAction);
        m_ui.replaceBtn->setEnabled(false);

        auto replaceAllAction = new QAction(Find::tr("Replace All"), m_q);
        replaceAllAction->setToolTip(Find::tr("Replace All"));
        m_ui.replaceAllBtn->setDefaultAction(replaceAllAction);
        m_ui.replaceAllBtn->setEnabled(false);

        m_textColor = m_ui.findEdit->palette().color(QPalette::Text);

        QObject::connect(findPrevAction, &QAction::triggered, m_editor, &Editor::onFindPrev);
        QObject::connect(findNextAction, &QAction::triggered, m_editor, &Editor::onFindNext);
        QObject::connect(replaceAction, &QAction::triggered, m_q, &Find::onReplace);
        QObject::connect(replaceAllAction, &QAction::triggered, m_q, &Find::onReplaceAll);
        QObject::connect(m_editor, &QPlainTextEdit::selectionChanged, m_q, &Find::onSelectionChanged);
        QObject::connect(m_ui.close, &QAbstractButton::clicked, m_q, &Find::onClose);
        QObject::connect(m_ui.m_caseSensitive, &QCheckBox::checkStateChanged, m_q, &Find::onFindFlagsChanged);
        QObject::connect(m_ui.m_wholeWord, &QCheckBox::checkStateChanged, m_q, &Find::onFindFlagsChanged);
    }

    void setState()
    {
        QColor c = m_textColor;

        if (!m_editor->foundHighlighted()) {
            c = Qt::red;
        }

        m_ui.findNextBtn->setEnabled(m_editor->foundHighlighted());
        m_ui.findPrevBtn->setEnabled(m_editor->foundHighlighted());
        m_ui.findNextBtn->defaultAction()->setEnabled(m_editor->foundHighlighted());
        m_ui.findPrevBtn->defaultAction()->setEnabled(m_editor->foundHighlighted());

        QPalette palette = m_ui.findEdit->palette();
        palette.setColor(QPalette::Text, c);
        m_ui.findEdit->setPalette(palette);
    }

    Find *m_q = nullptr;
    Editor *m_editor = nullptr;
    MainWindow *m_window = nullptr;
    QColor m_textColor;
    Ui::Find m_ui;
}; // struct FindPrivate

//
// Find
//

Find::Find(MainWindow *window, Editor *editor, QWidget *parent)
    : QFrame(parent)
    , m_d(new FindPrivate(window, editor, this))
{
    m_d->initUi();
}

Find::~Find()
{
}

QLineEdit *Find::editLine() const
{
    return m_d->m_ui.findEdit;
}

QLineEdit *Find::replaceLine() const
{
    return m_d->m_ui.replaceEdit;
}

bool Find::isCaseSensitive() const
{
    return m_d->m_ui.m_caseSensitive->isChecked();
}

bool Find::isWholeWord() const
{
    return m_d->m_ui.m_wholeWord->isChecked();
}

QTextDocument::FindFlags Find::findFlags() const
{
    QTextDocument::FindFlags flags;

    if (isCaseSensitive()) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    if (isWholeWord()) {
        flags |= QTextDocument::FindWholeWords;
    }

    return flags;
}

void Find::onFindTextChanged(const QString &)
{
    m_d->m_editor->highlight(m_d->m_ui.findEdit->text(), true, findFlags());

    m_d->setState();
}

void Find::setFindText(const QString &text)
{
    m_d->m_ui.findEdit->setText(text);

    setFocusOnFind();

    onSelectionChanged();
}

void Find::setFocusOnFind()
{
    m_d->m_ui.findEdit->setFocus();
    m_d->m_ui.findEdit->selectAll();

    m_d->m_editor->highlight(m_d->m_ui.findEdit->text(), true, findFlags());
}

void Find::setCaseSensitive(bool on)
{
    m_d->m_ui.m_caseSensitive->setChecked(on);
}

void Find::setWholeWord(bool on)
{
    m_d->m_ui.m_wholeWord->setChecked(on);
}

void Find::onReplace()
{
    m_d->m_editor->replaceCurrent(m_d->m_ui.replaceEdit->text());

    onFindTextChanged({});

    onSelectionChanged();
}

void Find::onReplaceAll()
{
    m_d->m_editor->replaceAll(m_d->m_ui.replaceEdit->text());

    onFindTextChanged({});

    onSelectionChanged();
}

void Find::onSelectionChanged()
{
    m_d->m_ui.replaceBtn->setEnabled(m_d->m_editor->foundSelected());
    m_d->m_ui.replaceAllBtn->setEnabled(m_d->m_editor->foundHighlighted());
}

void Find::onClose()
{
    hide();

    m_d->m_window->onToolHide();
}

void Find::onEditorReady()
{
    m_d->setState();

    onSelectionChanged();
}

void Find::onFindFlagsChanged(Qt::CheckState)
{
    m_d->m_editor->highlight(m_d->m_ui.findEdit->text(), true, findFlags());

    m_d->setState();
}

} /* namespace MdEditor */
