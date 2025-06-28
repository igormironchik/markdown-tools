/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "previewpage.h"

// Qt include.
#include <QDesktopServices>
#include <QWebEngineSettings>

namespace MdEditor
{

PreviewPage::PreviewPage(QObject *parent)
    : QWebEnginePage(parent)
{
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    settings()->setAttribute(QWebEngineSettings::LinksIncludedInFocusChain, false);
}

bool PreviewPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType /*type*/, bool /*isMainFrame*/)
{
    if (url.scheme() == QStringLiteral("qrc") || url.scheme() == QStringLiteral("data")) {
        return true;
    }

    QDesktopServices::openUrl(url);

    return false;
}

} /* namespace MdEditor */
