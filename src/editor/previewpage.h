/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWebEnginePage>

namespace MdEditor
{

//
// PreviewPage
//

//! Web preview page.
class PreviewPage : public QWebEnginePage
{
public:
    explicit PreviewPage(QObject *parent);
    ~PreviewPage() override = default;

protected:
    bool acceptNavigationRequest(const QUrl &url,
                                 NavigationType type,
                                 bool isMainFrame) override;
}; // class PreviewPage

} /* namespace MdEditor */
