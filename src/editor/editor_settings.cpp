/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "editor_settings.h"

namespace MdEditor
{

//
// EditorSettingsPage
//

EditorSettingsPage::EditorSettingsPage(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    connect(m_ui.m_rightMargin, &QCheckBox::checkStateChanged, this, &EditorSettingsPage::onEnableRightMargin);
    connect(m_ui.m_tabsMode, &QComboBox::currentIndexChanged, this, &EditorSettingsPage::onIndentModeChanged);
}

Ui::EditorSettingsPage &EditorSettingsPage::ui()
{
    return m_ui;
}

void EditorSettingsPage::onEnableRightMargin(Qt::CheckState st)
{
    m_ui.m_rightMarginValue->setEnabled(st == Qt::Checked);
}

void EditorSettingsPage::onIndentModeChanged(int index)
{
    m_ui.m_spacesAmount->setEnabled(index != 0);
}

} /* namespace MdEditor */
