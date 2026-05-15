/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "../decorationdefines.h"
#include <kdecoration3/private/kdecoration3_private_export.h>

#include <QIcon>
#include <QString>

class QMenu;

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KDecoration3 API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace KDecoration3
{
class Decoration;
class DecoratedWindow;
class Positioner;

class KDECORATIONS_PRIVATE_EXPORT DecoratedWindowPrivate
{
public:
    virtual ~DecoratedWindowPrivate();
    virtual bool isActive() const = 0;
    virtual QString caption() const = 0;
    virtual bool isOnAllDesktops() const = 0;
    virtual bool isShaded() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool isMaximized() const = 0;
    virtual bool isMaximizedHorizontally() const = 0;
    virtual bool isMaximizedVertically() const = 0;
    virtual bool isKeepAbove() const = 0;
    virtual bool isKeepBelow() const = 0;

    virtual bool isCloseable() const = 0;
    virtual bool isMaximizeable() const = 0;
    virtual bool isMinimizeable() const = 0;
    virtual bool providesContextHelp() const = 0;
    virtual bool isModal() const = 0;
    virtual bool isShadeable() const = 0;
    virtual bool isMoveable() const = 0;
    virtual bool isResizeable() const = 0;

    virtual qreal width() const = 0;
    virtual qreal height() const = 0;
    virtual QSizeF size() const = 0;
    virtual QPalette palette() const = 0;
    virtual Qt::Edges adjacentScreenEdges() const = 0;
    virtual qreal scale() const = 0;
    virtual qreal nextScale() const = 0;

    virtual void requestShowToolTip(const QString &text) = 0;
    virtual void requestHideToolTip() = 0;
    virtual void requestClose() = 0;
    virtual void requestToggleMaximization(Qt::MouseButtons buttons) = 0;
    virtual void requestMinimize() = 0;
    virtual void requestContextHelp() = 0;
    virtual void requestToggleOnAllDesktops() = 0;
    virtual void requestToggleShade() = 0;
    virtual void requestToggleKeepAbove() = 0;
    virtual void requestToggleKeepBelow() = 0;
    virtual void requestShowWindowMenu(const QRect &rect) = 0;

    Decoration *decoration();
    Decoration *decoration() const;

    virtual QColor color(ColorGroup group, ColorRole role) const;
    virtual QString windowClass() const = 0;

    virtual bool hasApplicationMenu() const = 0;
    virtual bool isApplicationMenuActive() const = 0;

    virtual void showApplicationMenu(int actionId) = 0;
    virtual void requestShowApplicationMenu(const QRect &rect, int actionId) = 0;

protected:
    explicit DecoratedWindowPrivate(DecoratedWindow *client, Decoration *decoration);
    DecoratedWindow *window();

private:
    class Private;
    const std::unique_ptr<Private> d;
};

class KDECORATIONS_PRIVATE_EXPORT DecoratedWindowPrivateV2 : public DecoratedWindowPrivate
{
public:
    virtual QString applicationMenuServiceName() const = 0;
    virtual QString applicationMenuObjectPath() const = 0;

protected:
    explicit DecoratedWindowPrivateV2(DecoratedWindow *client, Decoration *decoration);
};

class KDECORATIONS_PRIVATE_EXPORT DecoratedWindowPrivateV3 : public DecoratedWindowPrivateV2
{
public:
    virtual void popup(const Positioner &positioner, QMenu *menu) = 0;

protected:
    explicit DecoratedWindowPrivateV3(DecoratedWindow *client, Decoration *decoration);
};

class KDECORATIONS_PRIVATE_EXPORT DecoratedWindowPrivateV4 : public DecoratedWindowPrivateV3
{
public:
    virtual bool isExcludedFromCapture() const = 0;
    virtual void requestToggleExcludeFromCapture() = 0;

protected:
    explicit DecoratedWindowPrivateV4(DecoratedWindow *client, Decoration *decoration);
};

} // namespace
