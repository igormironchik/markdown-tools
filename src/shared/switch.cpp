
/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "switch.h"
#include "color.h"
#include "fingergeometry.h"

// Qt include.
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

namespace MdShared
{

//
// drawSliderHandle
//

void drawSliderHandle(QPainter *p,
                      const QRect &r,
                      int xRadius,
                      int yRadius,
                      const QColor &borderColor,
                      const QColor &lightColor)
{
    p->setPen(borderColor);
    p->setBrush(lightColor);
    p->drawRoundedRect(r, xRadius, yRadius);

    QLinearGradient g(QPointF(0.0, 0.0), QPointF(0.0, 1.0));
    g.setCoordinateMode(QGradient::ObjectBoundingMode);
    g.setColorAt(0.0, darkerColor(lightColor, 75));
    g.setColorAt(1.0, darkerColor(lightColor, 10));

    p->setPen(Qt::NoPen);
    p->setBrush(g);

    p->drawRoundedRect(r.marginsRemoved(QMargins(2, 2, 2, 2)), xRadius - 4, yRadius - 4);
}

//
// SwitchPrivate
//

class SwitchPrivate
{
public:
    explicit SwitchPrivate(Switch *parent)
        : q(parent)
        , state(Switch::AcceptedUncheck)
        , radius(0)
        , textWidth(0)
        , offset(0)
        , mouseMoveDelta(0)
        , leftMouseButtonPressed(false)
    {
        init();
    }

    bool isChecked() const;
    void setState(Switch::State st,
                  bool justUpdate = false);
    void init();
    void emitSignals();
    void drawText(QPainter *p,
                  const QStyleOption &opt,
                  const QColor &on,
                  const QColor &off);
    void initOffset(const QRect &r);

    Switch *q;
    Switch::State state;
    QString onText;
    QString offText;
    QColor onColor;
    int radius;
    int textWidth;
    int offset;
    int mouseMoveDelta;
    bool leftMouseButtonPressed;
    QPoint mousePos;
}; // class SwitchPrivate

bool SwitchPrivate::isChecked() const
{
    switch (state) {
    case Switch::NotAcceptedUncheck:
    case Switch::AcceptedUncheck:
        return false;

    case Switch::NotAcceptedCheck:
    case Switch::AcceptedCheck:
        return true;

    default:
        return false;
    }
}

void SwitchPrivate::setState(Switch::State st,
                             bool justUpdate)
{
    if (state != st || justUpdate) {
        state = st;

        QStyleOption opt;
        opt.initFrom(q);

        initOffset(opt.rect);

        if (!justUpdate) {
            emitSignals();
        }

        q->update();
    }
}

void SwitchPrivate::init()
{
    QStyleOption opt;
    opt.initFrom(q);

    onColor = opt.palette.color(QPalette::Highlight);
}

void SwitchPrivate::emitSignals()
{
    emit q->toggled(isChecked());
    emit q->stateChanged((int)state);
}

void SwitchPrivate::drawText(QPainter *p,
                             const QStyleOption &opt,
                             const QColor &on,
                             const QColor &off)
{
    const QRect r(radius, 0, opt.rect.width() - radius * 2, radius * 2);

    switch (state) {
    case Switch::NotAcceptedCheck:
    case Switch::AcceptedCheck: {
        if (!onText.isEmpty()) {
            p->setPen(on);
            p->drawText(r, Qt::AlignLeft | Qt::TextSingleLine | Qt::AlignVCenter, onText);
        }
    } break;

    case Switch::NotAcceptedUncheck:
    case Switch::AcceptedUncheck: {
        if (!offText.isEmpty()) {
            p->setPen(off);
            p->drawText(r, Qt::AlignRight | Qt::TextSingleLine | Qt::AlignVCenter, offText);
        }
    } break;
    }
}

void SwitchPrivate::initOffset(const QRect &r)
{
    switch (state) {
    case Switch::NotAcceptedCheck:
    case Switch::AcceptedCheck:
        offset = r.width() - radius * 2 - 2;
        break;

    case Switch::NotAcceptedUncheck:
    case Switch::AcceptedUncheck:
        offset = 0;
        break;
    }
}

//
// Switch
//

Switch::Switch(QWidget *parent)
    : QWidget(parent)
    , d(new SwitchPrivate(this))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}

Switch::Switch(State st,
               QWidget *parent)
    : QWidget(parent)
    , d(new SwitchPrivate(this))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    setState(st);
}

Switch::~Switch()
{
}

Switch::State Switch::state() const
{
    return d->state;
}

bool Switch::isChecked() const
{
    return d->isChecked();
}

QString Switch::onText() const
{
    return d->onText;
}

void Switch::setOnText(const QString &text)
{
    if (d->onText != text) {
        d->onText = text;
        updateGeometry();
    }
}

QString Switch::offText() const
{
    return d->offText;
}

void Switch::setOffText(const QString &text)
{
    if (d->offText != text) {
        d->offText = text;
        updateGeometry();
    }
}

void Switch::setState(State st)
{
    d->setState(st);
}

QColor Switch::onColor() const
{
    return d->onColor;
}

void Switch::setOnColor(const QColor &c)
{
    if (d->onColor != c) {
        d->onColor = c;

        switch (d->state) {
        case AcceptedCheck:
        case NotAcceptedUncheck:
            update();
            break;

        default:
            break;
        }
    }
}

QSize Switch::sizeHint() const
{
    QStyleOption opt;
    opt.initFrom(this);

    d->radius = opt.fontMetrics.ascent();

    int offWidth = 0;
    int onWidth = 0;

    if (!d->offText.isEmpty())
        offWidth = opt.fontMetrics.boundingRect(d->offText).width();

    if (!d->onText.isEmpty())
        onWidth = opt.fontMetrics.boundingRect(d->onText).width();

    d->textWidth = qMax(offWidth, onWidth);

    const QRect r(0, 0, d->textWidth + d->radius * 3 + d->radius / 2 + 2, d->radius * 2 + 2);

    d->initOffset(r);

    return QSize(r.width(), r.height());
}

void Switch::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);

    QColor borderColor = opt.palette.color(QPalette::Shadow);
    QColor lightColor = opt.palette.color(QPalette::Base);

    QPainter p(this);
    p.translate(1.0, 1.0);
    p.setRenderHint(QPainter::Antialiasing);

    switch (d->state) {
    case NotAcceptedUncheck:
    case AcceptedUncheck: {
        QLinearGradient g(QPointF(0.0, 0.0), QPointF(0.0, 1.0));
        g.setCoordinateMode(QGradient::ObjectBoundingMode);
        g.setColorAt(0.0, darkerColor(lightColor, 75));
        g.setColorAt(0.1, darkerColor(lightColor, 25));
        g.setColorAt(1.0, darkerColor(lightColor, 10));

        p.setBrush(g);
    } break;

    case NotAcceptedCheck:
        p.setBrush(darkerColor(d->onColor, 50));
        break;

    case AcceptedCheck:
        p.setBrush(d->onColor);
        break;
    }

    p.setPen(borderColor);

    QPainterPath rect;
    rect.addRoundedRect(0, 0, opt.rect.width() - 2, d->radius * 2, d->radius, d->radius);

    p.drawPath(rect);

    d->drawText(&p, opt, lightColor, borderColor);

    QColor lightAlphaColor = lightColor;
    lightAlphaColor.setAlpha(50);

    p.setPen(Qt::NoPen);
    p.setBrush(lightAlphaColor);

    QPainterPath glow;
    glow.addRoundedRect(d->radius / 4,
                        d->radius,
                        opt.rect.width() - d->radius / 2,
                        d->radius * 2,
                        d->radius / 2,
                        d->radius / 2);

    p.drawPath(rect.intersected(glow));

    drawSliderHandle(&p,
                     QRect(d->offset, 0, d->radius * 2, d->radius * 2),
                     d->radius,
                     d->radius,
                     borderColor,
                     lightColor);
}

void Switch::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        d->mousePos = event->pos();
        d->leftMouseButtonPressed = true;
        d->mouseMoveDelta = 0;

        event->accept();
    } else
        event->ignore();
}

void Switch::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();

        d->leftMouseButtonPressed = false;

        const int distance = rect().width() - d->radius * 2;

        if (d->mouseMoveDelta <= FingerGeometry::touchBounce()) {
            switch (d->state) {
            case NotAcceptedCheck:
            case AcceptedCheck:
                d->setState(NotAcceptedUncheck);
                break;

            case NotAcceptedUncheck:
            case AcceptedUncheck:
                d->setState(NotAcceptedCheck);
                break;

            default:
                break;
            }

            return;
        }

        if (d->offset <= distance / 2) {
            switch (d->state) {
            case NotAcceptedCheck:
            case AcceptedCheck:
                d->setState(NotAcceptedUncheck);
                break;

            default: {
                d->initOffset(rect());
                update();
            } break;
            }
        } else {
            switch (d->state) {
            case NotAcceptedUncheck:
            case AcceptedUncheck:
                d->setState(NotAcceptedCheck);
                break;

            default: {
                d->initOffset(rect());
                update();
            } break;
            }
        }
    } else
        event->ignore();
}

void Switch::mouseMoveEvent(QMouseEvent *event)
{
    if (d->leftMouseButtonPressed) {
        const int delta = event->pos().x() - d->mousePos.x();
        d->offset += delta;
        d->mouseMoveDelta += qAbs(delta);
        d->mousePos = event->pos();

        if (d->offset < 0)
            d->offset = 0;
        else if (d->offset > rect().width() - d->radius * 2 - 2)
            d->offset = rect().width() - d->radius * 2 - 2;

        update();
    }

    event->accept();
}

void Switch::resizeEvent(QResizeEvent *event)
{
    d->setState(d->state, true);

    event->accept();
}

} /* namespace MdShared */
