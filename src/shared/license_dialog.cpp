/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "license_dialog.hpp"
#include "ui_license_dialog.h"

// Qt include.
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QScrollArea>
#include <QScrollBar>
#include <QShowEvent>

namespace MdShared
{

namespace /* anonymous */
{

//
// Anchor
//

class Anchor : public QWidget
{
public:
    Anchor(QWidget *parent, const QString &name, QLabel *label, QScrollArea *scroll)
        : QWidget(parent)
        , m_label(label)
        , m_scroll(scroll)
        , m_pressed(false)
        , m_highlighted(false)
    {
        setToolTip(name);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMinimumSize(20, 10);
    }

    QRect labelRect() const
    {
        auto r = m_label->rect();
        r.moveTo(m_label->pos());

        return r;
    }

    void highlight(bool on)
    {
        m_highlighted = on;

        update();
    }

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton)
            m_pressed = true;

        e->accept();
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (m_pressed && e->button() == Qt::LeftButton) {
            m_pressed = false;

            m_scroll->verticalScrollBar()->setSliderPosition(m_label->y());
        }

        e->accept();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        p.setPen(QPen(m_highlighted ? palette().color(QPalette::LinkVisited) : palette().color(QPalette::Highlight), 2.0));
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
    QLabel *m_label;
    QScrollArea *m_scroll;
    bool m_pressed;
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
        : m_w(nullptr)
        , m_l(nullptr)
        , m_aw(nullptr)
        , m_al(nullptr)
    {
    }

    void highlightAnchor(int value)
    {
        const auto vh = this->m_ui.m_scrollArea->viewport()->height();
        const auto vr = QRect(0, value, this->m_ui.m_scrollArea->viewport()->width(), vh);

        for (const auto &a : std::as_const(this->m_anchors)) {
            const auto ir = vr.intersected(a->labelRect());

            a->highlight(ir.height() >= qRound((double)vh / 2.5) && !ir.isNull());
        }
    }

    void init(LicenseDialog *q)
    {
        m_w = new QWidget(q);
        m_l = new QVBoxLayout(m_w);
        m_aw = new QWidget(q);
        m_al = new QVBoxLayout(m_aw);
        m_al->setContentsMargins(2, 2, 2, 2);
        m_al->setSpacing(2);
        m_al->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

        m_ui.setupUi(q);

        m_ui.m_scrollArea->setWidget(m_w);
        m_ui.m_anchors->setWidget(m_aw);
        m_ui.m_anchors->setMinimumWidth(20 + 4 + m_ui.m_anchors->verticalScrollBar()->sizeHint().width());

        QObject::connect(m_ui.m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, [this](auto value) {
            this->highlightAnchor(value);
        });
    }

    //! Ui.
    Ui::LicenseDialog m_ui;
    //! Widget.
    QWidget *m_w;
    //! Layout.
    QVBoxLayout *m_l;
    //! Anchors widget.
    QWidget *m_aw;
    //! Anchors layout.
    QVBoxLayout *m_al;
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

void LicenseDialog::addLicense(const QString &title, const QString &license)
{
    const bool first = (d->m_l->count() == 0);

    if (!first) {
        QFrame *line = new QFrame(d->m_w);
        line->setFrameStyle(QFrame::HLine | QFrame::Plain);
        d->m_l->addWidget(line);
    }

    QLabel *label = new QLabel(d->m_w);
    label->setText(license);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard |
                                   Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    label->setOpenExternalLinks(true);

    d->m_l->addWidget(label);

    Anchor *a = new Anchor(d->m_aw, title, label, d->m_ui.m_scrollArea);
    d->m_al->insertWidget(d->m_al->count() - 1, a);
    d->m_anchors.push_back(a);

    d->highlightAnchor(d->m_ui.m_scrollArea->verticalScrollBar()->value());
}

void LicenseDialog::showEvent(QShowEvent *e)
{
    d->highlightAnchor(d->m_ui.m_scrollArea->verticalScrollBar()->value());

    e->accept();
}

} /* namespace MdShared */
