/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "findweb.h"
#include "mainwindow.h"
#include "ui_findweb.h"
#include "webview.h"

// Qt include.
#include <QPalette>
#include <QWebEngineFindTextResult>

namespace MdEditor
{

//
// FindWebPrivate
//

struct FindWebPrivate {
    FindWebPrivate(MainWindow *w,
                   WebView *wb,
                   FindWeb *parent)
        : m_q(parent)
        , m_web(wb)
        , m_window(w)
    {
    }

    void initUi()
    {
        m_ui.setupUi(m_q);

        QObject::connect(m_ui.findEdit, &QLineEdit::textChanged, m_q, &FindWeb::onFindWebTextChanged);

        auto findPrevAction = new QAction(FindWeb::tr("Find Previous"), m_q);
        findPrevAction->setShortcutContext(Qt::ApplicationShortcut);
        findPrevAction->setShortcut(FindWeb::tr("Shift+F4"));
        findPrevAction->setToolTip(FindWeb::tr("Find Previous <small>Shift+F4</small>"));
        m_ui.findPrevBtn->setDefaultAction(findPrevAction);
        m_ui.findPrevBtn->setEnabled(false);

        auto findNextAction = new QAction(FindWeb::tr("Find Next"), m_q);
        findNextAction->setShortcutContext(Qt::ApplicationShortcut);
        findNextAction->setShortcut(FindWeb::tr("F4"));
        findNextAction->setToolTip(FindWeb::tr("Find Next <small>F4</small>"));
        m_ui.findNextBtn->setDefaultAction(findNextAction);
        m_ui.findNextBtn->setEnabled(false);

        m_textColor = m_ui.findEdit->palette().color(QPalette::Text);

        QObject::connect(findPrevAction, &QAction::triggered, m_q, &FindWeb::onFindPrev);
        QObject::connect(findNextAction, &QAction::triggered, m_q, &FindWeb::onFindNext);
        QObject::connect(m_ui.close, &QAbstractButton::clicked, m_q, &FindWeb::onClose);
    }

    FindWeb *m_q = nullptr;
    WebView *m_web = nullptr;
    MainWindow *m_window = nullptr;
    QColor m_textColor;
    Ui::FindWeb m_ui;
}; // struct FindWebPrivate

//
// FindWeb
//

FindWeb::FindWeb(MainWindow *window,
                 WebView *web,
                 QWidget *parent)
    : QFrame(parent)
    , m_d(new FindWebPrivate(window,
                             web,
                             this))
{
    m_d->initUi();
}

FindWeb::~FindWeb()
{
}

QLineEdit *FindWeb::line() const
{
    return m_d->m_ui.findEdit;
}

void FindWeb::onFindWebTextChanged(const QString &str)
{
    auto result = [&](const QWebEngineFindTextResult &r) {
        QColor c = m_d->m_textColor;

        if (!r.numberOfMatches())
            c = Qt::red;

        m_d->m_ui.findNextBtn->setEnabled(r.numberOfMatches());
        m_d->m_ui.findPrevBtn->setEnabled(r.numberOfMatches());
        m_d->m_ui.findNextBtn->defaultAction()->setEnabled(r.numberOfMatches());
        m_d->m_ui.findPrevBtn->defaultAction()->setEnabled(r.numberOfMatches());

        QPalette palette = m_d->m_ui.findEdit->palette();
        palette.setColor(QPalette::Text, c);
        m_d->m_ui.findEdit->setPalette(palette);
    };

    m_d->m_web->findText(str, QWebEnginePage::FindCaseSensitively, result);
}

void FindWeb::setFindWebText(const QString &text)
{
    m_d->m_ui.findEdit->setText(text);

    setFocusOnFindWeb();
}

void FindWeb::setFocusOnFindWeb()
{
    m_d->m_ui.findEdit->setFocus();
    m_d->m_ui.findEdit->selectAll();
    m_d->m_web->findText(m_d->m_ui.findEdit->text(), QWebEnginePage::FindCaseSensitively);
}

void FindWeb::onClose()
{
    hide();

    m_d->m_window->onToolHide();
}

void FindWeb::onFindNext()
{
    m_d->m_web->findText(m_d->m_ui.findEdit->text(), QWebEnginePage::FindCaseSensitively);
}

void FindWeb::onFindPrev()
{
    m_d->m_web->findText(m_d->m_ui.findEdit->text(),
                         QWebEnginePage::FindBackward | QWebEnginePage::FindCaseSensitively);
}

void FindWeb::hideEvent(QHideEvent *event)
{
    m_d->m_web->findText(QString());
    m_d->m_web->setFocus();
    m_d->m_web->triggerPageAction(QWebEnginePage::Unselect, true);

    QFrame::hideEvent(event);
}

} /* namespace MdEditor */
