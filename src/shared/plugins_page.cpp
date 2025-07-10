/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "plugins_page.h"
#include "ui_plugins_page.h"

namespace MdShared
{

//
// PluginsPagePrivate
//

class PluginsPagePrivate
{
public:
    PluginsPagePrivate()
    {
    }

    void init(PluginsPage *q)
    {
        m_ui.setupUi(q);

        m_ui.m_supChar->setText(QStringLiteral("^"));
        m_ui.m_subChar->setText(QStringLiteral("-"));
        m_ui.m_markChar->setText(QStringLiteral("="));
    }

    //! Ui.
    Ui::PluginsPage m_ui;
}; // class PluginsPagePrivate

//
// PluginsPage
//

PluginsPage::PluginsPage(QWidget *parent)
    : QWidget(parent)
    , d(new PluginsPagePrivate)
{
    d->init(this);
}

PluginsPage::~PluginsPage()
{
}

} /* namespace MdShared */
