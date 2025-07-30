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
// EmphasisPluginCfg
//

//! Configuration of emphasis plugin
struct EmphasisPluginCfg
{
    //! Delimiter.
    QChar m_delimiter;
    //! Is on?
    bool m_on = false;
}; // struct EmphasisPluginCfg

inline bool operator != (const EmphasisPluginCfg &c1, const EmphasisPluginCfg &c2)
{
    return (c1.m_delimiter != c2.m_delimiter || c1.m_on != c2.m_on);
}

//
// PluginsCfg
//

//! Configuration of plugins.
struct PluginsCfg
{
    //! Configuration of superscript plugin.
    EmphasisPluginCfg m_sup;
    //! Configuration of subscrip plugin.
    EmphasisPluginCfg m_sub;
    //! Configuration of mark plugin.
    EmphasisPluginCfg m_mark;
    //! YAML header ON/OFF.
    bool m_yamlEnabled = false;
}; // struct PluginsCfg

inline bool operator != (const PluginsCfg &c1, const PluginsCfg &c2)
{
    return (c1.m_sup != c2.m_sup || c1.m_sub != c2.m_sub || c1.m_mark != c2.m_mark
            || c1.m_yamlEnabled != c2.m_yamlEnabled);
}

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
    void setCfg(const PluginsCfg &cfg);
    //! \return Configuration.
    PluginsCfg cfg() const;

private slots:
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
