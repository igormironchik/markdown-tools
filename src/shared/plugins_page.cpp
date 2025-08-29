/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
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

        QObject::connect(m_ui.m_supSwitch, &MdShared::Switch::stateChanged, q, &PluginsPage::onButtonStateChanged);
        QObject::connect(m_ui.m_subSwitch, &MdShared::Switch::stateChanged, q, &PluginsPage::onButtonStateChanged);
        QObject::connect(m_ui.m_markSwitch, &MdShared::Switch::stateChanged, q, &PluginsPage::onButtonStateChanged);
        QObject::connect(m_ui.m_yamlSwitch, &MdShared::Switch::stateChanged, q, &PluginsPage::onButtonStateChanged);
        QObject::connect(m_ui.m_supChar, &QLineEdit::textChanged, q, &PluginsPage::onSupDelimChanged);
        QObject::connect(m_ui.m_subChar, &QLineEdit::textChanged, q, &PluginsPage::onSubDelimChanged);
        QObject::connect(m_ui.m_markChar, &QLineEdit::textChanged, q, &PluginsPage::onMarkDelimChanged);
    }

    bool isOk(QLineEdit *e,
              MdShared::Switch *s = nullptr)
    {
        const auto text = e->text().simplified();
        bool ok = true;

        if (text.isEmpty()) {
            ok = false;

            if (s) {
                s->setState(MdShared::Switch::AcceptedUncheck);
            }
        }

        return ok;
    }

    bool isOk()
    {
        return (isOk(m_ui.m_supChar) && isOk(m_ui.m_subChar) && isOk(m_ui.m_markChar));
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

void PluginsPage::setCfg(const PluginsCfg &cfg)
{
    if (!cfg.m_sup.m_delimiter.isNull()) {
        d->m_ui.m_supChar->setText(cfg.m_sup.m_delimiter);
    }

    d->m_ui.m_supSwitch->setState(cfg.m_sup.m_on ? MdShared::Switch::AcceptedCheck : MdShared::Switch::AcceptedUncheck);

    if (!cfg.m_sub.m_delimiter.isNull()) {
        d->m_ui.m_subChar->setText(cfg.m_sub.m_delimiter);
    }

    d->m_ui.m_subSwitch->setState(cfg.m_sub.m_on ? MdShared::Switch::AcceptedCheck : MdShared::Switch::AcceptedUncheck);

    if (!cfg.m_mark.m_delimiter.isNull()) {
        d->m_ui.m_markChar->setText(cfg.m_mark.m_delimiter);
    }

    d->m_ui.m_markSwitch->setState(cfg.m_mark.m_on ? MdShared::Switch::AcceptedCheck
                                                   : MdShared::Switch::AcceptedUncheck);

    d->m_ui.m_yamlSwitch->setState(cfg.m_yamlEnabled ? MdShared::Switch::AcceptedCheck
                                                     : MdShared::Switch::AcceptedUncheck);

    d->isOk(d->m_ui.m_supChar, d->m_ui.m_supSwitch);
    d->isOk(d->m_ui.m_subChar, d->m_ui.m_subSwitch);
    d->isOk(d->m_ui.m_markChar, d->m_ui.m_markSwitch);
}

PluginsCfg PluginsPage::cfg() const
{
    if (!d->isOk()) {
        return {};
    }

    PluginsCfg c;

    const auto sup = d->m_ui.m_supChar->text().simplified();

    if (!sup.isEmpty()) {
        c.m_sup.m_delimiter = sup[0];
        c.m_sup.m_on = d->m_ui.m_supSwitch->isChecked();
    } else {
        c.m_sup.m_delimiter = QChar();
        c.m_sup.m_on = false;
    }

    const auto sub = d->m_ui.m_subChar->text().simplified();

    if (!sub.isEmpty()) {
        c.m_sub.m_delimiter = sub[0];
        c.m_sub.m_on = d->m_ui.m_subSwitch->isChecked();
    } else {
        c.m_sub.m_delimiter = QChar();
        c.m_sub.m_on = false;
    }

    const auto mark = d->m_ui.m_markChar->text().simplified();

    if (!mark.isEmpty()) {
        c.m_mark.m_delimiter = mark[0];
        c.m_mark.m_on = d->m_ui.m_markSwitch->isChecked();
    } else {
        c.m_mark.m_delimiter = QChar();
        c.m_mark.m_on = false;
    }

    c.m_yamlEnabled = d->m_ui.m_yamlSwitch->isChecked();

    return c;
}

void PluginsPage::onButtonStateChanged(int st)
{
    auto btn = static_cast<MdShared::Switch *>(sender());

    switch (st) {
    case MdShared::Switch::NotAcceptedCheck:
        btn->setState(MdShared::Switch::AcceptedCheck);
        break;

    case MdShared::Switch::NotAcceptedUncheck:
        btn->setState(MdShared::Switch::AcceptedUncheck);
        break;

    default:
        break;
    }
}

void PluginsPage::onSupDelimChanged(const QString &)
{
    d->isOk(d->m_ui.m_supChar, d->m_ui.m_supSwitch);
}

void PluginsPage::onSubDelimChanged(const QString &)
{
    d->isOk(d->m_ui.m_subChar, d->m_ui.m_subSwitch);
}

void PluginsPage::onMarkDelimChanged(const QString &)
{
    d->isOk(d->m_ui.m_markChar, d->m_ui.m_markSwitch);
}

} /* namespace MdShared */
