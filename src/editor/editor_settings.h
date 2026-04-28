/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-editor include.
#include "ui_editor_settings.h"

// Qt include.
#include <QWidget>

namespace MdEditor
{

//
// EditorSettingsPage
//

//! Editor settings page.
class EditorSettingsPage : public QWidget
{
    Q_OBJECT

public:
    EditorSettingsPage(QWidget *parent = nullptr);
    ~EditorSettingsPage() override = default;

    Ui::EditorSettingsPage &ui();

private slots:
    void onEnableRightMargin(Qt::CheckState st);
    void onIndentModeChanged(int index);

private:
    Q_DISABLE_COPY(EditorSettingsPage)

    Ui::EditorSettingsPage m_ui;
}; // class EditorSettingsPage

} /* namespace MdEditor */
