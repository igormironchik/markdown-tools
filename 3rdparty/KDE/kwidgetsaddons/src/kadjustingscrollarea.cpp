// SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB
// SPDX-FileCopyrightText: 2022 g10 Code GmbH
// SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>
// SPDX-FileContributor: Carl Schwan <carl.schwan@gnupg.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "kadjustingscrollarea.h"

#include <QApplication>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QVBoxLayout>

KAdjustingScrollArea::KAdjustingScrollArea(QWidget *parent)
    : QScrollArea{parent}
{
    setWidgetResizable(true);

    connect(qApp, &QApplication::focusChanged, this, [this](QWidget *old, QWidget *now) {
        Q_UNUSED(old);
        ensureWidgetVisible(now);
    });

    viewport()->installEventFilter(this);
}

KAdjustingScrollArea::~KAdjustingScrollArea()
{
    viewport()->removeEventFilter(this);
    if (auto w = widget()) {
        w->removeEventFilter(this);
    }
}

QSize KAdjustingScrollArea::minimumSizeHint() const
{
    const int fw = frameWidth();
    QSize sz{2 * fw, 2 * fw};
    sz += {widget()->minimumSizeHint().width(), 0};
    if (verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        sz.setWidth(sz.width() + verticalScrollBar()->sizeHint().width());
    }
    if (horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        sz.setHeight(sz.height() + horizontalScrollBar()->sizeHint().height());
    }
    return QScrollArea::minimumSizeHint().expandedTo(sz);
}

QSize KAdjustingScrollArea::sizeHint() const
{
    const int fw = frameWidth();
    QSize sz{2 * fw, 2 * fw};
    sz += viewportSizeHint();
    if (verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        sz.setWidth(sz.width() + verticalScrollBar()->sizeHint().width());
    }
    if (horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOff) {
        sz.setHeight(sz.height() + horizontalScrollBar()->sizeHint().height());
    }
    sz = QScrollArea::sizeHint().expandedTo(sz);
    return sz;
}

static void adjustSizeOfWindowBy(const QSize &extent, QWidget *window)
{
    if (window) {
        const auto currentSize = window->size();
        // we limit the automatic size adjustment to 2/3 of the screen's size
        const auto maxWindowSize = window->screen()->geometry().size() * 2 / 3;
        const auto newWindowSize = currentSize.expandedTo((currentSize + extent).boundedTo(maxWindowSize));
        if (newWindowSize != currentSize) {
            window->resize(newWindowSize);
        }
    }
}

bool KAdjustingScrollArea::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::ChildAdded && viewport() == obj) {
        auto childAddedEvent = static_cast<QChildEvent *>(ev);
        auto widget = childAddedEvent->child();
        if (widget->objectName().isEmpty()) {
            widget->setObjectName(QLatin1StringView("scrollarea_widget"));
        }
        widget->installEventFilter(this);
    }
    if (ev->type() == QEvent::ChildRemoved && viewport() == obj) {
        auto childRemovedEvent = static_cast<QChildEvent *>(ev);
        auto widget = childRemovedEvent->child();
        widget->removeEventFilter(this);
    }
    if (ev->type() == QEvent::Resize && obj == widget() && sizeAdjustPolicy() == AdjustToContents) {
        const auto *const event = static_cast<QResizeEvent *>(ev);
        if (event->size().height() > event->oldSize().height()) {
            const auto currentViewportHeight = viewport()->height();
            const auto wantedViewportHeight = event->size().height();
            const auto wantedAdditionalHeight = wantedViewportHeight - currentViewportHeight;
            if (wantedAdditionalHeight > 0) {
                adjustSizeOfWindowBy(QSize{0, wantedAdditionalHeight}, window());
            }
        }
    }
    return QScrollArea::eventFilter(obj, ev);
}

bool KAdjustingScrollArea::event(QEvent *event)
{
    return QScrollArea::event(event);
}

#include "moc_kadjustingscrollarea.cpp"
