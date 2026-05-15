/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decoratedwindow.h"
#include "private/decoratedwindowprivate.h"
#include "private/decorationbridge.h"
#include "scalehelpers.h"

#include <QColor>

namespace KDecoration3
{
DecoratedWindow::DecoratedWindow(Decoration *parent, DecorationBridge *bridge)
    : QObject()
    , d(bridge->createClient(this, parent))
{
}

DecoratedWindow::~DecoratedWindow() = default;

bool DecoratedWindow::isActive() const
{
    return d->isActive();
}

QString DecoratedWindow::caption() const
{
    return d->caption();
}

bool DecoratedWindow::isOnAllDesktops() const
{
    return d->isOnAllDesktops();
}

bool DecoratedWindow::isShaded() const
{
    return d->isShaded();
}

QIcon DecoratedWindow::icon() const
{
    return d->icon();
}

bool DecoratedWindow::isMaximized() const
{
    return d->isMaximized();
}

bool DecoratedWindow::isMaximizedHorizontally() const
{
    return d->isMaximizedHorizontally();
}

bool DecoratedWindow::isMaximizedVertically() const
{
    return d->isMaximizedVertically();
}

bool DecoratedWindow::isKeepAbove() const
{
    return d->isKeepAbove();
}

bool DecoratedWindow::isKeepBelow() const
{
    return d->isKeepBelow();
}

bool DecoratedWindow::isExcludedFromCapture() const
{
    if (auto impl = dynamic_cast<DecoratedWindowPrivateV4 *>(d.get())) {
        return impl->isExcludedFromCapture();
    }
    return false;
}

bool DecoratedWindow::isCloseable() const
{
    return d->isCloseable();
}

bool DecoratedWindow::isMaximizeable() const
{
    return d->isMaximizeable();
}

bool DecoratedWindow::isMinimizeable() const
{
    return d->isMinimizeable();
}

bool DecoratedWindow::providesContextHelp() const
{
    return d->providesContextHelp();
}

bool DecoratedWindow::isModal() const
{
    return d->isModal();
}

bool DecoratedWindow::isShadeable() const
{
    return d->isShadeable();
}

bool DecoratedWindow::isMoveable() const
{
    return d->isMoveable();
}

bool DecoratedWindow::isResizeable() const
{
    return d->isResizeable();
}

qreal DecoratedWindow::width() const
{
    return d->width();
}

qreal DecoratedWindow::height() const
{
    return d->height();
}

QSizeF DecoratedWindow::size() const
{
    return d->size();
}

QPalette DecoratedWindow::palette() const
{
    return d->palette();
}

Qt::Edges DecoratedWindow::adjacentScreenEdges() const
{
    return d->adjacentScreenEdges();
}

QString DecoratedWindow::windowClass() const
{
    return d->windowClass();
}

bool DecoratedWindow::hasApplicationMenu() const
{
    return d->hasApplicationMenu();
}

bool DecoratedWindow::isApplicationMenuActive() const
{
    return d->isApplicationMenuActive();
}

Decoration *DecoratedWindow::decoration() const
{
    return d->decoration();
}

QColor DecoratedWindow::color(QPalette::ColorGroup group, QPalette::ColorRole role) const
{
    return d->palette().color(group, role);
}

QColor DecoratedWindow::color(ColorGroup group, ColorRole role) const
{
    return d->color(group, role);
}

void DecoratedWindow::showApplicationMenu(int actionId)
{
    d->showApplicationMenu(actionId);
}

qreal DecoratedWindow::scale() const
{
    return d->scale();
}

qreal DecoratedWindow::nextScale() const
{
    return d->nextScale();
}

QString DecoratedWindow::applicationMenuServiceName() const
{
    if (auto impl = dynamic_cast<DecoratedWindowPrivateV2 *>(d.get())) {
        return impl->applicationMenuServiceName();
    }
    return QString();
}

QString DecoratedWindow::applicationMenuObjectPath() const
{
    if (auto impl = dynamic_cast<DecoratedWindowPrivateV2 *>(d.get())) {
        return impl->applicationMenuObjectPath();
    }
    return QString();
}

} // namespace

#include "moc_decoratedwindow.cpp"
