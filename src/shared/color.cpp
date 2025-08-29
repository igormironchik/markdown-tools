
/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: MIT
*/

// Widgets include.
#include "color.h"

namespace MdShared
{

//
// lighterColor
//

QColor lighterColor(const QColor &c,
                    int b)
{
    if (b <= 0)
        return c;

    int h = 0;
    int s = 0;
    int v = 0;
    int a = 0;

    QColor hsv = c.toHsv();
    hsv.getHsv(&h, &s, &v, &a);

    v += b;

    if (v > 255) {
        s -= v - 255;

        if (s < 0)
            s = 0;

        v = 255;
    }

    hsv.setHsv(h, s, v, a);

    return hsv.convertTo(c.spec());
}

//
// darkerColor
//

QColor darkerColor(const QColor &c,
                   int b)
{
    if (b <= 0)
        return c;

    int h = 0;
    int s = 0;
    int v = 0;
    int a = 0;

    QColor hsv = c.toHsv();
    hsv.getHsv(&h, &s, &v, &a);

    v -= b;

    if (v < 0)
        v = 0;

    hsv.setHsv(h, s, v, a);

    return hsv.convertTo(c.spec());
}

} /* namespace MdShared */
