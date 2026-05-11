/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decorationshadow.h"
#include "decorationshadow_p.h"

namespace KDecoration3
{
DecorationShadow::Private::Private(DecorationShadow *parent)
    : q(parent)
{
}

DecorationShadow::Private::~Private() = default;

DecorationShadow::DecorationShadow()
    : QObject()
    , d(new Private(this))
{
}

DecorationShadow::~DecorationShadow() = default;

QRectF DecorationShadow::topLeftGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(0, 0, d->innerShadowRect.left(), d->innerShadowRect.top());
}

QRectF DecorationShadow::topGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(d->innerShadowRect.left(), 0, d->innerShadowRect.width(), d->innerShadowRect.top());
}

QRectF DecorationShadow::topRightGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(d->innerShadowRect.left() + d->innerShadowRect.width(),
                  0,
                  d->shadow.width() - d->innerShadowRect.width() - d->innerShadowRect.left(),
                  d->innerShadowRect.top());
}

QRectF DecorationShadow::rightGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(d->innerShadowRect.left() + d->innerShadowRect.width(),
                  d->innerShadowRect.top(),
                  d->shadow.width() - d->innerShadowRect.width() - d->innerShadowRect.left(),
                  d->innerShadowRect.height());
}

QRectF DecorationShadow::bottomRightGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(d->innerShadowRect.left() + d->innerShadowRect.width(),
                  d->innerShadowRect.top() + d->innerShadowRect.height(),
                  d->shadow.width() - d->innerShadowRect.width() - d->innerShadowRect.left(),
                  d->shadow.height() - d->innerShadowRect.top() - d->innerShadowRect.height());
}

QRectF DecorationShadow::bottomGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(d->innerShadowRect.left(),
                  d->innerShadowRect.top() + d->innerShadowRect.height(),
                  d->innerShadowRect.width(),
                  d->shadow.height() - d->innerShadowRect.top() - d->innerShadowRect.height());
}

QRectF DecorationShadow::bottomLeftGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(0,
                  d->innerShadowRect.top() + d->innerShadowRect.height(),
                  d->innerShadowRect.left(),
                  d->shadow.height() - d->innerShadowRect.top() - d->innerShadowRect.height());
}

QRectF DecorationShadow::leftGeometry() const
{
    if (d->innerShadowRect.isNull() || d->shadow.isNull()) {
        return QRectF();
    }
    return QRectF(0, d->innerShadowRect.top(), d->innerShadowRect.left(), d->innerShadowRect.height());
}

#ifndef K_DOXYGEN

QImage DecorationShadow::shadow() const
{
    return d->shadow;
}

QMarginsF DecorationShadow::padding() const
{
    return d->padding;
}

QRectF DecorationShadow::innerShadowRect() const
{
    return d->innerShadowRect;
}

qreal DecorationShadow::paddingTop() const
{
    return d->padding.top();
}

qreal DecorationShadow::paddingBottom() const
{
    return d->padding.bottom();
}

qreal DecorationShadow::paddingRight() const
{
    return d->padding.right();
}

qreal DecorationShadow::paddingLeft() const
{
    return d->padding.left();
}

void DecorationShadow::setShadow(const QImage &shadow)
{
    if (d->shadow == shadow) {
        return;
    }
    d->shadow = shadow;
    Q_EMIT shadowChanged(d->shadow);
}

#endif

void DecorationShadow::setPadding(const QMarginsF &margins)
{
    if (d->padding == margins) {
        return;
    }
    d->padding = margins;
    Q_EMIT paddingChanged();
}

void DecorationShadow::setInnerShadowRect(const QRectF &rect)
{
    if (d->innerShadowRect == rect) {
        return;
    }
    d->innerShadowRect = rect;
    Q_EMIT innerShadowRectChanged();
}

}

#include "moc_decorationshadow.cpp"
