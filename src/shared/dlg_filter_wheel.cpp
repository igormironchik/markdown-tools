/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "dlg_filter_wheel.h"

// Qt include.
#include <QChildEvent>
#include <QComboBox>
#include <QEvent>
#include <QSpinBox>

namespace MdShared
{

DlgWheelFilter::DlgWheelFilter(QWidget *parent)
    : QDialog(parent)
{
}

bool DlgWheelFilter::eventFilter(QObject *watched,
                                 QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        event->ignore();

        return true;
    }

    return QObject::eventFilter(watched, event);
}

void DlgWheelFilter::installFilterForChildren(QObject *parent)
{
    for (const auto &o : std::as_const(parent->children())) {
        if (isFilteredType(o)) {
            o->installEventFilter(this);
        } else {
            installFilterForChildren(o);
        }
    }
}

bool DlgWheelFilter::isFilteredType(QObject *o)
{
    return (qobject_cast<QComboBox *>(o) || qobject_cast<QSpinBox *>(o));
}

} /* namespace MdShared */
