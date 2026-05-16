
/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "fingergeometry.h"

// Qt include.
#include <QApplication>
#include <QByteArray>
#include <QScreen>

namespace MdShared
{

static const qreal fingerSize = 0.0393700787 * 10;

//
// FingerGeometry
//

int FingerGeometry::width()
{
#ifdef Q_OS_ANDROID

    static const int w = qRound(qgetenv("QT_ANDROID_THEME_DISPLAY_DPI").toDouble() * fingerSize);

    return w;

#else

    static const int w = qRound((qreal)qMax(QApplication::primaryScreen()->logicalDotsPerInchX(),
                                            QApplication::primaryScreen()->physicalDotsPerInchX())
                                * fingerSize);

    return w;

#endif
}

int FingerGeometry::height()
{
#ifdef Q_OS_ANDROID

    static const int h = qRound(qgetenv("QT_ANDROID_THEME_DISPLAY_DPI").toDouble() * fingerSize);

    return h;

#else

    static const int h = qRound((qreal)qMax(QApplication::primaryScreen()->logicalDotsPerInchY(),
                                            QApplication::primaryScreen()->physicalDotsPerInchY())
                                * fingerSize);

    return h;

#endif
}

int FingerGeometry::touchBounce()
{
    static const int bounce = qRound((qreal)qMax(FingerGeometry::height(), FingerGeometry::width()) * 0.2);

    return bounce;
}

int FingerGeometry::longTouchBounce()
{
    static const int bounce = qRound((qreal)qMax(FingerGeometry::height(), FingerGeometry::width()) * 0.6);

    return bounce;
}

} /* namespace MdShared */
