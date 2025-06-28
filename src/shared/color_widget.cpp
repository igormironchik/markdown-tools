/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "color_widget.h"

// Qt include.
#include <QMouseEvent>
#include <QPainter>

namespace MdShared
{

//
// ColorWidget
//

ColorWidget::ColorWidget(QWidget *parent)
    : QFrame(parent)
    , m_pressed(false)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setColor(Qt::white);
}

const QColor &ColorWidget::color() const
{
    return m_color;
}

void ColorWidget::setColor(const QColor &c)
{
    m_color = c;

    update();
}

void ColorWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    p.setPen(m_color);
    p.setBrush(m_color);
    p.drawRect(frameRect());

    QFrame::paintEvent(e);
}

void ColorWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_pressed = true;
    }

    e->accept();
}

void ColorWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_pressed && e->button() == Qt::LeftButton) {
        m_pressed = false;

        emit clicked();
    }

    e->accept();
}

} /* namespace MdShared */
