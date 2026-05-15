/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "mockwindow.h"
#include <decoratedwindow.h>

#include <QPalette>

MockWindow::MockWindow(KDecoration3::DecoratedWindow *client, KDecoration3::Decoration *decoration)
    : QObject()
    , DecoratedWindowPrivateV4(client, decoration)
{
}

Qt::Edges MockWindow::adjacentScreenEdges() const
{
    return Qt::Edges();
}

QString MockWindow::caption() const
{
    return QString();
}

qreal MockWindow::height() const
{
    return m_height;
}

QIcon MockWindow::icon() const
{
    return QIcon();
}

bool MockWindow::isActive() const
{
    return false;
}

bool MockWindow::isCloseable() const
{
    return m_closeable;
}

bool MockWindow::isKeepAbove() const
{
    return m_keepAbove;
}

bool MockWindow::isKeepBelow() const
{
    return m_keepBelow;
}

bool MockWindow::isExcludedFromCapture() const
{
    return m_excludeFromCapture;
}

bool MockWindow::isMaximizeable() const
{
    return m_maximizable;
}

bool MockWindow::isMaximized() const
{
    return isMaximizedHorizontally() && isMaximizedVertically();
}

bool MockWindow::isMaximizedHorizontally() const
{
    return m_maximizedHorizontally;
}

bool MockWindow::isMaximizedVertically() const
{
    return m_maximizedVertically;
}

bool MockWindow::isMinimizeable() const
{
    return m_minimizable;
}

bool MockWindow::isModal() const
{
    return false;
}

bool MockWindow::isMoveable() const
{
    return false;
}

bool MockWindow::isOnAllDesktops() const
{
    return m_onAllDesktops;
}

bool MockWindow::isResizeable() const
{
    return false;
}

bool MockWindow::isShadeable() const
{
    return m_shadeable;
}

bool MockWindow::isShaded() const
{
    return m_shaded;
}

QPalette MockWindow::palette() const
{
    return QPalette();
}

bool MockWindow::hasApplicationMenu() const
{
    return true;
}

bool MockWindow::isApplicationMenuActive() const
{
    return false;
}

bool MockWindow::providesContextHelp() const
{
    return m_contextHelp;
}

void MockWindow::requestClose()
{
    Q_EMIT closeRequested();
}

void MockWindow::requestContextHelp()
{
    Q_EMIT quickHelpRequested();
}

void MockWindow::requestToggleMaximization(Qt::MouseButtons buttons)
{
    bool maximizedHorizontally = m_maximizedHorizontally;
    bool maximizedVertically = m_maximizedVertically;
    if (buttons.testFlag(Qt::LeftButton)) {
        maximizedHorizontally = !m_maximizedHorizontally;
        maximizedVertically = !m_maximizedVertically;
    }
    if (buttons.testFlag(Qt::MiddleButton)) {
        maximizedHorizontally = !m_maximizedHorizontally;
    }
    if (buttons.testFlag(Qt::RightButton)) {
        maximizedVertically = !m_maximizedVertically;
    }
    const bool wasMaximized = isMaximized();
    if (m_maximizedHorizontally != maximizedHorizontally) {
        m_maximizedHorizontally = maximizedHorizontally;
        Q_EMIT window()->maximizedHorizontallyChanged(m_maximizedHorizontally);
    }
    if (m_maximizedVertically != maximizedVertically) {
        m_maximizedVertically = maximizedVertically;
        Q_EMIT window()->maximizedVerticallyChanged(m_maximizedVertically);
    }
    if (wasMaximized != isMaximized()) {
        Q_EMIT window()->maximizedChanged(isMaximized());
    }
}

void MockWindow::requestMinimize()
{
    Q_EMIT minimizeRequested();
}

void MockWindow::requestShowWindowMenu(const QRect &rect)
{
    Q_EMIT menuRequested();
}

void MockWindow::requestShowApplicationMenu(const QRect &rect, int actionId)
{
    Q_UNUSED(rect);
    Q_UNUSED(actionId);
    Q_EMIT applicationMenuRequested(); // FIXME TODO pass geometry
}

void MockWindow::requestToggleKeepAbove()
{
    m_keepAbove = !m_keepAbove;
    Q_EMIT window()->keepAboveChanged(m_keepAbove);
}

void MockWindow::requestToggleKeepBelow()
{
    m_keepBelow = !m_keepBelow;
    Q_EMIT window()->keepBelowChanged(m_keepBelow);
}

void MockWindow::requestToggleExcludeFromCapture()
{
    m_excludeFromCapture = !m_excludeFromCapture;
    Q_EMIT window()->excludeFromCaptureChanged(m_excludeFromCapture);
}

void MockWindow::requestToggleOnAllDesktops()
{
    m_onAllDesktops = !m_onAllDesktops;
    Q_EMIT window()->onAllDesktopsChanged(m_onAllDesktops);
}

void MockWindow::requestToggleShade()
{
    m_shaded = !m_shaded;
    Q_EMIT window()->shadedChanged(m_shaded);
}

void MockWindow::requestShowToolTip(const QString &text)
{
    Q_UNUSED(text);
}

void MockWindow::requestHideToolTip()
{
}

QSizeF MockWindow::size() const
{
    return QSize(m_width, m_height);
}

qreal MockWindow::width() const
{
    return m_width;
}

QString MockWindow::windowClass() const
{
    return QString();
}

void MockWindow::setCloseable(bool set)
{
    m_closeable = set;
    Q_EMIT window()->closeableChanged(set);
}

void MockWindow::setMinimizable(bool set)
{
    m_minimizable = set;
    Q_EMIT window()->minimizeableChanged(set);
}

void MockWindow::setProvidesContextHelp(bool set)
{
    m_contextHelp = set;
    Q_EMIT window()->providesContextHelpChanged(set);
}

void MockWindow::setShadeable(bool set)
{
    m_shadeable = set;
    Q_EMIT window()->shadeableChanged(set);
}

void MockWindow::setMaximizable(bool set)
{
    m_maximizable = set;
    Q_EMIT window()->maximizeableChanged(set);
}

void MockWindow::setWidth(int w)
{
    m_width = w;
    Q_EMIT window()->widthChanged(w);
}

void MockWindow::setHeight(int h)
{
    m_height = h;
    Q_EMIT window()->heightChanged(h);
}

void MockWindow::showApplicationMenu(int actionId)
{
    Q_UNUSED(actionId)
}

qreal MockWindow::scale() const
{
    return 1;
}

qreal MockWindow::nextScale() const
{
    return 1;
}

QString MockWindow::applicationMenuServiceName() const
{
    return QString();
}

QString MockWindow::applicationMenuObjectPath() const
{
    return QString();
}

void MockWindow::popup(const KDecoration3::Positioner &positioner, QMenu *menu)
{
    Q_UNUSED(positioner)
    Q_UNUSED(menu)
}

#include "moc_mockwindow.cpp"
