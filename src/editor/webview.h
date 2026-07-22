/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QScopedPointer>
#include <QWebEngineView>

namespace MdEditor
{

//
// WebView
//

struct WebViewPrivate;

//! HTML preview.
class WebView : public QWebEngineView
{
    Q_OBJECT

Q_SIGNALS:
    void scrollRequested(const QString &id);

public:
    explicit WebView(QWidget *parent);
    ~WebView() override;

public Q_SLOTS:
    void enableScrollEditor(bool on = true);

private Q_SLOTS:
    void onSelectionChanged();
    void onCopy();
    void onScroll();

private:
    friend class WebViewPrivate;

    Q_DISABLE_COPY(WebView)

    QScopedPointer<WebViewPrivate> m_d;
}; // class WebView

} /* namespace MdEditor */
