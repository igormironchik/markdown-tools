/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "preview_settings.h"

namespace MdEditor
{

//
// PreviewPage
//

PreviewSettingsPage::PreviewSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

Ui::PreviewPage &PreviewSettingsPage::ui()
{
    return m_ui;
}

bool PreviewSettingsPage::shouldPreviewFollowEditor() const
{
    return m_ui.m_follow->isChecked();
}

} /* namespace MdEditor */
