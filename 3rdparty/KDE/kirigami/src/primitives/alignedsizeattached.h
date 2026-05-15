/*
 *  SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef ALIGNEDSIZEATTACHED_H
#define ALIGNEDSIZEATTACHED_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

class QQuickItem;

/*!
 * \qmltype AlignedSize
 * \inqmlmodule org.kde.kirigami.primitives
 *
 * \brief An attached property that aligns a size on the physical pixel grid.
 *
 * Transforms a size to the nearest one perfectly aligned to the pixel grid
 * in case fractional scaling in used.
 * \qml
 * import org.kde.kirigami as Kirigami
 *
 * Rectangle {
 *    width: AlignedSize.alignedWidth
 *    height: AlignedSize.alignedHeight
 *    AlignedSize.width: Units.gridUnit
 *    AlignedSize.height: 1
 * }
 * \endqml
 *
 * \since 6.20
 */
class AlignedSizeAttached : public QObject
{
    Q_OBJECT
    QML_ATTACHED(AlignedSizeAttached)
    QML_NAMED_ELEMENT(AlignedSize)
    QML_UNCREATABLE("")

    /*!
     * \qmlattachedproperty real AlignedSize::width
     *
     * The width of the item in logical pixels
     *
     */
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged RESET resetWidth FINAL)

    /*!
     * \qmlattachedproperty real AlignedSize::height
     *
     * The height of the item in logical pixels
     *
     */
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged RESET resetHeight FINAL)

public:
    explicit AlignedSizeAttached(QObject *parent = nullptr);
    ~AlignedSizeAttached() override;

    qreal width() const;
    void setWidth(qreal width);
    void resetWidth();

    qreal height() const;
    void setHeight(qreal height);
    void resetHeight();

    // QML attached property
    static AlignedSizeAttached *qmlAttachedProperties(QObject *object);

    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void widthChanged();
    void heightChanged();

private:
    qreal alignedWidth() const;
    qreal alignedHeight() const;
    void dprChanged();

    QQuickItem *m_item = nullptr;
    QPointer<QQuickWindow> m_window;
    qreal m_dpr = 1.0;
    qreal m_width = -1.0;
    qreal m_height = -1.0;
};

QML_DECLARE_TYPEINFO(AlignedSizeAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif // ALIGNEDSIZEATTACHED_H
