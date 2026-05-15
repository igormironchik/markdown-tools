/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "decoration.h"
#include "decorationdefines.h"
#include <kdecoration3/kdecoration3_export.h>

#include <QFont>
#include <QIcon>
#include <QObject>
#include <QPalette>
#include <QPointer>
#include <QtGui/qwindowdefs.h>

#include <memory>

namespace KDecoration3
{
class DecorationBridge;
class DecoratedWindowPrivate;

/**
 * @brief The Client which gets decorated.
 *
 * The DecoratedWindow provides access to all the properties relevant for decorating the Client.
 * Each DecoratedWindow is bound to one Decoration and each Decoration is bound to this one
 * DecoratedWindow.
 *
 * The DecoratedWindow only exports properties, it does not provide any means to change the state.
 * To change state one needs to call the methods on Decoration. This is as the backend might
 * disallow state changes. Therefore any changes should be bound to the change signals of the
 * DecoratedWindow and not be bound to state changes of input elements (such as a button).
 */
class KDECORATIONS3_EXPORT DecoratedWindow : public QObject
{
    Q_OBJECT
    /**
     * The Decoration of this DecoratedWindow
     **/
    Q_PROPERTY(KDecoration3::Decoration *decoration READ decoration CONSTANT)
    /**
     * Whether the DecoratedWindow is active (has focus) or is inactive.
     **/
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    /**
     * The caption of the DecoratedWindow.
     **/
    Q_PROPERTY(QString caption READ caption NOTIFY captionChanged)
    /**
     * Whether the DecoratedWindow is on all desktops or on just one.
     **/
    Q_PROPERTY(bool onAllDesktops READ isOnAllDesktops NOTIFY onAllDesktopsChanged)
    /**
     * Whether the DecoratedWindow is shaded. Shaded means that the actual content is
     * not visible, only the Decoration is visible.
     **/
    Q_PROPERTY(bool shaded READ isShaded NOTIFY shadedChanged)
    /**
     * The icon of the DecoratedWindow. This can be used as the icon for the window menu button.
     **/
    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)
    /**
     * Whether the DecoratedWindow is maximized. A DecoratedWindow is maximized if it is both
     * maximizedHorizontally and maximizedVertically. The Decoration of a maximized DecoratedWindow
     * should only consist of the title bar area.
     **/
    Q_PROPERTY(bool maximized READ isMaximized NOTIFY maximizedChanged)
    /**
     * Whether the DecoratedWindow is maximized horizontally. A horizontally maximized DecoratedWindow
     * uses the maximal possible width.
     **/
    Q_PROPERTY(bool maximizedHorizontally READ isMaximizedHorizontally NOTIFY maximizedHorizontallyChanged)
    /**
     * Whether the DecoratedWindow is maximized vertically. A vertically maximized DecoratedWindow
     * uses the maximal possible height.
     **/
    Q_PROPERTY(bool maximizedVertically READ isMaximizedVertically NOTIFY maximizedVerticallyChanged)
    /**
     * Whether the DecoratedWindow is set to be kept above other DecoratedWindows. There can be multiple
     * DecoratedWindows which are set to be kept above.
     **/
    Q_PROPERTY(bool keepAbove READ isKeepAbove NOTIFY keepAboveChanged)
    /**
     * Whether the DecoratedWindow is set to be kept below other DecoratedWindows. There can be multiple
     * DecoratedWindows which are set to be kept below.
     **/
    Q_PROPERTY(bool keepBelow READ isKeepBelow NOTIFY keepBelowChanged)
    /**
     * Whether the DecoratedWindow is hidden from Screencasts.
     **/
    Q_PROPERTY(bool excludeFromCapture READ isExcludedFromCapture NOTIFY excludeFromCaptureChanged)

    /**
     * Whether the DecoratedWindow can be closed. If this property is @c false a DecorationButton
     * for closing the DecoratedWindow should be disabled.
     **/
    Q_PROPERTY(bool closeable READ isCloseable NOTIFY closeableChanged)
    /**
     * Whether the DecoratedWindow can be maximized. If this property is @c false a DecorationButton
     * for maximizing the DecoratedWindow should be disabled.
     **/
    Q_PROPERTY(bool maximizeable READ isMaximizeable NOTIFY maximizeableChanged)
    /**
     * Whether the DecoratedWindow can be minimized. If this property is @c false a DecorationButton
     * for minimizing the DecoratedWindow should be disabled.
     **/
    Q_PROPERTY(bool minimizeable READ isMinimizeable NOTIFY minimizeableChanged)
    /**
     * Whether the DecoratedWindow provides context help.
     * The Decoration should only show a context help button if this property is @c true.
     **/
    Q_PROPERTY(bool providesContextHelp READ providesContextHelp NOTIFY providesContextHelpChanged)
    /**
     * Whether the DecoratedWindow is a modal dialog.
     **/
    Q_PROPERTY(bool modal READ isModal CONSTANT)
    /**
     * Whether the DecoratedWindow can be shaded. If this property is @c false a DecorationButton
     * for shading the DecoratedWindow should be disabled.
     **/
    Q_PROPERTY(bool shadeable READ isShadeable NOTIFY shadeableChanged)
    /**
     * Whether the DecoratedWindow can be moved.
     **/
    Q_PROPERTY(bool moveable READ isMoveable NOTIFY moveableChanged)
    /**
     * Whether the DecoratedWindow can be resized.
     **/
    Q_PROPERTY(bool resizeable READ isResizeable NOTIFY resizeableChanged)

    /**
     * The width of the DecoratedWindow.
     **/
    Q_PROPERTY(qreal width READ width NOTIFY widthChanged)
    /**
     * The height of the DecoratedWindow.
     **/
    Q_PROPERTY(qreal height READ height NOTIFY heightChanged)
    /**
     * The size of the DecoratedWindow.
     **/
    Q_PROPERTY(QSizeF size READ size NOTIFY sizeChanged)
    /**
     * The palette this DecoratedWindow uses. The palette might be different for each
     * DecoratedWindow and the Decoration should honor the palette.
     **/
    Q_PROPERTY(QPalette palette READ palette NOTIFY paletteChanged)
    /**
     * The Edges which are adjacent to a screen edge. E.g. for a maximized DecoratedWindow this
     * will include all Edges. The Decoration can use this information to hide borders.
     **/
    Q_PROPERTY(Qt::Edges adjacentScreenEdges READ adjacentScreenEdges NOTIFY adjacentScreenEdgesChanged)
    /**
     * Whether the DecoratedWindow has an application menu
     * @since 5.9
     */
    Q_PROPERTY(bool hasApplicationMenu READ hasApplicationMenu NOTIFY hasApplicationMenuChanged)
    /**
     * Whether the application menu for this DecoratedWindow is currently shown to the user
     * The Decoration can use this information to highlight the respective button.
     * @since 5.9
     */
    Q_PROPERTY(bool applicationMenuActive READ isApplicationMenuActive NOTIFY applicationMenuActiveChanged)
    /**
     * Returns the DBus service name of the application menu. If the window has no application
     * menu associated with it, an empty string will be returned.
     *
     * @since 6.3.2
     */
    Q_PROPERTY(QString applicationMenuServiceName READ applicationMenuServiceName NOTIFY applicationMenuChanged)
    /**
     * Returns the DBus object path of the application menu. If the window has no application
     * menu associated with it, an empty string will be returned.
     *
     * @since 6.3.2
     */
    Q_PROPERTY(QString applicationMenuObjectPath READ applicationMenuObjectPath NOTIFY applicationMenuChanged)

    /**
     * The current scale this decorated window is targeting.
     * @since 6.3
     */
    Q_PROPERTY(qreal scale READ scale NOTIFY scaleChanged);

    /**
     * The next scale this decorated window is going to target.
     * @since 6.3
     */
    Q_PROPERTY(qreal nextScale READ nextScale NOTIFY nextScaleChanged)

public:
    DecoratedWindow() = delete;
    ~DecoratedWindow() override;
    bool isActive() const;
    QString caption() const;
    bool isOnAllDesktops() const;
    bool isShaded() const;
    QIcon icon() const;
    bool isMaximized() const;
    bool isMaximizedHorizontally() const;
    bool isMaximizedVertically() const;
    bool isKeepAbove() const;
    bool isKeepBelow() const;
    bool isExcludedFromCapture() const;

    bool isCloseable() const;
    bool isMaximizeable() const;
    bool isMinimizeable() const;
    bool providesContextHelp() const;
    bool isModal() const;
    bool isShadeable() const;
    bool isMoveable() const;
    bool isResizeable() const;

    Qt::Edges adjacentScreenEdges() const;

    QString windowClass() const;

    qreal width() const;
    qreal height() const;
    QSizeF size() const;

    Decoration *decoration() const;
    QPalette palette() const;
    /**
     * Used to get colors in QPalette.
     * @param group The color group
     * @param role The color role
     * @return palette().color(group, role)
     * @since 5.3
     **/
    QColor color(QPalette::ColorGroup group, QPalette::ColorRole role) const;
    /**
     * Used to get additional colors that are not in QPalette.
     * @param group The color group
     * @param role The color role
     * @return The color if provided for combination of group and role, otherwise invalid QColor.
     * @since 5.3
     **/
    QColor color(ColorGroup group, ColorRole role) const;

    /**
     * Whether the DecoratedWindow has an application menu
     * @since 5.9
     */
    bool hasApplicationMenu() const;
    /**
     * Returns the DBus service name of the application menu. If the window has no application
     * menu associated with it, an empty string will be returned.
     *
     * @since 6.3.2
     */
    QString applicationMenuServiceName() const;
    /**
     * Returns the DBus object path of the application menu. If the window has no application
     * menu associated with it, an empty string will be returned.
     *
     * @since 6.3.2
     */
    QString applicationMenuObjectPath() const;
    /**
     * Whether the application menu for this DecoratedWindow is currently shown to the user
     * The Decoration can use this information to highlight the respective button.
     * @since 5.9
     */
    bool isApplicationMenuActive() const;

    /**
     * Request the application menu to be shown to the user
     * @param actionId The DBus menu ID of the action that should be highlighted, 0 for none.
     */
    void showApplicationMenu(int actionId);

    /**
     * Returns the current scale this decorated window is targeting
     * @since 6.3
     */
    qreal scale() const;
    /**
     * Returns the next scale this decorated window is going to target.
     */
    qreal nextScale() const;

Q_SIGNALS:
    void activeChanged(bool);
    void captionChanged(QString);
    void onAllDesktopsChanged(bool);
    void shadedChanged(bool);
    void iconChanged(QIcon);
    void maximizedChanged(bool);
    void maximizedHorizontallyChanged(bool);
    void maximizedVerticallyChanged(bool);
    void keepAboveChanged(bool);
    void keepBelowChanged(bool);
    void excludeFromCaptureChanged(bool);

    void closeableChanged(bool);
    void maximizeableChanged(bool);
    void minimizeableChanged(bool);
    void providesContextHelpChanged(bool);
    void shadeableChanged(bool);
    void moveableChanged(bool);
    void resizeableChanged(bool);

    void widthChanged(qreal);
    void heightChanged(qreal);
    void sizeChanged(const QSizeF &size);
    void paletteChanged(const QPalette &palette);
    void adjacentScreenEdgesChanged(Qt::Edges edges);

    void hasApplicationMenuChanged(bool);
    void applicationMenuActiveChanged(bool);
    void scaleChanged();
    void nextScaleChanged();
    void applicationMenuChanged();

private:
    friend class Decoration;
    DecoratedWindow(Decoration *parent, DecorationBridge *bridge);
    const std::unique_ptr<DecoratedWindowPrivate> d;
};

} // namespace
