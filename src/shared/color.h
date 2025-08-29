
/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QColor>

namespace MdShared
{

//
// lighterColor
//

//! \return Lighter color with HSV value bias \a b.
QColor lighterColor(const QColor &c,
                    int b);

//
// darkerColor
//

//! \return Darker color with HSV value bias \a b.
QColor darkerColor(const QColor &c,
                   int b);

} /* namespace MdShared */
