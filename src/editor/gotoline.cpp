/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "gotoline.h"
#include "editor.h"
#include "mainwindow.h"
#include "ui_gotoline.h"

// Qt include.
#include <QIntValidator>

namespace MdEditor
{

//
// GoToLinePrivate
//

struct GoToLinePrivate {
    GoToLinePrivate(MainWindow *w,
                    Editor *e,
                    GoToLine *parent)
        : m_q(parent)
        , m_editor(e)
        , m_window(w)
    {
    }

    void initUi()
    {
        m_ui.setupUi(m_q);

        auto intValidator = new QIntValidator(1, 999999, m_ui.line);
        m_ui.line->setValidator(intValidator);

        QObject::connect(m_ui.line, &QLineEdit::returnPressed, m_q, &GoToLine::onEditingFinished);

        QObject::connect(m_ui.close, &QAbstractButton::clicked, m_q, &GoToLine::onClose);
    }

    GoToLine *m_q = nullptr;
    Editor *m_editor = nullptr;
    MainWindow *m_window = nullptr;
    Ui::GoToLine m_ui;
}; // struct FindPrivate

//
// GoToLine
//

GoToLine::GoToLine(MainWindow *window,
                   Editor *editor,
                   QWidget *parent)
    : QFrame(parent)
    , m_d(new GoToLinePrivate(window,
                              editor,
                              this))
{
    m_d->initUi();
}

GoToLine::~GoToLine()
{
}

QLineEdit *GoToLine::line() const
{
    return m_d->m_ui.line;
}

void GoToLine::setFocusOnLine()
{
    m_d->m_ui.line->setFocus();
    m_d->m_ui.line->selectAll();
}

void GoToLine::onEditingFinished()
{
    m_d->m_editor->goToLine(m_d->m_ui.line->text().toInt());

    hide();
}

void GoToLine::onClose()
{
    hide();

    m_d->m_window->onToolHide();
}

} /* namespace MdEditor */
