/*
 * SPDX-FileCopyrightText: 2024 Xaver Hugl <xaver.hugl@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>

namespace KDecoration3
{

/**
 * @returns the pixel size with the provided scale
 * @since 6.3
 */
inline qreal pixelSize(qreal scale)
{
    return 1.0 / scale;
}

/**
 * snaps the logical geometry value to a fractional logical geometry value
 * that aligns to the pixel grid of the provided scale
 * @since 6.3
 */
inline qreal snapToPixelGrid(qreal value, qreal scale)
{
    return std::round(value * scale) / scale;
}

/**
 * snaps the logical geometry value to a fractional logical geometry value
 * that aligns to the pixel grid of the provided scale
 * @since 6.3
 */
inline QPointF snapToPixelGrid(const QPointF &value, qreal scale)
{
    return QPointF(snapToPixelGrid(value.x(), scale), snapToPixelGrid(value.y(), scale));
}

/**
 * snaps the logical geometry value to a fractional logical geometry value
 * that aligns to the pixel grid of the provided scale
 * @since 6.3
 */
inline QSizeF snapToPixelGrid(const QSizeF &value, qreal scale)
{
    return QSizeF(snapToPixelGrid(value.width(), scale), snapToPixelGrid(value.height(), scale));
}

/**
 * snaps the logical geometry value to a fractional logical geometry value
 * that aligns to the pixel grid of the provided scale
 * @since 6.3
 */
inline QRectF snapToPixelGrid(const QRectF &value, qreal scale)
{
    return QRectF(snapToPixelGrid(value.topLeft(), scale), snapToPixelGrid(value.bottomRight(), scale));
}

}
