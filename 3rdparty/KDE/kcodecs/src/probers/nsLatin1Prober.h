/*  -*- C++ -*-
    SPDX-FileCopyrightText: 1998 Netscape Communications Corporation <developer@mozilla.org>

    SPDX-License-Identifier: MIT
*/

#ifndef nsLatin1Prober_h__
#define nsLatin1Prober_h__

#include "nsCharSetProber.h"

#include <array>

namespace kencodingprober
{
class KCODECS_NO_EXPORT nsLatin1Prober : public nsCharSetProber
{
public:
    nsLatin1Prober(void);
    ~nsLatin1Prober(void) override = default;
    nsProbingState HandleData(const char *aBuf, unsigned int aLen) override;
    const char *GetCharSetName() override
    {
        return "windows-1252";
    }
    nsProbingState GetState(void) override
    {
        return mState;
    }
    float GetConfidence(void) override;

#ifdef DEBUG_PROBE
    void DumpStatus() override;
#endif

protected:
    nsProbingState mState = eDetecting;
    char mLastCharClass;
    std::array<unsigned int, 4> mFreqCounter = {0};
};
}

#endif /* nsLatin1Prober_h__ */
