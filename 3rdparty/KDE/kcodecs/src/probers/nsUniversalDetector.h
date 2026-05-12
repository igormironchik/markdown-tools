/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>
    SPDX-FileCopyrightText: 2008 Wang Kai <zealot.kai@gmail.com>

    SPDX-License-Identifier: MIT
*/

#ifndef nsUniversalDetector_h__
#define nsUniversalDetector_h__

#include "nsCharSetProber.h"

#include <array>
#include <memory>

#define NUM_OF_CHARSET_PROBERS 3

namespace kencodingprober
{
typedef enum {
    ePureAscii = 0,
    eEscAscii = 1,
    eHighbyte = 2,
} nsInputState;

class KCODECS_NO_EXPORT nsUniversalDetector : public nsCharSetProber
{
public:
    nsUniversalDetector() = default;
    ~nsUniversalDetector() override = default;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override;
    float GetConfidence(void) override;
    nsProbingState GetState() override;

protected:
    nsInputState mInputState = ePureAscii;
    bool mDone = false;
    bool mGotData = false;
    char mLastChar = '\0';
    const char *mDetectedCharset = nullptr;

    std::array<std::unique_ptr<nsCharSetProber>, NUM_OF_CHARSET_PROBERS> mCharSetProbers = {};
    std::unique_ptr<nsCharSetProber> mEscCharSetProber;
};
}

#endif
