/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsEscSM_h__
#define nsEscSM_h__

#include "nsCodingStateMachine.h"

namespace kencodingprober
{
static constexpr std::array<const unsigned char, 256> HZ_cls{
    // clang-format off
    1, 0, 0, 0, 0, 0, 0, 0, // 00 - 07
    0, 0, 0, 0, 0, 0, 0, 0, // 08 - 0f
    0, 0, 0, 0, 0, 0, 0, 0, // 10 - 17
    0, 0, 0, 1, 0, 0, 0, 0, // 18 - 1f
    0, 0, 0, 0, 0, 0, 0, 0, // 20 - 27
    0, 0, 0, 0, 0, 0, 0, 0, // 28 - 2f
    0, 0, 0, 0, 0, 0, 0, 0, // 30 - 37
    0, 0, 0, 0, 0, 0, 0, 0, // 38 - 3f
    0, 0, 0, 0, 0, 0, 0, 0, // 40 - 47
    0, 0, 0, 0, 0, 0, 0, 0, // 48 - 4f
    0, 0, 0, 0, 0, 0, 0, 0, // 50 - 57
    0, 0, 0, 0, 0, 0, 0, 0, // 58 - 5f
    0, 0, 0, 0, 0, 0, 0, 0, // 60 - 67
    0, 0, 0, 0, 0, 0, 0, 0, // 68 - 6f
    0, 0, 0, 0, 0, 0, 0, 0, // 70 - 77
    0, 0, 0, 4, 0, 5, 2, 0, // 78 - 7f
    1, 1, 1, 1, 1, 1, 1, 1, // 80 - 87
    1, 1, 1, 1, 1, 1, 1, 1, // 88 - 8f
    1, 1, 1, 1, 1, 1, 1, 1, // 90 - 97
    1, 1, 1, 1, 1, 1, 1, 1, // 98 - 9f
    1, 1, 1, 1, 1, 1, 1, 1, // a0 - a7
    1, 1, 1, 1, 1, 1, 1, 1, // a8 - af
    1, 1, 1, 1, 1, 1, 1, 1, // b0 - b7
    1, 1, 1, 1, 1, 1, 1, 1, // b8 - bf
    1, 1, 1, 1, 1, 1, 1, 1, // c0 - c7
    1, 1, 1, 1, 1, 1, 1, 1, // c8 - cf
    1, 1, 1, 1, 1, 1, 1, 1, // d0 - d7
    1, 1, 1, 1, 1, 1, 1, 1, // d8 - df
    1, 1, 1, 1, 1, 1, 1, 1, // e0 - e7
    1, 1, 1, 1, 1, 1, 1, 1, // e8 - ef
    1, 1, 1, 1, 1, 1, 1, 1, // f0 - f7
    1, 1, 1, 1, 1, 1, 1, 1  // f8 - ff
    // clang-format on
};

static constexpr std::array<const unsigned char, 42> HZ_st{
    // clang-format off
    //       <INV>       ~     <?>       {       }
    eStart, eError,      3, eStart, eStart, eStart, // eStart
    eError, eError, eError, eError, eError, eError, // eError
    eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, // eItsMe
    eError, eError, eStart, eStart,      4, eError, // 3 '~'
    5,      eError,      6, eError,      5,      5, // 4 '~ { [ <Z> <R> ]*'
    4,      eError,      4, eError,      4,      4, // 5 '~ { <Z> [ <R> <Z> ]*'
    4,      eError,      4, eError,      4, eItsMe, // 6
    // clang-format on
};

static const unsigned int HZCharLenTable[] = {0, 0, 0, 0, 0, 0};

static constexpr SMModel HZSMModel{
    HZ_cls,
    6,
    HZ_st,
    HZCharLenTable,
    "HZ-GB-2312",
};

static constexpr std::array<const unsigned char, 256> ISO2022JP_cls{
    // clang-format off
    2, 0, 0, 0, 0, 0, 0, 0, // 00 - 07
    0, 0, 0, 0, 0, 0, 2, 2, // 08 - 0f
    0, 0, 0, 0, 0, 0, 0, 0, // 10 - 17
    0, 0, 0, 1, 0, 0, 0, 0, // 18 - 1f
    0, 0, 0, 0, 7, 0, 0, 0, // 20 - 27
    3, 0, 0, 0, 0, 0, 0, 0, // 28 - 2f
    0, 0, 0, 0, 0, 0, 0, 0, // 30 - 37
    0, 0, 0, 0, 0, 0, 0, 0, // 38 - 3f
    6, 0, 4, 0, 8, 0, 0, 0, // 40 - 47
    0, 9, 5, 0, 0, 0, 0, 0, // 48 - 4f
    0, 0, 0, 0, 0, 0, 0, 0, // 50 - 57
    0, 0, 0, 0, 0, 0, 0, 0, // 58 - 5f
    0, 0, 0, 0, 0, 0, 0, 0, // 60 - 67
    0, 0, 0, 0, 0, 0, 0, 0, // 68 - 6f
    0, 0, 0, 0, 0, 0, 0, 0, // 70 - 77
    0, 0, 0, 0, 0, 0, 0, 0, // 78 - 7f
    2, 2, 2, 2, 2, 2, 2, 2, // 80 - 87
    2, 2, 2, 2, 2, 2, 2, 2, // 88 - 8f
    2, 2, 2, 2, 2, 2, 2, 2, // 90 - 97
    2, 2, 2, 2, 2, 2, 2, 2, // 98 - 9f
    2, 2, 2, 2, 2, 2, 2, 2, // a0 - a7
    2, 2, 2, 2, 2, 2, 2, 2, // a8 - af
    2, 2, 2, 2, 2, 2, 2, 2, // b0 - b7
    2, 2, 2, 2, 2, 2, 2, 2, // b8 - bf
    2, 2, 2, 2, 2, 2, 2, 2, // c0 - c7
    2, 2, 2, 2, 2, 2, 2, 2, // c8 - cf
    2, 2, 2, 2, 2, 2, 2, 2, // d0 - d7
    2, 2, 2, 2, 2, 2, 2, 2, // d8 - df
    2, 2, 2, 2, 2, 2, 2, 2, // e0 - e7
    2, 2, 2, 2, 2, 2, 2, 2, // e8 - ef
    2, 2, 2, 2, 2, 2, 2, 2, // f0 - f7
    2, 2, 2, 2, 2, 2, 2, 2 // f8 - ff
    // clang-format on
};

// ISO-2022-JP-1 / RFC 2237
static constexpr std::array<const unsigned char, 70> ISO2022JP_st{
    // clang-format off
    //       <ESC>   <INV>       (       B       J       @       $       D       I
    eStart,      3, eError, eStart, eStart, eStart, eStart, eStart, eStart, eStart, // eStart
    eError, eError, eError, eError, eError, eError, eError, eError, eError, eError, // eError
    eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, eItsMe, // eItsMe
    eError, eError, eError,      5, eError, eError, eError,      4, eError, eError, // 3 'ESC'
    eError, eError, eError,      6, eItsMe, eError, eItsMe, eError, eError, eError, // 4 'ESC $'
    eError, eError, eError, eError, eItsMe, eItsMe, eError, eError, eError, eItsMe, // 5 'ESC ('
    eError, eError, eError, eError, eError, eError, eError, eError, eItsMe, eError, // 6 'ESC $ ('
    // clang-format on
};

static const unsigned int ISO2022JPCharLenTable[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static constexpr SMModel ISO2022JPSMModel = {
    ISO2022JP_cls,
    10,
    ISO2022JP_st,
    ISO2022JPCharLenTable,
    "ISO-2022-JP",
};
}
#endif // nsEscSM_h__
