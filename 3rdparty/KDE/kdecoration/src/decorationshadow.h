/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include <kdecoration3/kdecoration3_export.h>

#include <QImage>
#include <QMargins>
#include <QObject>

namespace KDecoration3
{
class DecorationShadowPrivate;

/**
 * @brief A wrapper to define the shadow around the Decoration.
 *
 * The shadow around the Decoration should not be rendered as part of the Decoration.
 * Instead a DecorationShadow should be used. That way a backend can optimize the
 * rendering of the shadow in a better way. If the shadow were part of the Decoration
 * directly it would need to be updated when the rendering changes. By using a dedicated
 * DecorationShadow the same shadow can be shared between multiple DecoratedWindows.
 *
 * The DecorationShadow consists of a shadow QImage which is composed of multiple parts:
 * @li topLeft: rendered as it is
 * @li top: stretched in x direction
 * @li topRight: rendered as it is
 * @li right: stretched in y direction
 * @li bottomRight: rendered as it is
 * @li bottom: stretched in x direction
 * @li bottomLeft: rendered as it is
 * @li left: stretched in y direction
 *
 * The sizes of these parts is denoted in the property innerShadowRect and the layout is the
 * following:
 * #######################################
 * # topLeft #     top        # topRight #
 * #######################################
 * # left #                      # right #
 * #######################################
 * # bottomLeft #  bottom  # bottomRight #
 * #######################################
 *
 * The innerShadowRect property is a QRect of the geometry of the areas not covered by any of the
 * elements. This means that:
 * @li x/y of the rect is the same as the size of the topLeft element
 * @li width of the rect is the same as the width of the top and bottom element
 * @li height of the rect is the same as the height of the left and the right element
 * By that the actual sizes of all elements can be derived out of the size of the shadow image
 * and the innerShadowRect.
 *
 * The position of the rendering depends on the values;
 * @li paddingTop
 * @li paddingRight
 * @li paddingBottom
 * @li paddingLeft
 *
 * The top left element is rendered with an offset of paddingLeft and paddingTop.
 * The non-stretched elements are rendered in the size as specified, the area
 * between two non-stretched elements (e.g. between topLeft and topRight) is filled
 * by the element with one direction stretched and the other direction fixed at the
 * corresponding padding value. E.g. the top element is stretched in x direction and
 * fixed at paddingTop value. If stretching the side elements is not wanted one needs
 * to provide a shadow image with those elements at a size that stretching is not
 * required.
 *
 * If the padding values are smaller than the sizes of the shadow elements the shadow
 * will overlap with the Decoration and be rendered behind the Decoration.
 *
 **/
class KDECORATIONS3_EXPORT DecorationShadow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QImage shadow READ shadow WRITE setShadow NOTIFY shadowChanged)
    Q_PROPERTY(QRectF innerShadowRect READ innerShadowRect WRITE setInnerShadowRect NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF topLeftGeometry READ topLeftGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF topGeometry READ topGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF topRightGeometry READ topRightGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF rightGeometry READ rightGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF bottomRightGeometry READ bottomRightGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF bottomGeometry READ bottomGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF bottomLeftGeometry READ bottomLeftGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(QRectF leftGeometry READ leftGeometry NOTIFY innerShadowRectChanged)
    Q_PROPERTY(qreal paddingTop READ paddingTop NOTIFY paddingChanged)
    Q_PROPERTY(qreal paddingRight READ paddingRight NOTIFY paddingChanged)
    Q_PROPERTY(qreal paddingBottom READ paddingBottom NOTIFY paddingChanged)
    Q_PROPERTY(qreal paddingLeft READ paddingLeft NOTIFY paddingChanged)
    Q_PROPERTY(QMarginsF padding READ padding WRITE setPadding NOTIFY paddingChanged)
public:
    explicit DecorationShadow();
    ~DecorationShadow() override;

    QImage shadow() const;
    QRectF innerShadowRect() const;
    QRectF topLeftGeometry() const;
    QRectF topGeometry() const;
    QRectF topRightGeometry() const;
    QRectF rightGeometry() const;
    QRectF bottomRightGeometry() const;
    QRectF bottomGeometry() const;
    QRectF bottomLeftGeometry() const;
    QRectF leftGeometry() const;
    qreal paddingTop() const;
    qreal paddingRight() const;
    qreal paddingBottom() const;
    qreal paddingLeft() const;
    QMarginsF padding() const;

    void setShadow(const QImage &image);
    void setInnerShadowRect(const QRectF &rect);
    void setPadding(const QMarginsF &margins);

Q_SIGNALS:
    void shadowChanged(const QImage &);
    void innerShadowRectChanged();
    void paddingChanged();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}

Q_DECLARE_METATYPE(KDecoration3::DecorationShadow *)
