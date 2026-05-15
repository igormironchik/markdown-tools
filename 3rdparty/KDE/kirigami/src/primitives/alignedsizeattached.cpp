/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "alignedsizeattached.h"
#include <QDebug>
#include <QQuickItem>

AlignedSizeAttached::AlignedSizeAttached(QObject *parent)
    : QObject(parent)
{
    m_item = qobject_cast<QQuickItem *>(parent);

    if (!m_item) {
        qWarning() << "AlignedSizeAttached attached to" << parent << "Attaching to a non QQuickItem derived object won't work";
        return;
    }

    connect(m_item, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        if (m_window) {
            m_window->removeEventFilter(this);
        }
        m_window = qobject_cast<QQuickWindow *>(m_item->window());
        if (!m_window) {
            return;
        }
        m_window->installEventFilter(this);
        dprChanged();
    });
    m_window = qobject_cast<QQuickWindow *>(m_item->window());
    if (!m_window) {
        return;
    }
    dprChanged();
    m_window->installEventFilter(this);
}

AlignedSizeAttached::~AlignedSizeAttached()
{
}

qreal AlignedSizeAttached::width() const
{
    return m_width;
}

void AlignedSizeAttached::setWidth(qreal width)
{
    if (width == m_width) {
        return;
    }

    m_width = width;

    if (m_width >= 0) {
        m_item->setWidth(alignedWidth());
    }

    Q_EMIT widthChanged();
}

void AlignedSizeAttached::resetWidth()
{
    m_width = -1.0;
    Q_EMIT widthChanged();
}

qreal AlignedSizeAttached::height() const
{
    return m_height;
}

void AlignedSizeAttached::setHeight(qreal height)
{
    if (height == m_height) {
        return;
    }

    m_height = height;

    if (m_height >= 0) {
        m_item->setHeight(alignedHeight());
    }

    Q_EMIT heightChanged();
}

void AlignedSizeAttached::resetHeight()
{
    m_height = -1.0;
    Q_EMIT heightChanged();
}

qreal AlignedSizeAttached::alignedWidth() const
{
    return std::round(m_width * m_dpr) / m_dpr;
}

qreal AlignedSizeAttached::alignedHeight() const
{
    return std::round(m_height * m_dpr) / m_dpr;
}

void AlignedSizeAttached::dprChanged()
{
    m_dpr = m_window->effectiveDevicePixelRatio();
    if (m_width >= 0) {
        m_item->setWidth(alignedWidth());
    }
    if (m_height >= 0) {
        m_item->setHeight(alignedHeight());
    }
}

bool AlignedSizeAttached::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::DevicePixelRatioChange) {
        dprChanged();
    }
    return QObject::eventFilter(watched, event);
}

AlignedSizeAttached *AlignedSizeAttached::qmlAttachedProperties(QObject *object)
{
    return new AlignedSizeAttached(object);
}

#include "moc_alignedsizeattached.cpp"
