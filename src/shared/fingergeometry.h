
/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

namespace MdShared
{

//
// FingerGeometry
//

/*!
    FingerGeometry provides information about width and
    height of finger.
*/
class FingerGeometry
{
public:
    //! \return Width of finger.
    static int width();

    //! \return Height of finger.
    static int height();

    //! \return Bounce of the touch.
    static int touchBounce();

    //! \return Bounce of the long touch.
    static int longTouchBounce();
}; // class FingerGeometry

} /* namespace MdShared */
