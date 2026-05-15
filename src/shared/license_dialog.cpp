/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "license_dialog.h"
#include "ui_license_dialog.h"

// Qt include.
#include <QAbstractButton>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QScrollArea>
#include <QScrollBar>

namespace MdShared
{

namespace /* anonymous */
{

//
// Anchor
//

//! A small "flag" on left side - anchor for a license.
class Anchor : public QAbstractButton
{
public:
    Anchor(QWidget *parent,
           const QString &name,
           QScrollArea *scroll)
        : QAbstractButton(parent)
        , m_scroll(scroll)
        , m_highlighted(false)
    {
        setToolTip(name);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMinimumSize(20, 10);
    }

    void highlight(bool on)
    {
        m_highlighted = on;

        update();
    }

    QScrollArea *widget() const
    {
        return m_scroll;
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        p.setPen(
            QPen(m_highlighted ? palette().color(QPalette::LinkVisited) : palette().color(QPalette::Highlight), 2.0));
        p.setBrush(palette().color(QPalette::Button));

        QPainterPath pt;
        pt.moveTo(1, 1);
        pt.lineTo(width() - 1, 1);
        pt.lineTo(width() - height() / 2, height() / 2);
        pt.lineTo(width() - 1, height() - 1);
        pt.lineTo(1, height() - 1);
        pt.lineTo(1, 1);

        p.drawPath(pt);
    }

private:
    QScrollArea *m_scroll;
    bool m_highlighted;
}; // class Anchor

} /* namespace anonymous */

//
// LicenseDialogPrivate
//

class LicenseDialogPrivate
{
public:
    LicenseDialogPrivate()
    {
    }

    void init(LicenseDialog *q)
    {
        m_aw = new QWidget(q);
        m_al = new QVBoxLayout(m_aw);
        m_al->setContentsMargins(2, 2, 2, 2);
        m_al->setSpacing(2);
        m_al->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

        m_ui.setupUi(q);

        m_ui.m_anchors->setWidget(m_aw);
        m_ui.m_anchors->setMinimumWidth(20 + 4 + m_ui.m_anchors->verticalScrollBar()->sizeHint().width());
    }

    void clearHighlighting()
    {
        for (const auto &a : std::as_const(m_anchors)) {
            a->highlight(false);
        }
    }

    //! Ui.
    Ui::LicenseDialog m_ui;
    //! Anchors widget.
    QWidget *m_aw = nullptr;
    //! Anchors layout.
    QVBoxLayout *m_al = nullptr;
    //! Anchros.
    QVector<Anchor *> m_anchors;
}; // class LicenseDialogPrivate

//
// LicenseDialog
//

LicenseDialog::LicenseDialog(QWidget *parent)
    : QDialog(parent)
    , d(new LicenseDialogPrivate)
{
    d->init(this);
}

LicenseDialog::~LicenseDialog()
{
}

void LicenseDialog::addLicense(const QString &title,
                               const QString &license)
{
    QScrollArea *scroll = new QScrollArea(d->m_ui.m_stack);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QLabel *label = new QLabel(d->m_ui.m_stack);
    label->setText(license);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse
                                   | Qt::TextSelectableByKeyboard
                                   | Qt::LinksAccessibleByMouse
                                   | Qt::LinksAccessibleByKeyboard);
    label->setOpenExternalLinks(true);

    scroll->setWidget(label);

    Anchor *a = new Anchor(d->m_aw, title, scroll);
    d->m_al->insertWidget(d->m_al->count() - 1, a);
    d->m_anchors.push_back(a);

    connect(a, &Anchor::clicked, this, &LicenseDialog::anchorClicked);

    d->m_ui.m_stack->addWidget(scroll);

    if (d->m_ui.m_stack->count() == 1) {
        a->highlight(true);
        d->m_ui.m_stack->setCurrentIndex(0);
    }
}

void LicenseDialog::anchorClicked()
{
    d->clearHighlighting();

    auto a = static_cast<Anchor *>(sender());

    a->highlight(true);

    d->m_ui.m_stack->setCurrentWidget(a->widget());
}

} /* namespace MdShared */
