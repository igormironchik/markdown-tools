/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

// md-editor include.
#include "ui_preview_settings.h"

namespace MdEditor
{

//
// PreviewSettingsPage
//

//! Settings page for preview.
class PreviewSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewSettingsPage(QWidget *parent = nullptr);
    ~PreviewSettingsPage() override = default;

    //! \return UI component.
    Ui::PreviewPage &ui();
    //! \return Should preview follow editor?
    bool shouldPreviewFollowEditor() const;

private:
    Q_DISABLE_COPY(PreviewSettingsPage)

    Ui::PreviewPage m_ui;
}; // class PreviewSettingsPage

} /* namespace MdEditor */
