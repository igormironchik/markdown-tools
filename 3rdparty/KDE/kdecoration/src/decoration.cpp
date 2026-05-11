/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decoration.h"
#include "decoratedwindow.h"
#include "decoration_p.h"
#include "decorationbutton.h"
#include "decorationsettings.h"
#include "private/decoratedwindowprivate.h"
#include "private/decorationbridge.h"

#include <QCoreApplication>
#include <QHoverEvent>

#include <cmath>

namespace KDecoration3
{
namespace
{
DecorationBridge *findBridge(const QVariantList &args)
{
    for (const auto &arg : args) {
        if (auto bridge = arg.toMap().value(QStringLiteral("bridge")).value<DecorationBridge *>()) {
            return bridge;
        }
    }
    Q_UNREACHABLE();
}
}

BorderRadius::BorderRadius()
{
}

BorderRadius::BorderRadius(qreal radius)
    : BorderRadius(radius, radius, radius, radius)
{
}

BorderRadius::BorderRadius(qreal topLeft, qreal topRight, qreal bottomRight, qreal bottomLeft)
    : m_topLeft(topLeft)
    , m_topRight(topRight)
    , m_bottomRight(bottomRight)
    , m_bottomLeft(bottomLeft)
{
}

qreal BorderRadius::topLeft() const
{
    return m_topLeft;
}

qreal BorderRadius::topRight() const
{
    return m_topRight;
}

qreal BorderRadius::bottomRight() const
{
    return m_bottomRight;
}

qreal BorderRadius::bottomLeft() const
{
    return m_bottomLeft;
}

BorderOutline::BorderOutline()
{
}

BorderOutline::BorderOutline(qreal thickness, const QColor &color, const BorderRadius &radius)
    : m_thickness(thickness)
    , m_color(color)
    , m_radius(radius)
{
}

bool BorderOutline::isNull() const
{
    return qFuzzyIsNull(m_thickness);
}

qreal BorderOutline::thickness() const
{
    return m_thickness;
}

QColor BorderOutline::color() const
{
    return m_color;
}

BorderRadius BorderOutline::radius() const
{
    return m_radius;
}

class DecorationStateData : public QSharedData
{
public:
    QMarginsF borders;
    BorderRadius borderRadius;
    BorderOutline borderOutline;
};

DecorationState::DecorationState()
    : d(new DecorationStateData)
{
}

DecorationState::DecorationState(const DecorationState &other)
    : d(other.d)
{
}

DecorationState::~DecorationState()
{
}

std::shared_ptr<DecorationState> DecorationState::clone() const
{
    return std::make_shared<DecorationState>(*this);
}

QMarginsF DecorationState::borders() const
{
    return d->borders;
}

void DecorationState::setBorders(const QMarginsF &borders)
{
    d->borders = borders;
}

BorderRadius DecorationState::borderRadius() const
{
    return d->borderRadius;
}

void DecorationState::setBorderRadius(const BorderRadius &radius)
{
    d->borderRadius = radius;
}

BorderOutline DecorationState::borderOutline() const
{
    return d->borderOutline;
}

void DecorationState::setBorderOutline(const BorderOutline &outline)
{
    d->borderOutline = outline;
}

Positioner::Positioner()
    : d(new PositionerData)
{
}

Positioner::Positioner(const Positioner &other)
    : d(other.d)
{
}

Positioner::~Positioner()
{
}

QRectF Positioner::anchorRect() const
{
    return d->anchorRect;
}

void Positioner::setAnchorRect(const QRectF &rect)
{
    d->anchorRect = rect;
}

Decoration::Private::Private(Decoration *deco, const QVariantList &args)
    : sectionUnderMouse(Qt::NoSection)
    , bridge(findBridge(args))
    , client(std::shared_ptr<DecoratedWindow>(new DecoratedWindow(deco, bridge)))
    , opaque(false)
    , q(deco)
{
}

void Decoration::Private::setSectionUnderMouse(Qt::WindowFrameSection section)
{
    if (sectionUnderMouse == section) {
        return;
    }
    sectionUnderMouse = section;
    Q_EMIT q->sectionUnderMouseChanged(sectionUnderMouse);
}

void Decoration::Private::updateSectionUnderMouse(const QPoint &mousePosition)
{
    if (titleBar.toRect().contains(mousePosition)) {
        setSectionUnderMouse(Qt::TitleBarArea);
        return;
    }
    const QSizeF size = q->size();
    const QMarginsF borders = current->borders();
    const int corner = 2 * settings->largeSpacing();
    const bool left = mousePosition.x() < borders.left();
    const bool top = mousePosition.y() < borders.top();
    const bool bottom = mousePosition.y() >= size.height() - borders.bottom();
    const bool right = mousePosition.x() >= size.width() - borders.right();
    if (left) {
        if (top && mousePosition.y() < titleBar.top() + corner) {
            setSectionUnderMouse(Qt::TopLeftSection);
        } else if (mousePosition.y() >= size.height() - borders.bottom() - corner && mousePosition.y() >= titleBar.bottom()) {
            setSectionUnderMouse(Qt::BottomLeftSection);
        } else {
            setSectionUnderMouse(Qt::LeftSection);
        }
        return;
    }
    if (right) {
        if (top && mousePosition.y() < titleBar.top() + corner) {
            setSectionUnderMouse(Qt::TopRightSection);
        } else if (mousePosition.y() >= size.height() - borders.bottom() - corner && mousePosition.y() >= titleBar.bottom()) {
            setSectionUnderMouse(Qt::BottomRightSection);
        } else {
            setSectionUnderMouse(Qt::RightSection);
        }
        return;
    }
    if (bottom) {
        if (mousePosition.y() >= titleBar.bottom()) {
            if (mousePosition.x() < borders.left() + corner) {
                setSectionUnderMouse(Qt::BottomLeftSection);
            } else if (mousePosition.x() >= size.width() - borders.right() - corner) {
                setSectionUnderMouse(Qt::BottomRightSection);
            } else {
                setSectionUnderMouse(Qt::BottomSection);
            }
        } else {
            setSectionUnderMouse(Qt::TitleBarArea);
        }
        return;
    }
    if (top) {
        if (mousePosition.y() < titleBar.top()) {
            if (mousePosition.x() < borders.left() + corner) {
                setSectionUnderMouse(Qt::TopLeftSection);
            } else if (mousePosition.x() >= size.width() - borders.right() - corner) {
                setSectionUnderMouse(Qt::TopRightSection);
            } else {
                setSectionUnderMouse(Qt::TopSection);
            }
        } else {
            setSectionUnderMouse(Qt::TitleBarArea);
        }
        return;
    }
    setSectionUnderMouse(Qt::NoSection);
}

void Decoration::Private::addButton(DecorationButton *button)
{
    Q_ASSERT(!buttons.contains(button));
    buttons << button;
    QObject::connect(button, &QObject::destroyed, q, [this](QObject *o) {
        auto it = buttons.begin();
        while (it != buttons.end()) {
            if (*it == static_cast<DecorationButton *>(o)) {
                it = buttons.erase(it);
            } else {
                it++;
            }
        }
    });
}

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , d(new Private(this, args))
{
}

Decoration::~Decoration() = default;

DecoratedWindow *Decoration::window() const
{
    return d->client.get();
}

void Decoration::requestClose()
{
    d->client->d->requestClose();
}

void Decoration::requestContextHelp()
{
    d->client->d->requestContextHelp();
}

void Decoration::requestMinimize()
{
    d->client->d->requestMinimize();
}

void Decoration::requestToggleOnAllDesktops()
{
    d->client->d->requestToggleOnAllDesktops();
}

void Decoration::requestToggleShade()
{
    d->client->d->requestToggleShade();
}

void Decoration::requestToggleKeepAbove()
{
    d->client->d->requestToggleKeepAbove();
}

void Decoration::requestToggleKeepBelow()
{
    d->client->d->requestToggleKeepBelow();
}

#if KDECORATIONS3_ENABLE_DEPRECATED_SINCE(5, 21)
void Decoration::requestShowWindowMenu()
{
    requestShowWindowMenu(QRect());
}
#endif

void Decoration::requestShowWindowMenu(const QRect &rect)
{
    d->client->d->requestShowWindowMenu(rect);
}

void Decoration::requestShowToolTip(const QString &text)
{
    d->client->d->requestShowToolTip(text);
}

void Decoration::requestHideToolTip()
{
    d->client->d->requestHideToolTip();
}

void Decoration::requestToggleMaximization(Qt::MouseButtons buttons)
{
    d->client->d->requestToggleMaximization(buttons);
}

void Decoration::showApplicationMenu(int actionId)
{
    const auto it = std::find_if(d->buttons.constBegin(), d->buttons.constEnd(), [](DecorationButton *button) {
        return button->type() == DecorationButtonType::ApplicationMenu;
    });
    if (it != d->buttons.constEnd()) {
        requestShowApplicationMenu((*it)->geometry().toRect(), actionId);
    }
}

void Decoration::requestShowApplicationMenu(const QRect &rect, int actionId)
{
    d->client->d->requestShowApplicationMenu(rect, actionId);
}

void Decoration::setBlurRegion(const QRegion &region)
{
    if (d->blurRegion != region) {
        d->blurRegion = region;
        Q_EMIT blurRegionChanged();
    }
}

void Decoration::setBorders(const QMarginsF &borders)
{
    if (d->next->borders() != borders) {
        setState([borders](DecorationState *state) {
            state->setBorders(borders);
        });
    }
}

void Decoration::setResizeOnlyBorders(const QMarginsF &borders)
{
    if (d->resizeOnlyBorders != borders) {
        d->resizeOnlyBorders = borders;
        Q_EMIT resizeOnlyBordersChanged();
    }
}

void Decoration::setBorderRadius(const BorderRadius &radius)
{
    if (d->next->borderRadius() != radius) {
        setState([radius](DecorationState *state) {
            state->setBorderRadius(radius);
        });
    }
}

void Decoration::setBorderOutline(const BorderOutline &outline)
{
    if (d->next->borderOutline() != outline) {
        setState([outline](DecorationState *state) {
            state->setBorderOutline(outline);
        });
    }
}

void Decoration::setTitleBar(const QRectF &rect)
{
    if (d->titleBar != rect) {
        d->titleBar = rect;
        Q_EMIT titleBarChanged();
    }
}

void Decoration::setOpaque(bool opaque)
{
    if (d->opaque != opaque) {
        d->opaque = opaque;
        Q_EMIT opaqueChanged(opaque);
    }
}

void Decoration::setShadow(const std::shared_ptr<DecorationShadow> &shadow)
{
    if (d->shadow != shadow) {
        d->shadow = shadow;
        Q_EMIT shadowChanged(shadow);
    }
}

QRegion Decoration::blurRegion() const
{
    return d->blurRegion;
}

QMarginsF Decoration::borders() const
{
    return d->current->borders();
}

QMarginsF Decoration::resizeOnlyBorders() const
{
    return d->resizeOnlyBorders;
}

QRectF Decoration::titleBar() const
{
    return d->titleBar;
}

Qt::WindowFrameSection Decoration::sectionUnderMouse() const
{
    return d->sectionUnderMouse;
}

std::shared_ptr<DecorationShadow> Decoration::shadow() const
{
    return d->shadow;
}

bool Decoration::isOpaque() const
{
    return d->opaque;
}

qreal Decoration::borderLeft() const
{
    return d->current->borders().left();
}

qreal Decoration::resizeOnlyBorderLeft() const
{
    return d->resizeOnlyBorders.left();
}

qreal Decoration::borderRight() const
{
    return d->current->borders().right();
}

qreal Decoration::resizeOnlyBorderRight() const
{
    return d->resizeOnlyBorders.right();
}

qreal Decoration::borderTop() const
{
    return d->current->borders().top();
}

qreal Decoration::resizeOnlyBorderTop() const
{
    return d->resizeOnlyBorders.top();
}

qreal Decoration::borderBottom() const
{
    return d->current->borders().bottom();
}

qreal Decoration::resizeOnlyBorderBottom() const
{
    return d->resizeOnlyBorders.bottom();
}

BorderRadius Decoration::borderRadius() const
{
    return d->current->borderRadius();
}

BorderOutline Decoration::borderOutline() const
{
    return d->current->borderOutline();
}

QSizeF Decoration::size() const
{
    const QMarginsF b = d->current->borders();
    return QSizeF(d->client->width() + b.left() + b.right(), (d->client->isShaded() ? 0 : d->client->height()) + b.top() + b.bottom());
}

QRectF Decoration::rect() const
{
    return QRectF(QPointF(0, 0), size());
}

bool Decoration::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverEnter:
        hoverEnterEvent(static_cast<QHoverEvent *>(event));
        return true;
    case QEvent::HoverLeave:
        hoverLeaveEvent(static_cast<QHoverEvent *>(event));
        return true;
    case QEvent::HoverMove:
        hoverMoveEvent(static_cast<QHoverEvent *>(event));
        return true;
    case QEvent::MouseButtonPress:
        mousePressEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseButtonRelease:
        mouseReleaseEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseMove:
        mouseMoveEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::Wheel:
        wheelEvent(static_cast<QWheelEvent *>(event));
        return true;
    default:
        return QObject::event(event);
    }
}

void Decoration::hoverEnterEvent(QHoverEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        QCoreApplication::instance()->sendEvent(button, event);
    }
    auto flooredPos = QPoint(std::floor(event->position().x()), std::floor(event->position().y()));
    d->updateSectionUnderMouse(flooredPos);
}

void Decoration::hoverLeaveEvent(QHoverEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        QCoreApplication::instance()->sendEvent(button, event);
    }
    d->setSectionUnderMouse(Qt::NoSection);
}

void Decoration::hoverMoveEvent(QHoverEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        if (!button->isEnabled() || !button->isVisible()) {
            continue;
        }
        const bool hovered = button->isHovered();
        const bool contains = button->contains(event->position());
        if (!hovered && contains) {
            QHoverEvent e(QEvent::HoverEnter, event->position(), event->oldPosF(), event->modifiers());
            QCoreApplication::instance()->sendEvent(button, &e);
        } else if (hovered && !contains) {
            QHoverEvent e(QEvent::HoverLeave, event->position(), event->oldPosF(), event->modifiers());
            QCoreApplication::instance()->sendEvent(button, &e);
        } else if (hovered && contains) {
            QCoreApplication::instance()->sendEvent(button, event);
        }
    }
    auto flooredPos = QPoint(std::floor(event->position().x()), std::floor(event->position().y()));
    d->updateSectionUnderMouse(flooredPos);
}

void Decoration::mouseMoveEvent(QMouseEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        if (button->isPressed()) {
            QCoreApplication::instance()->sendEvent(button, event);
            return;
        }
    }
    // not handled, take care ourselves
}

void Decoration::mousePressEvent(QMouseEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        if (button->isHovered()) {
            if (button->acceptedButtons().testFlag(event->button())) {
                QCoreApplication::instance()->sendEvent(button, event);
            }
            event->setAccepted(true);
            return;
        }
    }
}

void Decoration::mouseReleaseEvent(QMouseEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        if (button->isPressed() && button->acceptedButtons().testFlag(event->button())) {
            QCoreApplication::instance()->sendEvent(button, event);
            return;
        }
    }
    // not handled, take care ourselves
    d->updateSectionUnderMouse(event->pos());
}

void Decoration::wheelEvent(QWheelEvent *event)
{
    for (DecorationButton *button : d->buttons) {
        if (button->contains(event->position())) {
            QCoreApplication::instance()->sendEvent(button, event);
            event->setAccepted(true);
        }
    }
}

void Decoration::update(const QRectF &r)
{
    Q_EMIT damaged(r.isNull() ? rect().toAlignedRect() : r.toAlignedRect());
}

void Decoration::update()
{
    update(QRect());
}

void Decoration::setSettings(const std::shared_ptr<DecorationSettings> &settings)
{
    d->settings = settings;
}

std::shared_ptr<DecorationSettings> Decoration::settings() const
{
    return d->settings;
}

std::shared_ptr<DecorationState> Decoration::createState()
{
    return std::make_shared<DecorationState>();
}

std::shared_ptr<DecorationState> Decoration::currentState() const
{
    return d->current;
}

std::shared_ptr<DecorationState> Decoration::nextState() const
{
    return d->next;
}

void Decoration::create()
{
    d->next = createState();
    d->current = createState();
}

void Decoration::setState(std::function<void(DecorationState *state)> callback)
{
    callback(d->next.get());
    Q_EMIT nextStateChanged(d->next);
}

void Decoration::apply(std::shared_ptr<DecorationState> state)
{
    if (d->current == state) {
        return;
    }

    const auto previous = d->current;
    d->current = state;
    update();

    if (previous->borders() != state->borders()) {
        Q_EMIT bordersChanged();
    }
    if (previous->borderRadius() != state->borderRadius()) {
        Q_EMIT borderRadiusChanged();
    }
    if (previous->borderOutline() != state->borderOutline()) {
        Q_EMIT borderOutlineChanged();
    }

    Q_EMIT currentStateChanged(state);
}

void Decoration::popup(const Positioner &positioner, QMenu *menu)
{
    if (auto window = dynamic_cast<DecoratedWindowPrivateV3 *>(d->client->d.get())) {
        window->popup(positioner, menu);
    }
}

void Decoration::requestToggleExcludeFromCapture()
{
    if (auto window = dynamic_cast<DecoratedWindowPrivateV4 *>(d->client->d.get())) {
        window->requestToggleExcludeFromCapture();
    }
}

} // namespace

#include "moc_decoration.cpp"
