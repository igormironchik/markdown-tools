/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "webview.h"

// Qt include.
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>

namespace MdEditor
{

//
// WebViewPrivate
//

struct WebViewPrivate {
    WebViewPrivate(WebView *parent)
        : m_q(parent)
    {
    }

    void initUi()
    {
        m_copyAction = new QAction(WebView::tr("Copy"), m_q);
        m_q->addAction(m_copyAction);
        m_copyAction->setEnabled(false);
        m_q->setContextMenuPolicy(Qt::ActionsContextMenu);
        m_q->setFocusPolicy(Qt::ClickFocus);

        QObject::connect(m_copyAction, &QAction::triggered, m_q, &WebView::onCopy);
        QObject::connect(m_q, &QWebEngineView::selectionChanged, m_q, &WebView::onSelectionChanged);
    }

    WebView *m_q;
    QAction *m_copyAction = nullptr;
}; // struct WebViewPrivate

//
// WebView
//

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
    , m_d(new WebViewPrivate(this))
{
    m_d->initUi();
}

WebView::~WebView()
{
}

void WebView::onSelectionChanged()
{
    m_d->m_copyAction->setEnabled(hasSelection());
}

void WebView::onCopy()
{
    QApplication::clipboard()->setText(selectedText());
}

} /* namespace MdEditor */
