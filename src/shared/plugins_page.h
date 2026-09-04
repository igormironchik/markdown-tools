/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// md-shared include.
#include "ui_plugins_page.h"

// Qt include.
#include <QWidget>

namespace MdPdf
{

struct PluginsCfg;

}

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

    //! Set configuration.
    void setCfg(const MdPdf::PluginsCfg &cfg);
    //! \return Configuration.
    MdPdf::PluginsCfg cfg() const;

    //! \return Ui.
    Ui::PluginsPage &ui();

private Q_SLOTS:
    void onButtonStateChanged(int st);
    void onSupDelimChanged(const QString &);
    void onSubDelimChanged(const QString &);
    void onMarkDelimChanged(const QString &);

private:
    friend class PluginsPagePrivate;

    QScopedPointer<PluginsPagePrivate> d;

    Q_DISABLE_COPY(PluginsPage)
}; // class PluginsPage

} /* namespace MdShared */
