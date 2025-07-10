/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QWidget>

namespace MdShared
{

//
// PluginsPage
//

class PluginsPagePrivate;

//! Page with plugins settings.
class PluginsPage : public QWidget
{
    Q_OBJECT

public:
    explicit PluginsPage(QWidget *parent = nullptr);
    ~PluginsPage() override;

private:
    QScopedPointer<PluginsPagePrivate> d;

    Q_DISABLE_COPY(PluginsPage)
}; // class PluginsPage

} /* namespace MdShared */
