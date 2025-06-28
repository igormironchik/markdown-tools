/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "closebutton.h"

// Qt include.
#include <QEnterEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>

namespace MdEditor
{

//
// CloseButtonPrivate
//

struct CloseButtonPrivate {
    CloseButtonPrivate(CloseButton *parent)
        : m_q(parent)
    {
    }

    void initUi()
    {
        m_q->setCheckable(false);

        m_activePixmap = QPixmap(QStringLiteral(":/res/img/dialog-close.png"));

        auto source = m_activePixmap.toImage();
        QImage target = QImage(source.width(), source.height(), QImage::Format_ARGB32);

        for (int x = 0; x < source.width(); ++x) {
            for (int y = 0; y < source.height(); ++y) {
                const auto g = qGray(source.pixel(x, y));
                target.setPixelColor(x, y, QColor(g, g, g, source.pixelColor(x, y).alpha()));
            }
        }

        m_inactivePixmap = QPixmap::fromImage(target);

        m_q->setFocusPolicy(Qt::NoFocus);
    }

    CloseButton *m_q = nullptr;
    bool m_hovered = false;
    QPixmap m_activePixmap;
    QPixmap m_inactivePixmap;
}; // struct CloseButtonPrivate

//
// CloseButton
//

CloseButton::CloseButton(QWidget *parent)
    : QAbstractButton(parent)
    , m_d(new CloseButtonPrivate(this))
{
    m_d->initUi();
}

CloseButton::~CloseButton()
{
}

QSize CloseButton::sizeHint() const
{
    return {16, 16};
}

void CloseButton::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    if (m_d->m_hovered) {
        p.drawPixmap(rect(), m_d->m_activePixmap);
    } else {
        p.drawPixmap(rect(), m_d->m_inactivePixmap);
    }
}

void CloseButton::enterEvent(QEnterEvent *event)
{
    m_d->m_hovered = true;

    update();

    event->accept();
}

void CloseButton::leaveEvent(QEvent *event)
{
    m_d->m_hovered = false;

    update();

    event->accept();
}

} /* namespace MdEditor */
